#pragma once

#include <array>
#include <vector>

#include "core/bitboard.hpp"
#include "utils/types.hpp"

namespace TungstenChess
{
  typedef uint64_t Magic;
  typedef uint8_t Shift;

  class MagicMoveGen
  {
  private:
    static inline std::array<std::vector<Bitboard>, 64> ROOK_LOOKUP_TABLES = {};
    static inline std::array<std::vector<Bitboard>, 64> BISHOP_LOOKUP_TABLES = {};

    static inline const Magic ROOK_MAGICS[64] = {4625740269727738703ULL, 2325879782407501441ULL, 1059064755748548594ULL, 14739310110451763957ULL, 7016267706751106017ULL, 3781345397251105029ULL, 13303438595010102933ULL, 9309026126387632697ULL, 4000505245162437516ULL, 12564241966740396266ULL, 12669716721831026133ULL, 420159353752536399ULL, 9843743546850262014ULL, 202287029417635990ULL, 16554375331101290388ULL, 6183018435729925929ULL, 4452280247386076534ULL, 12936471821140264087ULL, 473986066400590733ULL, 15769688511090972215ULL, 17029828723900167213ULL, 11939544487826122076ULL, 16509560208669233779ULL, 2223134766388218840ULL, 9111941433538796228ULL, 11465825907482955869ULL, 7702846506175785270ULL, 10605314906479771235ULL, 8105570278718031599ULL, 13577567351565191538ULL, 9364718046461149069ULL, 1198261861037735249ULL, 4384321680121366125ULL, 8381499821544189212ULL, 17008551077454199283ULL, 1332410127023292753ULL, 7543359725946010112ULL, 2624779248238288370ULL, 8773784550601919254ULL, 9605337738619133841ULL, 16384946300303267107ULL, 16404155560864241440ULL, 101225447883459661ULL, 6023316291641687875ULL, 15202426484588440574ULL, 2927993501301818265ULL, 8018074258325649879ULL, 10163029637771407028ULL, 1136315583175641770ULL, 12673094063367902480ULL, 9041410284329441792ULL, 3541780511067141965ULL, 881564376849180413ULL, 10128850033918239872ULL, 660888779246539829ULL, 4863094497379876576ULL, 720573872436776650ULL, 9315686963794633262ULL, 15110589183561151606ULL, 547683930248644902ULL, 12195930310977001734ULL, 7932720242597564430ULL, 16425704944517605740ULL, 11987361915627374662ULL};
    static inline const Shift ROOK_SHIFTS[64] = {50, 51, 51, 51, 51, 51, 51, 50, 52, 53, 52, 53, 53, 52, 53, 52, 51, 53, 53, 52, 52, 52, 53, 52, 52, 53, 53, 52, 52, 52, 53, 52, 52, 53, 52, 52, 53, 52, 53, 52, 52, 53, 52, 52, 52, 52, 53, 52, 52, 53, 53, 52, 52, 52, 53, 52, 52, 52, 52, 52, 52, 52, 52, 52};

