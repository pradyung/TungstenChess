#include "GUI.hpp"

#include <iostream>

using namespace sf;

std::filesystem::path ResourceManager::getResourcePath()
{
#ifdef __APPLE__
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  char path[PATH_MAX];
  if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
  {
    return "";
  }
  CFRelease(resourcesURL);
  return path;
#else
  return TUNGSTENCHESS_RESOURCES_DIR;
#endif
}

ResourceManager::ResourceManager()
{
  std::filesystem::path resourcePath = getResourcePath();

  m_openingBookPath = resourcePath / "opening_book.dat";

  m_yellowOutlineTexture.loadFromFile(resourcePath / "yellow_outline.png");

  Texture atlas;
  atlas.loadFromFile(resourcePath / "atlas.png");

  Image atlasImage = atlas.copyToImage();

  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      IntRect pieceTextureRect = IntRect(j * SPRITE_SIZE, i * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE);
      m_pieceTextures[(WHITE << i) | (PAWN + j)].loadFromImage(atlasImage, pieceTextureRect);
    }
  }

  m_windowIcon.loadFromFile(resourcePath / "high_res_wn.png");
}

GUIHandler::GUIHandler(RenderWindow &window) : m_window(window)
{
  m_whiteBot.loadOpeningBook(m_resourceManager.m_openingBookPath);
  m_blackBot.loadOpeningBook(m_resourceManager.m_openingBookPath);

  loadSquareTextures();
  loadBoardSquares();

  loadPieces();
  loadPromotionPieces();

  m_window.setIcon(m_resourceManager.m_windowIcon.getSize().x,
                   m_resourceManager.m_windowIcon.getSize().y,
                   m_resourceManager.m_windowIcon.getPixelsPtr());
}

void GUIHandler::runMainLoop()
{
  Event event;

  bool needsRefresh = true;

  while (m_window.isOpen())
  {
    bool boardUpdated = m_boardUpdated.pop_flag();
    bool draggingPieceReleased = m_draggingPieceReleased.pop_flag();

    if (needsRefresh || boardUpdated || draggingPieceReleased)
    {
      render();
      needsRefresh = false;
    }

    if (!m_isThinking && !m_gameOver && m_board.sideToMove() == BOT_COLOR)
      startThinking();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (m_isThinking || m_boardUpdated)
      m_window.pollEvent(event);
    else
      m_window.waitEvent(event);

    if (event.type == Event::Closed)
    {
      m_window.close();
      return;
    }

    if (!m_isThinking && event.mouseButton.button == Mouse::Left)
    {
      if (!m_gameOver && event.type == Event::MouseButtonPressed)
        needsRefresh = handleLeftClick(event);

      else if (event.type == Event::MouseButtonReleased)
        needsRefresh = handleLeftRelease(event);
    }

    if (event.type == Event::MouseMoved)
      needsRefresh = (m_draggingPieceIndex != NO_SQUARE) || (getMouseSquareIndex() != m_yellowOutlineIndex);

    if (event.type == Event::MouseLeft)
    {
      m_yellowOutlineIndex = NO_SQUARE;
      m_draggingPieceIndex = NO_SQUARE;
      m_draggingPieceReleased.set_flag();
      needsRefresh = true;
    }
  }
}

bool GUIHandler::handleLeftClick(Event &event)
{
  Square index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

  bool shouldRefresh = false;

  if (m_selectedSquareIndex != NO_SQUARE)
  {
    if (m_highlightsBitboards[GRAY_HIGHLIGHT] & (1ULL << index))
    {
      Move move = Moves::createMove(m_selectedSquareIndex, index);
      if (!Moves::isPromotion(index, m_board[m_selectedSquareIndex] & TYPE))
      {
        makeMove(move);
      }
      else
      {
        m_awaitingPromotion = true;
        m_promotionMove = move;
      }
    }

    m_selectedSquareIndex = NO_SQUARE;
    clearHighlights(GRAY_HIGHLIGHT);

    shouldRefresh = true;
  }

  if (!m_awaitingPromotion)
  {
    if (!(m_board[index] & m_board.sideToMove()))
      return shouldRefresh;

    m_highlightsBitboards[GRAY_HIGHLIGHT] = m_board.getLegalPieceMovesBitboard(index);
    m_draggingPieceIndex = index;
    m_selectedSquareIndex = m_draggingPieceIndex;
  }
  else
  {
    Piece promotionPiece = getPromotionPiece(event.mouseButton.x, event.mouseButton.y);

    if (promotionPiece == NO_PIECE || !(promotionPiece & m_board.sideToMove()))
    {
      m_awaitingPromotion = false;
      m_draggingPieceIndex = NO_SQUARE;
      m_draggingPieceReleased.set_flag();
      clearHighlights(GRAY_HIGHLIGHT);
      return true;
    }

    m_awaitingPromotion = false;
    m_draggingPieceIndex = NO_SQUARE;
    m_promotionMove |= (promotionPiece & TYPE) << 12;

    makeMove(m_promotionMove);
  }

  return true;
}

