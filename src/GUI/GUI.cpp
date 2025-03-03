#include "GUI.hpp"

namespace TungstenChess
{
  GUIHandler::GUIHandler(RenderWindow &window) : m_window(window)
  {
    m_whiteBot.loadOpeningBook(m_resourceManager.m_openingBookPath, m_resourceManager.m_openingBookSize);
    m_blackBot.loadOpeningBook(m_resourceManager.m_openingBookPath, m_resourceManager.m_openingBookSize);

    loadSquareTextures();
    loadBoardSquares();

    loadPieces();
    loadPromotionPieces();

    m_window.setIcon(m_resourceManager.m_icon.getSize().x, m_resourceManager.m_icon.getSize().y, m_resourceManager.m_icon.getPixelsPtr());
  }

  void GUIHandler::runMainLoop()
  {
    Event event;

    bool needsRefresh = true;

    while (m_window.isOpen())
    {
      if (needsRefresh || m_boardUpdated.pop_flag() || m_draggingPieceReleased.pop_flag())
      {
        render();
        needsRefresh = false;
      }

      if (!(m_board.sideToMove() & PLAYER_COLOR) && !m_gameOver)
      {
        if (!m_isThinking)
          startThinking();
      }

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
        if (event.type == Event::MouseButtonPressed && !m_gameOver)
          needsRefresh = handleLeftClick(event);

        else if (event.type == Event::MouseButtonReleased)
          needsRefresh = handleLeftRelease(event);
      }

      if (event.type == Event::MouseMoved)
        needsRefresh = (m_draggingPieceIndex != NO_SQUARE) || (getMouseSquareIndex() != m_yellowOutlineIndex);
    }
  }

  bool GUIHandler::handleLeftClick(Event &event)
  {
    Square index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

    if (!(m_board.sideToMove() & m_board[index]) && !m_awaitingPromotion)
      return false;

    if (m_awaitingPromotion)
    {
      Piece promotionPiece = getPromotionPiece(event.mouseButton.x, event.mouseButton.y);

      if (promotionPiece == NO_PIECE || !(promotionPiece & m_board.sideToMove()))
        return false;

      m_promotionMove |= (promotionPiece & TYPE) << 12;

      m_draggingPieceIndex = NO_SQUARE;

      m_awaitingPromotion = false;

      makeMove(m_promotionMove);
    }
    else
    {
      if (!(m_board.sideToMove() & m_board[index]))
        return false;

      m_grayHighlightsBitboard = m_board.getLegalPieceMovesBitboard(index);

      m_draggingPieceIndex = index;
    }

    return true;
  }

  bool GUIHandler::handleLeftRelease(Event &event)
  {
    if (m_draggingPieceIndex == NO_SQUARE)
      return false;

    if (!Bitboards::hasBit(m_grayHighlightsBitboard, GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y)))
    {
      m_draggingPieceIndex = NO_SQUARE;
      m_draggingPieceReleased.set_flag();
      clearHighlights(GRAY_HIGHLIGHT);
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
    sf::Image square;

    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(255, 255, 255));
    m_squareTextures[WHITE_SQUARE].loadFromImage(square);
    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(216, 181, 149));
    m_squareTextures[BLACK_SQUARE].loadFromImage(square);
    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(255, 255, 0, 127));
    m_squareTextures[YELLOW_HIGHLIGHT].loadFromImage(square);
    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(255, 0, 0, 200));
    m_squareTextures[RED_HIGHLIGHT].loadFromImage(square);
    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(127, 127, 127, 200));
    m_squareTextures[GRAY_HIGHLIGHT].loadFromImage(square);
  }

  void GUIHandler::loadBoardSquares()
  {
    Square squareIndex = 0;
    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        m_boardSquares[squareIndex].setTexture(m_squareTextures[(i + j) % 2]);
        m_redHighlightsSprites[squareIndex].setTexture(m_squareTextures[RED_HIGHLIGHT]);
        m_yellowHighlightsSprites[squareIndex].setTexture(m_squareTextures[YELLOW_HIGHLIGHT]);
        m_grayHighlightsSprites[squareIndex].setTexture(m_squareTextures[GRAY_HIGHLIGHT]);
        m_yellowOutlineSprites[squareIndex].setTexture(m_resourceManager.m_yellowOutlineTexture);

        m_boardSquares[squareIndex].setPosition(getSquareCoordinates(j, i));
        m_redHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        m_yellowHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        m_grayHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        m_yellowOutlineSprites[squareIndex].setPosition(getSquareCoordinates(j, i));

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
        m_pieceSprites[j][i].setScale(SQUARE_SIZE / SPRITE_SIZE, SQUARE_SIZE / SPRITE_SIZE);
      }

      m_draggingPieceSprite.setScale(SQUARE_SIZE / SPRITE_SIZE, SQUARE_SIZE / SPRITE_SIZE);
    }
  }

  void GUIHandler::loadPromotionPieces()
  {
    for (int i = 0; i < 4; i++)
    {
      m_whitePromotionPieces[i].setTexture(m_resourceManager.m_pieceTextures[WHITE_QUEEN - i]);
      m_blackPromotionPieces[i].setTexture(m_resourceManager.m_pieceTextures[BLACK_QUEEN - i]);

      m_whitePromotionPieces[i].setPosition(getSquareCoordinates(10 + i));
      m_blackPromotionPieces[i].setPosition(getSquareCoordinates(50 + i));

      m_whitePromotionPieces[i].setScale(SQUARE_SIZE / SPRITE_SIZE, SQUARE_SIZE / SPRITE_SIZE);
      m_blackPromotionPieces[i].setScale(SQUARE_SIZE / SPRITE_SIZE, SQUARE_SIZE / SPRITE_SIZE);
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
      if (m_draggingPieceIndex == i || (m_awaitingPromotion && (m_promotionMove & FROM) == i))
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
      m_draggingPieceSprite.setPosition(mouseX - SQUARE_SIZE / 2, mouseY - SQUARE_SIZE / 2);
      m_window.draw(m_draggingPieceSprite);
    }
  }

  void GUIHandler::drawHighlights()
  {
    for (Square i = 0; i < 64; i++)
    {
      if (Bitboards::hasBit(m_redHighlightsBitboard, i))
        m_window.draw(m_redHighlightsSprites[i]);
      else if (Bitboards::hasBit(m_grayHighlightsBitboard, i))
        m_window.draw(m_grayHighlightsSprites[i]);
      else if (Bitboards::hasBit(m_yellowHighlightsBitboard, i) || m_draggingPieceIndex == i)
        m_window.draw(m_yellowHighlightsSprites[i]);

      if (m_yellowOutlineIndex == i)
        m_window.draw(m_yellowOutlineSprites[i]);
    }
  }

  void GUIHandler::drawPromotionPieces()
  {
    for (int i = 0; i < 4; i++)
    {
      if (m_board.sideToMove() == WHITE)
      {
        m_window.draw(m_whitePromotionPieces[i]);
      }
      else
      {
        m_window.draw(m_blackPromotionPieces[i]);
      }
    }
  }

  void GUIHandler::clearHighlights()
  {
    m_redHighlightsBitboard = 0;
    m_yellowHighlightsBitboard = 0;
    m_grayHighlightsBitboard = 0;
  }

  void GUIHandler::clearHighlights(Highlight highlight)
  {
    if (highlight == RED_HIGHLIGHT)
      m_redHighlightsBitboard = 0;
    else if (highlight == YELLOW_HIGHLIGHT)
      m_yellowHighlightsBitboard = 0;
    else if (highlight == GRAY_HIGHLIGHT)
      m_grayHighlightsBitboard = 0;
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
      Bitboards::addBit(m_redHighlightsBitboard, m_board.kingIndex(m_board.sideToMove() | KING));

    Board::GameStatus gameStatus = m_board.getGameStatus(m_board.sideToMove());

    if (gameStatus)
    {
      m_gameOver = true;

      if (gameStatus == Board::LOSE)
        std::cout << "Checkmate" << std::endl;
      else
        std::cout << "Stalemate" << std::endl;
    }

    Bitboards::addBit(m_yellowHighlightsBitboard, from);
    Bitboards::addBit(m_yellowHighlightsBitboard, to);

    m_boardUpdated.set_flag();
  }

  void GUIHandler::makeBotMove()
  {
    Move move = (m_board.sideToMove() == WHITE) ? m_whiteBot.generateBotMove() : m_blackBot.generateBotMove();

    makeMove(move);

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
}