    static inline const Magic BISHOP_MAGICS[64] = {15342714675989640190ULL, 6007577461340950354ULL, 16908823917554112256ULL, 6464933238120839123ULL, 13926855253894263872ULL, 7515183294807424303ULL, 3233825377581302821ULL, 16050787983471935383ULL, 17090357297884846079ULL, 11342765302400929788ULL, 4109376412544377872ULL, 17081916031869536565ULL, 17798098201767970974ULL, 10719835993853963214ULL, 11974279035893710756ULL, 9487302550151921657ULL, 14036655820037376640ULL, 18018917383005545748ULL, 4902182831682394452ULL, 15071297304116933011ULL, 1281416051290030731ULL, 7282883659898543398ULL, 5616627072528383886ULL, 15717812301732065289ULL, 14986874731646972066ULL, 3366729443656503202ULL, 2542286397227582385ULL, 17910920930800835566ULL, 11084893600486527060ULL, 12040054947452418920ULL, 16954682630191060769ULL, 12701525341189308756ULL, 8705642971550188078ULL, 1270934356161875985ULL, 9161384431223931149ULL, 3295953231915050114ULL, 9495928444634759150ULL, 14471233643876311445ULL, 11743176281107040778ULL, 1157576931411790723ULL, 1351022554057381010ULL, 15654497522815450168ULL, 15596083846782157315ULL, 1619836206092341657ULL, 269468296888261626ULL, 11610344679111727573ULL, 14219259687439715590ULL, 4899857898985763749ULL, 16599342878468561509ULL, 13610989912004846955ULL, 5498930314710104633ULL, 12874985769411275323ULL, 14100637875542297370ULL, 2466531037541086642ULL, 3999110633058906306ULL, 3548711125109269170ULL, 16341307074128935558ULL, 9131822247971587526ULL, 11165780462286449975ULL, 4080446744692131855ULL, 11668739542541274627ULL, 2770844723505070599ULL, 5316234222833021038ULL, 16962301081265320392ULL};
    static inline const Shift BISHOP_SHIFTS[64] = {58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 60, 60, 59, 59, 57, 57, 57, 57, 59, 59, 59, 59, 57, 54, 54, 56, 59, 59, 59, 59, 56, 54, 54, 56, 59, 59, 59, 59, 56, 56, 56, 56, 59, 59, 59, 60, 59, 59, 59, 59, 59, 60, 58, 59, 59, 59, 59, 59, 59, 58};

  public:
    /**
     * @brief Initializes the magic move generation lookup tables
     */
    static void init();

    /**
     * @brief Gets the bishop moves bitboard for a given square and pieces bitboard
     * @param square The square to get moves for
     * @param allPieces The blockers to be used for the calculation (these are unmasked)
     */
    static Bitboard getBishopMoves(Square square, Bitboard allPieces);

    /**
     * @brief Gets the rook moves bitboard for a given square and pieces bitboard
     * @param square The square to get moves for
     * @param allPieces The blockers to be used for the calculation (these are unmasked)
     */
    static Bitboard getRookMoves(Square square, Bitboard allPieces);

  private:
    /**
     * @brief Gets all possible blocker bitboards for a given square and mask
     * @param blockers The vector to store the blockers in
     * @param square The square to get blockers for
     * @param mask The mask to get blockers for
     */
    static void getAllBlockers(std::vector<Bitboard> &blockers, Square square, Bitboard mask);

    /**
     * @brief Gets all possible shifted blocker bitboards for a given square, magic number, shift, and blockers
     * @param blockers The blocker bitboards to be shifted (in place)
     * @param magic The magic number to be used for the calculation
     * @param shift The shift to be used for the calculation
     * @param square The square to get shifted blockers for
     */
    static void shiftBlockers(std::vector<Bitboard> &blockers, Magic magic, Shift shift, Square square);

    /**
     * @brief Gets the rook moves bitboard for a given square and blockers
     * @param square The square to get moves for
     * @param blockers The blockers to be used for the calculation
     */
    static Bitboard getRookMovesBitboard(Square square, Bitboard blockers);

    /**
     * @brief Gets the bishop moves bitboard for a given square and blockers
     * @param square The square to get moves for
     * @param blockers The blockers to be used for the calculation
     */
    static Bitboard getBishopMovesBitboard(Square square, Bitboard blockers);

    /**
     * @brief Gets all possible moves bitboards for a given square and blockers
     * @param moves The vector to store the moves in
     * @param square The square to get moves for
     * @param blockers The blockers to be used for the calculation
     * @param rook Whether the moves are for a rook or bishop (true for rook, false for bishop)
     */
    static void getAllMovesBitboards(std::vector<Bitboard> &moves, Square square, std::vector<Bitboard> blockers, bool rook);

    /**
     * @brief Gets the lookup table for a given square and piece type
     * @param lookupTable The vector to store the lookup table in
     * @param square The square to get the lookup table for
     * @param rook Whether the lookup table is for a rook or bishop (true for rook, false for bishop)
     */
    static void getMovesLookupTable(std::vector<Bitboard> &lookupTable, Square square, bool rook);

    /**
     * @brief Initializes the rook lookup tables
     */
    static void initRookLookupTables();

    /**
     * @brief Initializes the bishop lookup tables
     */
    static void initBishopLookupTables();
  };
}