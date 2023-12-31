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
    WIDTH_PADDING = 0,
    HEIGHT_PADDING = 0
  };

  enum Highlights
  {
    YELLOW_HIGHLIGHT = 2,
    RED_HIGHLIGHT = 3,
    GRAY_HIGHLIGHT = 4
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

    Texture squares[5];
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

    Sprite redHighlightsSprites[64];
    Sprite yellowHighlightsSprites[64];
    Sprite grayHighlightsSprites[64];

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

    static int getSquareIndex(int x, int y);

    static int getPromotionPiece(int x, int y);

    static Vector2f getSquareCoordinates(int index);
    static Vector2f getSquareCoordinates(int x, int y);
  };
}