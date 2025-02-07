#pragma once

#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <CoreFoundation/CoreFoundation.h>
#include <string>
#include <thread>

#include "bot.hpp"

using namespace sf;

#define SQUARE_SIZE 100
#define SPRITE_SIZE 100.0f

#define NO_SQUARE 64

namespace TungstenChess
{
  const PieceColor PLAYER_COLOR = DEF_PLAYER_COLOR;
  const bool THREADING = DEF_GUI_THREADING;

  enum Highlight
  {
    WHITE_SQUARE = 0,
    BLACK_SQUARE = 1,
    YELLOW_HIGHLIGHT = 2,
    RED_HIGHLIGHT = 3,
    GRAY_HIGHLIGHT = 4,
  };

  class ResourceManager
  {
  private:
    /**
     * @brief Get the singleton instance of the ResourceManager
     * @return ResourceManager&
     */
    static ResourceManager &getInstance()
    {
      static ResourceManager instance;
      return instance;
    }

    std::string m_openingBookPath;
    uint m_openingBookSize;

    Texture m_yellowOutlineTexture;
    Texture m_pieceTextures[PIECE_NUMBER];

    Image m_icon;

    friend class GUIHandler;

    /**
     * @brief Construct a new ResourceManager object
     */
    ResourceManager()
    {
      std::string resourcePath = getResourcePath();

      m_openingBookPath = resourcePath + "opening_book";
      m_openingBookSize = std::ifstream(m_openingBookPath, std::ios::binary | std::ios::ate).tellg() / sizeof(uint);

      m_yellowOutlineTexture.loadFromFile(resourcePath + "yellow_outline.png");

      sf::Texture atlas;
      atlas.loadFromFile(resourcePath + "atlas.png");

      m_pieceTextures[WHITE_PAWN].loadFromImage(atlas.copyToImage(), sf::IntRect(0 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[WHITE_KNIGHT].loadFromImage(atlas.copyToImage(), sf::IntRect(1 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[WHITE_BISHOP].loadFromImage(atlas.copyToImage(), sf::IntRect(2 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[WHITE_ROOK].loadFromImage(atlas.copyToImage(), sf::IntRect(3 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[WHITE_QUEEN].loadFromImage(atlas.copyToImage(), sf::IntRect(4 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[WHITE_KING].loadFromImage(atlas.copyToImage(), sf::IntRect(5 * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));

      m_pieceTextures[BLACK_PAWN].loadFromImage(atlas.copyToImage(), sf::IntRect(0 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[BLACK_KNIGHT].loadFromImage(atlas.copyToImage(), sf::IntRect(1 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[BLACK_BISHOP].loadFromImage(atlas.copyToImage(), sf::IntRect(2 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[BLACK_ROOK].loadFromImage(atlas.copyToImage(), sf::IntRect(3 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[BLACK_QUEEN].loadFromImage(atlas.copyToImage(), sf::IntRect(4 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
      m_pieceTextures[BLACK_KING].loadFromImage(atlas.copyToImage(), sf::IntRect(5 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));

      m_icon.loadFromFile(resourcePath + "high_res_wn.png");
    }

    static std::string getResourcePath()
    {
      std::string resourcePath;

      CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
      char path[PATH_MAX];
      if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
        resourcePath = "";
      else
        resourcePath = std::string(path) + "/";
      CFRelease(resourcesURL);

      return resourcePath;
    }
  };

  class GUIHandler
  {
  private:
    ResourceManager &resourceManager = ResourceManager::getInstance();

    Board board;
    Bot bot = Bot(board, 2000);

    Piece bufferBoard[64];

    RenderWindow *window;

    Texture squareTextures[5];

    Sprite boardSquares[64];
    Sprite pieces[64];

    std::array<Sprite, 64> pieceSprites[PIECE_NUMBER];

    Bitboard redHighlightsBitboard = 0;
    Bitboard yellowHighlightsBitboard = 0;
    Bitboard grayHighlightsBitboard = 0;

    Square yellowOutlineIndex = NO_SQUARE;

    Sprite redHighlightsSprites[64];
    Sprite yellowHighlightsSprites[64];
    Sprite grayHighlightsSprites[64];
    Sprite yellowOutlineSprites[64];

    Square draggingPieceIndex = NO_SQUARE;
    Sprite draggingPieceSprite;
    flag draggingPieceReleased;

    Sprite whitePromotionPieces[4];
    Sprite blackPromotionPieces[4];

    bool awaitingPromotion = false;
    Move promotionMove;

    flag boardUpdated;

    bool gameOver = false;

    bool isThinking = false;

    std::thread thinkingThread;

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
    void loadSquareTextures();

    void loadBoardSquares();
    void loadPieces();

    void loadPromotionPieces();

    void clearHighlights();
    void clearHighlights(Highlight highlight);

    Square getMouseSquareIndex();

    bool handleLeftClick(Event &event);   // Returns true if screen needs to be redrawn
    bool handleLeftRelease(Event &event); // Returns true if screen needs to be redrawn

    void render();

    void makeMove(Move move);
    void makeBotMove();

    void startThinking();
    void stopThinking();

    void saveBufferBoard();

    void drawBoardSquares();
    void drawPieces();
    void drawHighlights();
    void drawPromotionPieces();

    static Square getSquareIndex(int x, int y) { return (y / SQUARE_SIZE) * 8 + (x / SQUARE_SIZE); }

    static Piece getPromotionPiece(int x, int y)
    {
      Square index = getSquareIndex(x, y);

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

    static Vector2f getSquareCoordinates(Square index) { return getSquareCoordinates(index % 8, index / 8); }
    static Vector2f getSquareCoordinates(Square x, Square y) { return Vector2f(x * SQUARE_SIZE, y * SQUARE_SIZE); }
  };
}