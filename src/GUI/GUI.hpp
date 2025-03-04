#pragma once

#include <fstream>
#include <SFML/Graphics.hpp>

#include "bot/engine.hpp"
#include "utils/types.hpp"

#define SQUARE_SIZE 100
#define SPRITE_SIZE 100.0f

#define NO_SQUARE 64

#define DEF_PLAYER_COLOR DEBUG_MODE ? NO_COLOR : WHITE
#define DEF_GUI_THREADING !DEBUG_MODE

using namespace TungstenChess;

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

  sf::Texture m_yellowOutlineTexture;
  sf::Texture m_pieceTextures[PIECE_NUMBER];

  sf::Image m_icon;

  friend class GUIHandler;

  /**
   * @brief Construct a new ResourceManager object
   */
  ResourceManager();
};

class GUIHandler
{
public:
  /**
   * @brief Construct a new GUIHandler object
   * @param window The window to render to
   */
  GUIHandler(sf::RenderWindow &window);

private:
  ResourceManager &m_resourceManager = ResourceManager::getInstance();

  Board m_board;
  Bot m_whiteBot = Bot(m_board);
  Bot m_blackBot = Bot(m_board);

  Piece m_bufferBoard[64];

  sf::RenderWindow &m_window;

  sf::Texture m_squareTextures[5];

  sf::Sprite m_boardSquares[64];
  sf::Sprite m_pieces[64];

  std::array<sf::Sprite, 64> m_pieceSprites[PIECE_NUMBER];

  Bitboard m_redHighlightsBitboard = 0;
  Bitboard m_yellowHighlightsBitboard = 0;
  Bitboard m_grayHighlightsBitboard = 0;

  Square m_yellowOutlineIndex = NO_SQUARE;

  sf::Sprite m_redHighlightsSprites[64];
  sf::Sprite m_yellowHighlightsSprites[64];
  sf::Sprite m_grayHighlightsSprites[64];
  sf::Sprite m_yellowOutlineSprites[64];

  Square m_draggingPieceIndex = NO_SQUARE;
  sf::Sprite m_draggingPieceSprite;
  bool_flag m_draggingPieceReleased;

  sf::Sprite m_whitePromotionPieces[4];
  sf::Sprite m_blackPromotionPieces[4];

  bool m_awaitingPromotion = false;
  Move m_promotionMove;

  bool_flag m_boardUpdated;

  bool m_gameOver = false;

  bool m_isThinking = false;

  std::thread m_thinkingThread;

public:
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

  bool handleLeftClick(sf::Event &event);   // Returns true if screen needs to be redrawn
  bool handleLeftRelease(sf::Event &event); // Returns true if screen needs to be redrawn

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

  static Square getSquareIndex(int x, int y);

  static Piece getPromotionPiece(int x, int y);

  static sf::Vector2f getSquareCoordinates(Square index);
  static sf::Vector2f getSquareCoordinates(Square x, Square y);
};