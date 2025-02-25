#pragma once

#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <string>
#include <thread>

#include "bot.hpp"
#include "utils.hpp"

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

    std::filesystem::path m_openingBookPath;
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
      std::filesystem::path resourcePath = TUNGSTENCHESS_RESOURCES_DIR;

      m_openingBookPath = resourcePath / "opening_book";
      m_openingBookSize = std::ifstream(m_openingBookPath, std::ios::binary | std::ios::ate).tellg() / sizeof(uint);

      m_yellowOutlineTexture.loadFromFile(resourcePath / "yellow_outline.png");

      sf::Texture atlas;
      atlas.loadFromFile(resourcePath / "atlas.png");

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

      m_icon.loadFromFile(resourcePath / "high_res_wn.png");
    }
  };

  class GUIHandler
  {
  private:
    ResourceManager &m_resourceManager = ResourceManager::getInstance();

    Board m_board;
    Bot m_whiteBot = Bot(m_board, 2000);
    Bot m_blackBot = Bot(m_board, 2000);

    Piece m_bufferBoard[64];

    RenderWindow &m_window;

    Texture m_squareTextures[5];

    Sprite m_boardSquares[64];
    Sprite m_pieces[64];

    std::array<Sprite, 64> m_pieceSprites[PIECE_NUMBER];

    Bitboard m_redHighlightsBitboard = 0;
    Bitboard m_yellowHighlightsBitboard = 0;
    Bitboard m_grayHighlightsBitboard = 0;

    Square m_yellowOutlineIndex = NO_SQUARE;

    Sprite m_redHighlightsSprites[64];
    Sprite m_yellowHighlightsSprites[64];
    Sprite m_grayHighlightsSprites[64];
    Sprite m_yellowOutlineSprites[64];

    Square m_draggingPieceIndex = NO_SQUARE;
    Sprite m_draggingPieceSprite;
    bool_flag m_draggingPieceReleased;

    Sprite m_whitePromotionPieces[4];
    Sprite m_blackPromotionPieces[4];

    bool m_awaitingPromotion = false;
    Move m_promotionMove;

    bool_flag m_boardUpdated;

    bool m_gameOver = false;

    bool m_isThinking = false;

    std::thread m_thinkingThread;

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
        return NO_PIECE;
      }
    }

    static Vector2f getSquareCoordinates(Square index) { return getSquareCoordinates(index % 8, index / 8); }
    static Vector2f getSquareCoordinates(Square x, Square y) { return Vector2f(x * SQUARE_SIZE, y * SQUARE_SIZE); }
  };
}