bool GUIHandler::handleLeftRelease(Event &event)
{
  if (m_draggingPieceIndex == NO_SQUARE)
    return false;

  Square releasedSquareIndex = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);
  if (!Bitboards::hasBit(m_highlightsBitboards[GRAY_HIGHLIGHT], releasedSquareIndex))
  {
    m_draggingPieceIndex = NO_SQUARE;
    m_draggingPieceReleased.set_flag();
    return false;
  }

  Square index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

  Move move = Moves::createMove(m_draggingPieceIndex, index);
  if (!Moves::isPromotion(index, m_board[m_draggingPieceIndex] & TYPE))
  {
    makeMove(move);
  }
  else
  {
    m_awaitingPromotion = true;
    m_promotionMove = move;
  }

  m_draggingPieceIndex = NO_SQUARE;

  return true;
}

Square GUIHandler::getMouseSquareIndex()
{
  auto [x, y] = Mouse::getPosition(m_window);

  if (x >= 0 && x <= 8 * SQUARE_SIZE && y >= 0 && y <= 8 * SQUARE_SIZE)
    return getSquareIndex(x, y);
  else
    return NO_SQUARE;
}

void GUIHandler::render()
{
  m_yellowOutlineIndex = getMouseSquareIndex();

  m_window.clear();

  drawBoardSquares();

  if (m_awaitingPromotion)
  {
    drawPromotionPieces();
  }
  else
  {
    drawHighlights();
    drawPieces();
  }

  m_window.display();
}

void GUIHandler::loadSquareTextures()
{
  Image square;

  square.create(SQUARE_SIZE, SQUARE_SIZE, Color(255, 255, 255));
  m_boardSquareTextures[WHITE_SQUARE].loadFromImage(square);
  square.create(SQUARE_SIZE, SQUARE_SIZE, Color(216, 181, 149));
  m_boardSquareTextures[BLACK_SQUARE].loadFromImage(square);

  square.create(SQUARE_SIZE, SQUARE_SIZE, Color(255, 255, 0, 127));
  m_highlightTextures[YELLOW_HIGHLIGHT].loadFromImage(square);
  square.create(SQUARE_SIZE, SQUARE_SIZE, Color(255, 0, 0, 200));
  m_highlightTextures[RED_HIGHLIGHT].loadFromImage(square);
  square.create(SQUARE_SIZE, SQUARE_SIZE, Color(127, 127, 127, 200));
  m_highlightTextures[GRAY_HIGHLIGHT].loadFromImage(square);
}

void GUIHandler::loadBoardSquares()
{
  Square squareIndex = 0;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      m_boardSquares[squareIndex].setTexture(m_boardSquareTextures[(i + j) % 2]);
      m_yellowOutlineSprites[squareIndex].setTexture(m_resourceManager.m_yellowOutlineTexture);

      for (int highlight = YELLOW_HIGHLIGHT; highlight <= GRAY_HIGHLIGHT; highlight++)
        m_highlightSprites[highlight][squareIndex].setTexture(m_highlightTextures[highlight]);

      Vector2f squareCoordinates = getSquareCoordinates(j, i);
      m_boardSquares[squareIndex].setPosition(squareCoordinates);
      m_yellowOutlineSprites[squareIndex].setPosition(squareCoordinates);
      for (int highlight = YELLOW_HIGHLIGHT; highlight <= GRAY_HIGHLIGHT; highlight++)
        m_highlightSprites[highlight][squareIndex].setPosition(squareCoordinates);

      squareIndex++;
    }
  }
}

void GUIHandler::loadPieces()
{
  for (Square i = 0; i < 64; i++)
  {
    for (Piece j = 0; j < PIECE_NUMBER; j++)
    {
      m_pieceSprites[j][i].setTexture(m_resourceManager.m_pieceTextures[j]);
      m_pieceSprites[j][i].setPosition(getSquareCoordinates(i));
      m_pieceSprites[j][i].setScale(SPRITE_SCALE, SPRITE_SCALE);
    }

    m_draggingPieceSprite.setScale(SPRITE_SCALE, SPRITE_SCALE);
    m_draggingPieceSprite.setOrigin(SPRITE_SIZE / 2, SPRITE_SIZE / 2);
  }
}

void GUIHandler::loadPromotionPieces()
{
  for (int i = 0; i < 4; i++)
  {
    m_promotionPieceSprites[WHITE][i].setTexture(m_resourceManager.m_pieceTextures[WHITE_QUEEN - i]);
    m_promotionPieceSprites[BLACK][i].setTexture(m_resourceManager.m_pieceTextures[BLACK_QUEEN - i]);

    m_promotionPieceSprites[WHITE][i].setPosition(getSquareCoordinates(C7 + i));
    m_promotionPieceSprites[BLACK][i].setPosition(getSquareCoordinates(C2 + i));

    m_promotionPieceSprites[WHITE][i].setScale(SPRITE_SCALE, SPRITE_SCALE);
    m_promotionPieceSprites[BLACK][i].setScale(SPRITE_SCALE, SPRITE_SCALE);
  }
}

