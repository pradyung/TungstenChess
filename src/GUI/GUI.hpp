#pragma once

#include <fstream>
#include <SFML/Graphics.hpp>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "bot/engine.hpp"
#include "utils/types.hpp"

#define SQUARE_SIZE 100
#define SPRITE_SIZE 100
#define SPRITE_SCALE float(SQUARE_SIZE) / SPRITE_SIZE

#define NO_SQUARE 64

#define DEF_BOT_COLOR DEBUG_MODE ? BOTH : BLACK
#define DEF_GUI_THREADING !DEBUG_MODE

using namespace TungstenChess;

const PieceColor BOT_COLOR = DEF_BOT_COLOR;
const bool THREADING = DEF_GUI_THREADING;
const bool HIGHLIGHT_LEGAL_MOVES = true;

enum BoardSquareColor
{
  WHITE_SQUARE = 0,
  BLACK_SQUARE = 1,
};

enum Highlight
{
  YELLOW_HIGHLIGHT = 0,
  RED_HIGHLIGHT = 1,
  GRAY_HIGHLIGHT = 2,
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

  sf::Texture m_yellowOutlineTexture;
  sf::Texture m_pieceTextures[PIECE_NUMBER];

  sf::Image m_windowIcon;

  friend class GUIHandler;

  /**
   * @brief Construct a new ResourceManager object
   */
  ResourceManager();

  /**
   * @brief Gets the resource path for the current platform
   * @return std::filesystem::path
   */
  static std::filesystem::path getResourcePath();
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

  sf::Texture m_boardSquareTextures[2];
  sf::Texture m_highlightTextures[3];

  sf::Sprite m_boardSquares[64];
  sf::Sprite m_pieces[64];

  std::array<sf::Sprite, 64> m_pieceSprites[PIECE_NUMBER];

  Bitboard m_highlightsBitboards[3] = {0};
  sf::Sprite m_highlightSprites[3][64];

  Square m_yellowOutlineIndex = NO_SQUARE;
  sf::Sprite m_yellowOutlineSprites[64];

  Square m_selectedSquareIndex = NO_SQUARE;
  Square m_draggingPieceIndex = NO_SQUARE;
  sf::Sprite m_draggingPieceSprite;
  bool_flag m_draggingPieceReleased;

  sf::Sprite m_promotionPieceSprites[BLACK + 1][4];

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