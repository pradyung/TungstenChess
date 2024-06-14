#pragma once

#include <array>
#include <vector>

#include "move_gen_helpers.hpp"

namespace TungstenChess
{
  typedef uint64_t Magic;
  typedef uint8_t Shift;

  const Magic ROOK_MAGICS[64] = {4625740269727738703ULL, 2325879782407501441ULL, 1059064755748548594ULL, 14739310110451763957ULL, 7016267706751106017ULL, 3781345397251105029ULL, 13303438595010102933ULL, 9309026126387632697ULL, 4000505245162437516ULL, 12564241966740396266ULL, 12669716721831026133ULL, 420159353752536399ULL, 9843743546850262014ULL, 202287029417635990ULL, 16554375331101290388ULL, 6183018435729925929ULL, 4452280247386076534ULL, 12936471821140264087ULL, 473986066400590733ULL, 15769688511090972215ULL, 17029828723900167213ULL, 11939544487826122076ULL, 16509560208669233779ULL, 2223134766388218840ULL, 9111941433538796228ULL, 11465825907482955869ULL, 7702846506175785270ULL, 10605314906479771235ULL, 8105570278718031599ULL, 13577567351565191538ULL, 9364718046461149069ULL, 1198261861037735249ULL, 4384321680121366125ULL, 8381499821544189212ULL, 17008551077454199283ULL, 1332410127023292753ULL, 7543359725946010112ULL, 2624779248238288370ULL, 8773784550601919254ULL, 9605337738619133841ULL, 16384946300303267107ULL, 16404155560864241440ULL, 101225447883459661ULL, 6023316291641687875ULL, 15202426484588440574ULL, 2927993501301818265ULL, 8018074258325649879ULL, 10163029637771407028ULL, 1136315583175641770ULL, 12673094063367902480ULL, 9041410284329441792ULL, 3541780511067141965ULL, 881564376849180413ULL, 10128850033918239872ULL, 660888779246539829ULL, 4863094497379876576ULL, 720573872436776650ULL, 9315686963794633262ULL, 15110589183561151606ULL, 547683930248644902ULL, 12195930310977001734ULL, 7932720242597564430ULL, 16425704944517605740ULL, 11987361915627374662ULL};
  const Shift ROOK_SHIFTS[64] = {50, 51, 51, 51, 51, 51, 51, 50, 52, 53, 52, 53, 53, 52, 53, 52, 51, 53, 53, 52, 52, 52, 53, 52, 52, 53, 53, 52, 52, 52, 53, 52, 52, 53, 52, 52, 53, 52, 53, 52, 52, 53, 52, 52, 52, 52, 53, 52, 52, 53, 53, 52, 52, 52, 53, 52, 52, 52, 52, 52, 52, 52, 52, 52};

  const Magic BISHOP_MAGICS[64] = {15342714675989640190ULL, 6007577461340950354ULL, 16908823917554112256ULL, 6464933238120839123ULL, 13926855253894263872ULL, 7515183294807424303ULL, 3233825377581302821ULL, 16050787983471935383ULL, 17090357297884846079ULL, 11342765302400929788ULL, 4109376412544377872ULL, 17081916031869536565ULL, 17798098201767970974ULL, 10719835993853963214ULL, 11974279035893710756ULL, 9487302550151921657ULL, 14036655820037376640ULL, 18018917383005545748ULL, 4902182831682394452ULL, 15071297304116933011ULL, 1281416051290030731ULL, 7282883659898543398ULL, 5616627072528383886ULL, 15717812301732065289ULL, 14986874731646972066ULL, 3366729443656503202ULL, 2542286397227582385ULL, 17910920930800835566ULL, 11084893600486527060ULL, 12040054947452418920ULL, 16954682630191060769ULL, 12701525341189308756ULL, 8705642971550188078ULL, 1270934356161875985ULL, 9161384431223931149ULL, 3295953231915050114ULL, 9495928444634759150ULL, 14471233643876311445ULL, 11743176281107040778ULL, 1157576931411790723ULL, 1351022554057381010ULL, 15654497522815450168ULL, 15596083846782157315ULL, 1619836206092341657ULL, 269468296888261626ULL, 11610344679111727573ULL, 14219259687439715590ULL, 4899857898985763749ULL, 16599342878468561509ULL, 13610989912004846955ULL, 5498930314710104633ULL, 12874985769411275323ULL, 14100637875542297370ULL, 2466531037541086642ULL, 3999110633058906306ULL, 3548711125109269170ULL, 16341307074128935558ULL, 9131822247971587526ULL, 11165780462286449975ULL, 4080446744692131855ULL, 11668739542541274627ULL, 2770844723505070599ULL, 5316234222833021038ULL, 16962301081265320392ULL};
  const Shift BISHOP_SHIFTS[64] = {58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 60, 60, 59, 59, 57, 57, 57, 57, 59, 59, 59, 59, 57, 54, 54, 56, 59, 59, 59, 59, 56, 54, 54, 56, 59, 59, 59, 59, 56, 56, 56, 56, 59, 59, 59, 60, 59, 59, 59, 59, 59, 60, 58, 59, 59, 59, 59, 59, 59, 58};

