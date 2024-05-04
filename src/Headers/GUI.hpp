#pragma once

#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <CoreFoundation/CoreFoundation.h>
#include <string>
#include <thread>

#include "board.hpp"

using namespace sf;

#define SQUARE_SIZE 100
#define SPRITE_SIZE 100.0f

namespace Chess
{
  const int PLAYER_COLOR = BOTH;
  const bool THREADING = true;

  enum Highlights
  {
    WHITE_SQUARE = 0,
    BLACK_SQUARE = 1,
    YELLOW_HIGHLIGHT = 2,
    RED_HIGHLIGHT = 3,
    GRAY_HIGHLIGHT = 4,
  };

  class ResourceManager
  {
  public:
    /**
     * @brief Get the singleton instance of the ResourceManager
     * @return ResourceManager&
     */
    static ResourceManager &getInstance()
    {
      static ResourceManager instance;
      return instance;
    }

    std::string openingBookPath;
    uint openingBookSize;

    Texture yellowOutlineTexture;
    Texture pieceTextures[PIECE_NUMBER];

    Image icon;

  private:
    /**
     * @brief Construct a new ResourceManager object
     */
    ResourceManager()
    {
      CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
      char path[PATH_MAX];
      if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
        resourcePath = "";
      else
        resourcePath = std::string(path) + "/";
      CFRelease(resourcesURL);

      openingBookPath = resourcePath + "opening_book";
      openingBookSize = std::ifstream(openingBookPath, std::ios::binary | std::ios::ate).tellg() / sizeof(uint);

      yellowOutlineTexture.loadFromFile(resourcePath + "yellow_outline.png");

      sf::Texture atlas;
      atlas.loadFromFile(resourcePath + "atlas.png");

      pieceTextures[WHITE_PAWN].loadFromImage(atlas.copyToImage(), sf::IntRect(0 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[WHITE_KNIGHT].loadFromImage(atlas.copyToImage(), sf::IntRect(1 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[WHITE_BISHOP].loadFromImage(atlas.copyToImage(), sf::IntRect(2 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[WHITE_ROOK].loadFromImage(atlas.copyToImage(), sf::IntRect(3 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[WHITE_QUEEN].loadFromImage(atlas.copyToImage(), sf::IntRect(4 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[WHITE_KING].loadFromImage(atlas.copyToImage(), sf::IntRect(5 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));

      pieceTextures[BLACK_PAWN].loadFromImage(atlas.copyToImage(), sf::IntRect(0 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[BLACK_KNIGHT].loadFromImage(atlas.copyToImage(), sf::IntRect(1 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[BLACK_BISHOP].loadFromImage(atlas.copyToImage(), sf::IntRect(2 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[BLACK_ROOK].loadFromImage(atlas.copyToImage(), sf::IntRect(3 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[BLACK_QUEEN].loadFromImage(atlas.copyToImage(), sf::IntRect(4 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      pieceTextures[BLACK_KING].loadFromImage(atlas.copyToImage(), sf::IntRect(5 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));

      icon.loadFromFile(resourcePath + "high_res_wn.png");
    }

    std::string resourcePath;
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
    ResourceManager &resourceManager = ResourceManager::getInstance();

    Board board;

    Piece bufferBoard[64];

    RenderWindow *window;

    Texture squareTextures[5];

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
        return EMPTY;
      }
    }

    static Vector2f getSquareCoordinates(int index) { return getSquareCoordinates(index % 8, index / 8); }
    static Vector2f getSquareCoordinates(int x, int y) { return Vector2f(x * SQUARE_SIZE, y * SQUARE_SIZE); }
  };
}