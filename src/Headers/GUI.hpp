#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include "board.hpp"
#include "images.hpp"

using namespace sf;

namespace Chess
{
  enum ScreenConstants
  {
    SQUARE_SIZE = 80,
    WIDTH_PADDING = 0,
    HEIGHT_PADDING = 0
  };

  class GUIHandler
  {
  public:
    GUIHandler(RenderWindow &window);

    void runMainLoop();

  private:
    Board board;

    RenderWindow *window;

    Texture squares[5];
    Texture piecesTextures[PIECE_NUMBER];

    Sprite boardSquares[64];
    Sprite pieces[64];

    Bitboard redHighlightsBitboard;
    Bitboard yellowHighlightsBitboard;
    Bitboard grayHighlightsBitboard;

    Sprite redHighlightsSprites[64];
    Sprite yellowHighlightsSprites[64];
    Sprite grayHighlightsSprites[64];

    enum Highlights
    {
      YELLOW_HIGHLIGHT = 2,
      RED_HIGHLIGHT = 3,
      GRAY_HIGHLIGHT = 4
    };

    Sprite promotionPieces[4];

    void loadSquareTextures();
    void loadPieceTextures();

    void loadBoardSquares();
    void loadPieces();

    void loadPromotionPieces();

    void clearHighlights();
    void clearHighlights(int highlight);

    void makeMove(Move move);
    void makeBotMove();

    void drawBoardSquares();
    void drawPieces();
    void drawHighlights();
    void drawPromotionPieces();

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

    Sprite draggingPieceSprite;
    int draggingPieceIndex = -1;

    bool awaitingPromotion = false;
    Move promotionMove;

    bool gameOver = false;

    static int getSquareIndex(int x, int y);

    static int getPromotionPiece(int x, int y);

    static Vector2f getSquareCoordinates(int index);
    static Vector2f getSquareCoordinates(int x, int y);
  };
}