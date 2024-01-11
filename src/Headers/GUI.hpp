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
  const Piece PLAYER_COLOR = WHITE;

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

    std::array<Sprite, 64> pieceSprites[PIECE_NUMBER];

    Bitboard redHighlightsBitboard;
    Bitboard yellowHighlightsBitboard;
    Bitboard grayHighlightsBitboard;

    Square yellowOutlineIndex = MAX_UINT8;

    Sprite redHighlightsSprites[64];
    Sprite yellowHighlightsSprites[64];
    Sprite grayHighlightsSprites[64];
    Sprite yellowOutlineSprites[64];

    Square draggingPieceIndex = MAX_UINT8;
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
    void clearHighlights(uint8_t highlight);

    void makeMove(Move move);
    void makeBotMove();

    void startThinking();
    void stopThinking();

    void drawBoardSquares();
    void drawPieces();
    void drawHighlights();
    void drawPromotionPieces();

    static Square getSquareIndex(int x, int y) { return (y / SQUARE_SIZE) * 8 + (x / SQUARE_SIZE); }

    static Square getPromotionPiece(int x, int y)
    {
      Square index = getSquareIndex(x, y);

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
        return EMPTY;
      }
    }

    static Vector2f getSquareCoordinates(Square index) { return getSquareCoordinates(index % 8, index / 8); }
    static Vector2f getSquareCoordinates(Square x, Square y) { return Vector2f(x * SQUARE_SIZE, y * SQUARE_SIZE); }
  };
}