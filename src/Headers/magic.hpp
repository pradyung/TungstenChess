#pragma once

#include "types.hpp"
#include "Data/move_gen_helpers.hpp"
#include "bitboard.hpp"

namespace Chess
{
  class MagicMoveGen
  {
  public:
    std::array<std::vector<BitboardInt>, 64> ROOK_LOOKUP_TABLES;
    std::array<std::vector<BitboardInt>, 64> BISHOP_LOOKUP_TABLES;

    const Magic ROOK_MAGICS[64] = {13986269109025151658ULL, 455387236037336319ULL, 4904440556623498340ULL, 4207114460421289512ULL, 14925556156657734592ULL, 4445327252366687504ULL, 13303438595010102933ULL, 9309026126387632697ULL, 5499069074453028043ULL, 12560920730654516989ULL, 12669716721831026133ULL, 5215179849795952099ULL, 917129237152647097ULL, 202287029417635990ULL, 6247609062164340633ULL, 17853144352910636259ULL, 4452280247386076534ULL, 11011540942787492579ULL, 14339532337497002373ULL, 7245789500460358010ULL, 7885866682773708771ULL, 4582403393871527431ULL, 15635027257055869713ULL, 2223134766388218840ULL, 1089095503921861239ULL, 15712190305321630847ULL, 14625925940244155313ULL, 10605314906479771235ULL, 2793202652373093336ULL, 13577567351565191538ULL, 14645509110328724367ULL, 10036264502918288247ULL, 4384321680121366125ULL, 10864723711442536314ULL, 17008551077454199283ULL, 1332410127023292753ULL, 9288242940868138141ULL, 2624779248238288370ULL, 11796151673456193598ULL, 15308857462086270171ULL, 7979311280141582247ULL, 6162926258491406870ULL, 101225447883459661ULL, 6023316291641687875ULL, 12887182588201866524ULL, 2927993501301818265ULL, 252480181497719611ULL, 4924791429223128260ULL, 14250119487115505907ULL, 10577411385707545446ULL, 8199909949195686888ULL, 3541780511067141965ULL, 881564376849180413ULL, 10128850033918239872ULL, 8897753237078436454ULL, 4863094497379876576ULL, 16973873044716488022ULL, 11387510909049023394ULL, 7814565234667942950ULL, 10207090357350467518ULL, 12977269500768131634ULL, 5384924027994243970ULL, 10749459257138363041ULL, 14680491454483763662ULL};
    const Shift ROOK_SHIFTS[64] = {49, 50, 50, 50, 50, 50, 51, 50, 51, 52, 52, 51, 51, 52, 52, 51, 51, 52, 52, 51, 51, 51, 52, 52, 51, 52, 52, 52, 51, 52, 52, 51, 52, 52, 52, 52, 51, 52, 52, 51, 51, 52, 52, 52, 51, 52, 52, 51, 51, 52, 52, 52, 52, 52, 52, 52, 50, 51, 51, 51, 51, 51, 51, 51};

    const Magic BISHOP_MAGICS[64] = {8017588328645042171ULL, 6007577461340950354ULL, 3813354908954347151ULL, 6464933238120839123ULL, 11879815850915417173ULL, 568215059584905889ULL, 3233825377581302821ULL, 16050787983471935383ULL, 17090357297884846079ULL, 3305060684517928954ULL, 14197754980172280776ULL, 15794554870930224410ULL, 6324728021570488234ULL, 4386168619574438524ULL, 3264868769964939707ULL, 4105817386461839319ULL, 9776147838709414967ULL, 7499669276741979680ULL, 13335850962031936333ULL, 14439211441603787457ULL, 143927301612160585ULL, 10879532463223142353ULL, 3445146371971913458ULL, 5359785949763016537ULL, 5993755088132702138ULL, 11476755375037820729ULL, 16238298582571586052ULL, 11809961220229807678ULL, 1603936011622941651ULL, 12040054947452418920ULL, 10196609040130900149ULL, 7897780414771258984ULL, 13345293622117982603ULL, 15321604517186371231ULL, 6164256739328496077ULL, 3131202398497994824ULL, 15149606292837371442ULL, 13560759540658065164ULL, 17582529281731803094ULL, 15736960641726849905ULL, 6460605290323577482ULL, 164361425742159233ULL, 15596083846782157315ULL, 1619836206092341657ULL, 4751945299948273436ULL, 5347407521658715476ULL, 18135059554113176715ULL, 595316331634685307ULL, 16599342878468561509ULL, 3733481883356566675ULL, 4538998911170827286ULL, 4547734092616200512ULL, 10708688982182548051ULL, 2466531037541086642ULL, 3999110633058906306ULL, 1226899720714895682ULL, 6593071869795443279ULL, 9131822247971587526ULL, 4915140371221576591ULL, 3514027078995846705ULL, 13509357176708445488ULL, 16863355650733748365ULL, 5316234222833021038ULL, 9441936417810848256ULL};
    const Shift BISHOP_SHIFTS[64] = {57, 59, 58, 59, 58, 58, 59, 58, 59, 58, 58, 58, 58, 58, 58, 59, 58, 58, 56, 55, 55, 56, 58, 58, 58, 58, 55, 53, 52, 56, 58, 58, 58, 58, 55, 52, 52, 55, 58, 58, 58, 58, 56, 56, 55, 55, 58, 58, 59, 59, 58, 58, 58, 59, 59, 58, 57, 59, 58, 58, 58, 58, 59, 57};

