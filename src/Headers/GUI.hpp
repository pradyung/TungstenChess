#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include <thread>

#include "board.hpp"

using namespace sf;

namespace Chess
{
  const int PLAYER_COLOR = WHITE;

  enum ScreenConstants
  {
    SQUARE_SIZE = 100,
  };

  enum Highlights
  {
    WHITE_SQUARE = 0,
    BLACK_SQUARE = 1,
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

    Piece bufferBoard[64];

    RenderWindow *window;

    Texture squares[6];
    Texture piecesTextures[PIECE_NUMBER];

    Sprite boardSquares[64];
    Sprite pieces[64];

    std::array<Sprite, 64> pieceSprites[PIECE_NUMBER];

    Bitboard redHighlightsBitboard;
    Bitboard yellowHighlightsBitboard;
    Bitboard grayHighlightsBitboard;

    int yellowOutlineIndex = INVALID;

    Sprite redHighlightsSprites[64];
    Sprite yellowHighlightsSprites[64];
    Sprite grayHighlightsSprites[64];
    Sprite yellowOutlineSprites[64];

    int draggingPieceIndex = INVALID;
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

    void saveBufferBoard();

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
      case C7:
        return WHITE_QUEEN;
      case D7:
        return WHITE_ROOK;
      case E7:
        return WHITE_BISHOP;
      case F7:
        return WHITE_KNIGHT;
      case C2:
        return BLACK_QUEEN;
      case D2:
        return BLACK_ROOK;
      case E2:
        return BLACK_BISHOP;
      case F2:
        return BLACK_KNIGHT;
      default:
        return INVALID;
      }
    }

    static Vector2f getSquareCoordinates(int index) { return getSquareCoordinates(index % 8, index / 8); }
    static Vector2f getSquareCoordinates(int x, int y) { return Vector2f(x * SQUARE_SIZE, y * SQUARE_SIZE); }
  };
}