  class MagicMoveGen
  {
  public:
    /**
     * @brief Get the instance of the MagicMoveGen singleton
     * @return MagicMoveGen&
     */
    static MagicMoveGen &getInstance()
    {
      static MagicMoveGen instance;
      return instance;
    }

    std::array<std::vector<Bitboard>, 64> ROOK_LOOKUP_TABLES;
    std::array<std::vector<Bitboard>, 64> BISHOP_LOOKUP_TABLES;

    MovesLookup &movesLookup = MovesLookup::getInstance();

    /**
     * @brief Gets the bishop moves bitboard for a given square and pieces bitboard
     * @param square The square to get moves for
     * @param allPieces The blockers to be used for the calculation (these are unmasked)
     */
    Bitboard getBishopMoves(int square, Bitboard allPieces)
    {
      return BISHOP_LOOKUP_TABLES[square][((BISHOP_MAGICS[square] * (allPieces & movesLookup.BISHOP_MASKS[square])) >> BISHOP_SHIFTS[square])];
    }

    /**
     * @brief Gets the rook moves bitboard for a given square and pieces bitboard
     * @param square The square to get moves for
     * @param allPieces The blockers to be used for the calculation (these are unmasked)
     */
    Bitboard getRookMoves(int square, Bitboard allPieces)
    {
      return ROOK_LOOKUP_TABLES[square][((ROOK_MAGICS[square] * (allPieces & movesLookup.ROOK_MASKS[square])) >> ROOK_SHIFTS[square])];
    }

  private:
    /**
     * @brief Initializes the magic move generation lookup tables
     * @param movesLookup The move generation helper object with populated mask lookup tables
     */
    MagicMoveGen()
    {
      initRookLookupTables();
      initBishopLookupTables();
    }

    /**
     * @brief Gets all possible blocker bitboards for a given square and mask
     * @param square The square to get blockers for
     * @param mask The mask to get blockers for
     */
    static std::vector<Bitboard> getAllBlockers(int square, Bitboard mask)
    {
      std::vector<int> setBits = std::vector<int>();

      for (int i = 0; i < 64; i++)
        if (mask & (1ULL << i))
          setBits.push_back(i);

      std::vector<Bitboard> blockers = std::vector<Bitboard>();

      for (int i = 0; i < (1 << setBits.size()); i++)
      {
        Bitboard blocker = 0;

        for (int j = 0; j < setBits.size(); j++)
          if (i & (1 << j))
            blocker |= 1ULL << setBits[j];

        blockers.push_back(blocker);
      }

      return blockers;
    }

    /**
     * @brief Gets all possible shifted blocker bitboards for a given square, magic number, shift, and blockers
     * @param magic The magic number to be used for the calculation
     * @param shift The shift to be used for the calculation
     * @param square The square to get shifted blockers for
     * @param blocks The blocker bitboards to be shifted
     */
    static std::vector<Bitboard> getShiftedBlockers(Magic magic, Shift shift, int square, std::vector<Bitboard> blocks)
    {
      std::vector<Bitboard> shifts = std::vector<Bitboard>();

      for (int i = 0; i < blocks.size(); i++)
        shifts.push_back((magic * blocks[i]) >> shift);

      return shifts;
    }