    MagicMoveGen(MovesLookup &movesLookup)
    {
      initRookLookupTables(movesLookup);
      initBishopLookupTables(movesLookup);
    }

  private:
    static std::vector<BitboardInt> getAllBlockers(Square square, BitboardInt mask)
    {
      std::vector<Square> setBits = std::vector<Square>();

      for (Square i = 0; i < 64; i++)
        if (mask & (1ULL << i))
          setBits.push_back(i);

      std::vector<BitboardInt> blockers = std::vector<BitboardInt>();

      for (uint32_t i = 0; i < (1 << setBits.size()); i++)
      {
        BitboardInt blocker = 0;

        for (uint8_t j = 0; j < setBits.size(); j++)
          if (i & (1 << j))
            blocker |= 1ULL << setBits[j];

        blockers.push_back(blocker);
      }

      return blockers;
    }

    static std::vector<BitboardInt> getShiftedBlockers(Magic magic, Shift shift, Square square, std::vector<BitboardInt> blocks)
    {
      std::vector<BitboardInt> shifts = std::vector<BitboardInt>();

      for (uint32_t i = 0; i < blocks.size(); i++)
        shifts.push_back((magic * blocks[i]) >> shift);

      return shifts;
    }

    static BitboardInt getRookMovesBitboard(Square square, BitboardInt blockers)
    {
      BitboardInt movesBitboard = 0;

      int8_t directions[4] = {-8, -1, 1, 8};
      int8_t rankEdges[4] = {0, -1, -1, 7};
      int8_t fileEdges[4] = {-1, 0, 7, -1};

      for (uint8_t i = 0; i < 4; i++)
      {
        Square to = square;

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

    static BitboardInt getBishopMovesBitboard(Square square, BitboardInt blockers)
    {
      BitboardInt movesBitboard = 0;

      int8_t directions[4] = {-9, -7, 7, 9};
      int8_t rankEdges[4] = {0, 0, 7, 7};
      int8_t fileEdges[4] = {0, 7, 0, 7};

      for (uint8_t i = 0; i < 4; i++)
      {
        Square to = square;

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

    static std::vector<BitboardInt> getAllMovesBitboards(Square square, std::vector<BitboardInt> blockers, bool rook)
    {
      std::vector<BitboardInt> moves = std::vector<BitboardInt>();

      for (uint32_t i = 0; i < blockers.size(); i++)
      {
        if (rook)
          moves.push_back(getRookMovesBitboard(square, blockers[i]));
        else
          moves.push_back(getBishopMovesBitboard(square, blockers[i]));
      }

      return moves;
    }

    std::vector<BitboardInt> getMovesLookupTable(MovesLookup &movesLookup, Square square, bool rook)
    {
      Magic magic = rook ? ROOK_MAGICS[square] : BISHOP_MAGICS[square];
      Shift shift = rook ? ROOK_SHIFTS[square] : BISHOP_SHIFTS[square];

      std::vector<BitboardInt> blocks = getAllBlockers(square, rook ? movesLookup.ROOK_MASKS[square] : movesLookup.BISHOP_MASKS[square]);
      std::vector<BitboardInt> shifts = getShiftedBlockers(magic, shift, square, blocks);
      std::vector<BitboardInt> moves = getAllMovesBitboards(square, blocks, rook);

      BitboardInt maxShift = 0;
      for (uint32_t i = 0; i < shifts.size(); i++)
        maxShift = std::max(maxShift, shifts[i]);

      std::vector<BitboardInt> lookupTable = std::vector<BitboardInt>(maxShift + 1, 0);

      for (uint32_t i = 0; i < shifts.size(); i++)
        lookupTable[shifts[i]] = moves[i];

      return lookupTable;
    }

    void initRookLookupTables(MovesLookup &movesLookup)
    {
      ROOK_LOOKUP_TABLES = std::array<std::vector<BitboardInt>, 64>();

      for (Square i = 0; i < 64; i++)
      {
        ROOK_LOOKUP_TABLES[i] = getMovesLookupTable(movesLookup, i, true);
      }
    }

    void initBishopLookupTables(MovesLookup &movesLookup)
    {
      BISHOP_LOOKUP_TABLES = std::array<std::vector<BitboardInt>, 64>();

      for (Square i = 0; i < 64; i++)
      {
        BISHOP_LOOKUP_TABLES[i] = getMovesLookupTable(movesLookup, i, false);
      }
    }
  };
}