void GUIHandler::drawBoardSquares()
{
  for (Square i = 0; i < 64; i++)
  {
    m_window.draw(m_boardSquares[i]);
  }
}

void GUIHandler::drawPieces()
{
  for (Square i = 0; i < 64; i++)
  {
    if (m_draggingPieceIndex == i)
      continue;

    if (!m_isThinking)
      m_window.draw(m_pieceSprites[m_board[i]][i]);
    else
      m_window.draw(m_pieceSprites[m_bufferBoard[i]][i]);
  }

  if (m_isThinking)
    return;

  if (m_draggingPieceIndex != NO_SQUARE)
  {
    m_draggingPieceSprite.setTexture(m_resourceManager.m_pieceTextures[m_board[m_draggingPieceIndex]]);

    auto [mouseX, mouseY] = Mouse::getPosition(m_window);
    m_draggingPieceSprite.setPosition(mouseX, mouseY);
    m_window.draw(m_draggingPieceSprite);
  }
}

void GUIHandler::drawHighlights()
{
  for (Square i = 0; i < 64; i++)
  {
    for (int highlight = YELLOW_HIGHLIGHT; highlight <= GRAY_HIGHLIGHT; highlight++)
    {
      if (highlight == GRAY_HIGHLIGHT && !HIGHLIGHT_LEGAL_MOVES)
        continue;

      if (m_selectedSquareIndex == i)
        continue;

      if (Bitboards::hasBit(m_highlightsBitboards[highlight], i))
        m_window.draw(m_highlightSprites[highlight][i]);
    }

    if (m_selectedSquareIndex == i)
      m_window.draw(m_highlightSprites[YELLOW_HIGHLIGHT][i]);

    if (m_yellowOutlineIndex == i)
      m_window.draw(m_yellowOutlineSprites[i]);
  }
}

void GUIHandler::drawPromotionPieces()
{
  for (int i = 0; i < 4; i++)
    m_window.draw(m_promotionPieceSprites[m_board.sideToMove()][i]);
}

void GUIHandler::clearHighlights()
{
  for (int highlight = YELLOW_HIGHLIGHT; highlight <= GRAY_HIGHLIGHT; highlight++)
    m_highlightsBitboards[highlight] = 0;
}

void GUIHandler::clearHighlights(Highlight highlight)
{
  m_highlightsBitboards[highlight] = 0;
}

void GUIHandler::makeMove(Move move)
{
  uint8_t from = move & FROM;
  uint8_t to = (move & TO) >> 6;

  if (from == to)
    return;

  m_board.makeMove(move);

  clearHighlights();

  if (m_board.isInCheck(m_board.sideToMove()))
    Bitboards::addBit(m_highlightsBitboards[RED_HIGHLIGHT], m_board.kingIndex(m_board.sideToMove() | KING));

  Board::GameStatus gameStatus = m_board.getGameStatus(m_board.sideToMove());

  if (gameStatus)
  {
    m_gameOver = true;

    if (gameStatus == Board::LOSE)
      std::cout << "Checkmate" << std::endl;
    else
      std::cout << "Stalemate" << std::endl;
  }

  Bitboards::addBit(m_highlightsBitboards[YELLOW_HIGHLIGHT], from);
  Bitboards::addBit(m_highlightsBitboards[YELLOW_HIGHLIGHT], to);

  m_boardUpdated.set_flag();
}

void GUIHandler::makeBotMove()
{
  if (m_board.sideToMove() == WHITE)
    makeMove(m_whiteBot.generateBotMove());
  else
    makeMove(m_blackBot.generateBotMove());

  stopThinking();
}

void GUIHandler::saveBufferBoard()
{
  for (Square i = 0; i < 64; i++)
  {
    m_bufferBoard[i] = m_board[i];
  }
}

void GUIHandler::startThinking()
{
  m_isThinking = true;

  if (THREADING)
  {
    saveBufferBoard();

    m_thinkingThread = std::thread(&GUIHandler::makeBotMove, this);

    m_thinkingThread.detach();
  }
  else
  {
    makeBotMove();
  }
}

void GUIHandler::stopThinking()
{
  m_isThinking = false;
}

Square GUIHandler::getSquareIndex(int x, int y)
{
  return (y / SQUARE_SIZE) * 8 + (x / SQUARE_SIZE);
}

Piece GUIHandler::getPromotionPiece(int x, int y)
{
  Square index = getSquareIndex(x, y);

  switch (index)
  {
  case C7:
  case D7:
  case E7:
  case F7:
    return WHITE_QUEEN - (index - C7);
  case C2:
  case D2:
  case E2:
  case F2:
    return BLACK_QUEEN - (index - C2);
  default:
    return NO_PIECE;
  }
}

Vector2f GUIHandler::getSquareCoordinates(Square index)
{
  return getSquareCoordinates(index % 8, index / 8);
}

Vector2f GUIHandler::getSquareCoordinates(Square x, Square y)
{
  return Vector2f(x * SQUARE_SIZE, y * SQUARE_SIZE);
}