    /**
     * @brief Gets the rook moves bitboard for a given square and blockers
     * @param square The square to get moves for
     * @param blockers The blockers to be used for the calculation
     */
    static Bitboard getRookMovesBitboard(int square, Bitboard blockers)
    {
      Bitboard movesBitboard = 0;

      int directions[4] = {-8, -1, 1, 8};
      int rankEdges[4] = {0, -1, -1, 7};
      int fileEdges[4] = {-1, 0, 7, -1};

      for (int i = 0; i < 4; i++)
      {
        int to = square;

        while (true)
        {
          movesBitboard |= 1ULL << to;

          bool isEdge = (to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]);

          if (isEdge || (blockers & (1ULL << to)))
            break;

          to += directions[i];
        }
      }

      return movesBitboard & ~(1ULL << square);
    }

    /**
     * @brief Gets the bishop moves bitboard for a given square and blockers
     * @param square The square to get moves for
     * @param blockers The blockers to be used for the calculation
     */
    static Bitboard getBishopMovesBitboard(int square, Bitboard blockers)
    {
      Bitboard movesBitboard = 0;

      int directions[4] = {-9, -7, 7, 9};
      int rankEdges[4] = {0, 0, 7, 7};
      int fileEdges[4] = {0, 7, 0, 7};

      for (int i = 0; i < 4; i++)
      {
        int to = square;

        while (true)
        {
          movesBitboard |= 1ULL << to;

          bool isEdge = (to / 8 == rankEdges[i]) || (to % 8 == fileEdges[i]);

          if (isEdge || (blockers & (1ULL << to)))
            break;

          to += directions[i];
        }
      }

      return movesBitboard & ~(1ULL << square);
    }

    /**
     * @brief Gets all possible moves bitboards for a given square and blockers
     * @param square The square to get moves for
     * @param blockers The blockers to be used for the calculation
     * @param rook Whether the moves are for a rook or bishop (true for rook, false for bishop)
     */
    static std::vector<Bitboard> getAllMovesBitboards(int square, std::vector<Bitboard> blockers, bool rook)
    {
      std::vector<Bitboard> moves = std::vector<Bitboard>();

      for (int i = 0; i < blockers.size(); i++)
      {
        if (rook)
          moves.push_back(getRookMovesBitboard(square, blockers[i]));
        else
          moves.push_back(getBishopMovesBitboard(square, blockers[i]));
      }

      return moves;
    }

    /**
     * @brief Gets the lookup table for a given square and piece type
     * @param square The square to get the lookup table for
     * @param rook Whether the lookup table is for a rook or bishop (true for rook, false for bishop)
     */
    std::vector<Bitboard> getMovesLookupTable(int square, bool rook)
    {
      Magic magic = rook ? ROOK_MAGICS[square] : BISHOP_MAGICS[square];
      Shift shift = rook ? ROOK_SHIFTS[square] : BISHOP_SHIFTS[square];

      std::vector<Bitboard> blocks = getAllBlockers(square, rook ? movesLookup.ROOK_MASKS[square] : movesLookup.BISHOP_MASKS[square]);
      std::vector<Bitboard> shifts = getShiftedBlockers(magic, shift, square, blocks);
      std::vector<Bitboard> moves = getAllMovesBitboards(square, blocks, rook);

      Bitboard maxShift = 0;
      for (int i = 0; i < shifts.size(); i++)
        maxShift = std::max(maxShift, shifts[i]);

      std::vector<Bitboard> lookupTable = std::vector<Bitboard>(maxShift + 1, 0);

      for (int i = 0; i < shifts.size(); i++)
        lookupTable[shifts[i]] = moves[i];

      return lookupTable;
    }

    /**
     * @brief Initializes the rook lookup tables
     */
    void initRookLookupTables()
    {
      ROOK_LOOKUP_TABLES = std::array<std::vector<Bitboard>, 64>();

      for (int i = 0; i < 64; i++)
      {
        ROOK_LOOKUP_TABLES[i] = getMovesLookupTable(i, true);
      }
    }

    /**
     * @brief Initializes the bishop lookup tables
     */
    void initBishopLookupTables()
    {
      BISHOP_LOOKUP_TABLES = std::array<std::vector<Bitboard>, 64>();

      for (int i = 0; i < 64; i++)
      {
        BISHOP_LOOKUP_TABLES[i] = getMovesLookupTable(i, false);
      }
    }
  };
}