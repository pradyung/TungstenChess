#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include <thread>

#include "board.hpp"
#include "Data/images.hpp"

using namespace sf;

namespace Chess
{
  enum ScreenConstants
  {
    SQUARE_SIZE = 80,
  };

  enum Highlights
  {
    YELLOW_HIGHLIGHT = 2,
    RED_HIGHLIGHT = 3,
    GRAY_HIGHLIGHT = 4,
    YELLOW_OUTLINE = 5
  };

  class GUIHandler
  {
  public:
    /**
     * @brief Construct a new GUIHandler object
     * @param window The window to render to
     */
    GUIHandler(RenderWindow &window);

    /**
     * @brief Runs the main loop of the GUI, including rendering, input handling, and move making
     */
    void runMainLoop();

  private:
    Board board;

    RenderWindow *window;

    Images images;

    Texture squares[6];
    Texture piecesTextures[PIECE_NUMBER];

    Sprite boardSquares[64];
    Sprite pieces[64];

    Sprite whitePawns[64];
    Sprite whiteKnights[64];
    Sprite whiteBishops[64];
    Sprite whiteRooks[64];
    Sprite whiteQueens[64];
    Sprite whiteKings[64];

    Sprite blackPawns[64];
    Sprite blackKnights[64];
    Sprite blackBishops[64];
    Sprite blackRooks[64];
    Sprite blackQueens[64];
    Sprite blackKings[64];

    Bitboard redHighlightsBitboard;
    Bitboard yellowHighlightsBitboard;
    Bitboard grayHighlightsBitboard;

    int yellowOutlineIndex = -1;

    Sprite redHighlightsSprites[64];
    Sprite yellowHighlightsSprites[64];
    Sprite grayHighlightsSprites[64];
    Sprite yellowOutlineSprites[64];

    int draggingPieceIndex = -1;
    Sprite draggingPieceSprite;

    Sprite whitePromotionPieces[4];
    Sprite blackPromotionPieces[4];

    bool awaitingPromotion = false;
    Move promotionMove;

    bool gameOver = false;

    bool isThinking = false;

    std::thread thinkingThread;

    void loadSquareTextures();
    void loadPieceTextures();

    void loadBoardSquares();
    void loadPieces();

    void loadPromotionPieces();

    void clearHighlights();
    void clearHighlights(int highlight);

    void makeMove(Move move);
    void makeBotMove();

    void startThinking();
    void stopThinking();

    void drawBoardSquares();
    void drawPieces();
    void drawHighlights();
    void drawPromotionPieces();

    static int getSquareIndex(int x, int y) { return (y / SQUARE_SIZE) * 8 + (x / SQUARE_SIZE); }

    static int getPromotionPiece(int x, int y)
    {
      int index = getSquareIndex(x, y);

      switch (index)
      {
      case 10:
        return WHITE_QUEEN;
      case 11:
        return WHITE_ROOK;
      case 12:
        return WHITE_BISHOP;
      case 13:
        return WHITE_KNIGHT;
      case 50:
        return BLACK_QUEEN;
      case 51:
        return BLACK_ROOK;
      case 52:
        return BLACK_BISHOP;
      case 53:
        return BLACK_KNIGHT;
      default:
        return -1;
      }
    }

    static Vector2f getSquareCoordinates(int index) { return getSquareCoordinates(index % 8, index / 8); }
    static Vector2f getSquareCoordinates(int x, int y) { return Vector2f(x * SQUARE_SIZE, y * SQUARE_SIZE); }
  };
}