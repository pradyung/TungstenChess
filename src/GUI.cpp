#include "GUI.hpp"

int main()
{
  using namespace TungstenChess;

  RenderWindow window(VideoMode(SQUARE_SIZE * 8, SQUARE_SIZE * 8), "TungstenChess", sf::Style::Titlebar | sf::Style::Close);

  GUIHandler gui(window);

  gui.runMainLoop();

  return 0;
}

namespace TungstenChess
{
  GUIHandler::GUIHandler(RenderWindow &window)
  {
    this->window = &window;

    whiteBot.loadOpeningBook(resourceManager.m_openingBookPath, resourceManager.m_openingBookSize);
    blackBot.loadOpeningBook(resourceManager.m_openingBookPath, resourceManager.m_openingBookSize);

    loadSquareTextures();
    loadBoardSquares();

    loadPieces();
    loadPromotionPieces();

    window.setIcon(resourceManager.m_icon.getSize().x, resourceManager.m_icon.getSize().y, resourceManager.m_icon.getPixelsPtr());
  }

  void GUIHandler::runMainLoop()
  {
    Event event;

    bool needsRefresh = true;

    while (window->isOpen())
    {
      if (needsRefresh || boardUpdated.pop() || draggingPieceReleased.pop())
      {
        render();
        needsRefresh = false;
      }

      if (!(board.sideToMove() & PLAYER_COLOR) && !gameOver)
      {
        if (!isThinking)
          startThinking();
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      if (isThinking || boardUpdated)
        window->pollEvent(event);
      else
        window->waitEvent(event);

      if (event.type == Event::Closed)
      {
        window->close();
        return;
      }

      if (!isThinking && event.mouseButton.button == Mouse::Left)
      {
        if (event.type == Event::MouseButtonPressed && !gameOver)
          needsRefresh = handleLeftClick(event);

        else if (event.type == Event::MouseButtonReleased)
          needsRefresh = handleLeftRelease(event);
      }

      if (event.type == Event::MouseMoved)
        needsRefresh = (draggingPieceIndex != NO_SQUARE) || (getMouseSquareIndex() != yellowOutlineIndex);
    }
  }

  bool GUIHandler::handleLeftClick(Event &event)
  {
    Square index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

    if (!(board.sideToMove() & board[index]) && !awaitingPromotion)
      return false;

    if (awaitingPromotion)
    {
      Piece promotionPiece = getPromotionPiece(event.mouseButton.x, event.mouseButton.y);

      if (promotionPiece == EMPTY || !(promotionPiece & board.sideToMove()))
        return false;

      promotionMove |= (promotionPiece & TYPE) << 12;

      draggingPieceIndex = NO_SQUARE;

      awaitingPromotion = false;

      makeMove(promotionMove);
    }
    else
    {
      if (!(board.sideToMove() & board[index]))
        return false;

      grayHighlightsBitboard = board.getLegalPieceMovesBitboard(index);

      draggingPieceIndex = index;
    }

    return true;
  }

  bool GUIHandler::handleLeftRelease(Event &event)
  {
    if (draggingPieceIndex == NO_SQUARE)
      return false;

    if (!Bitboards::hasBit(grayHighlightsBitboard, GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y)))
    {
      draggingPieceIndex = NO_SQUARE;
      draggingPieceReleased.set();
      clearHighlights(GRAY_HIGHLIGHT);
      return false;
    }

    Square index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

    Move move = Moves::createMove(draggingPieceIndex, index);

    if (!Moves::isPromotion(index, board[draggingPieceIndex] & TYPE))
    {
      makeMove(move);
    }
    else
    {
      awaitingPromotion = true;
      promotionMove = move;
    }

    draggingPieceIndex = NO_SQUARE;

    return true;
  }

  Square GUIHandler::getMouseSquareIndex()
  {
    int x = Mouse::getPosition(*window).x;
    int y = Mouse::getPosition(*window).y;

    if (x >= 0 && x <= 8 * SQUARE_SIZE && y >= 0 && y <= 8 * SQUARE_SIZE)
      return getSquareIndex(x, y);
    else
      return NO_SQUARE;
  }

  void GUIHandler::render()
  {
    yellowOutlineIndex = getMouseSquareIndex();

    window->clear();

    drawBoardSquares();

    if (awaitingPromotion)
    {
      drawPromotionPieces();
    }
    else
    {
      drawHighlights();
      drawPieces();
    }

    window->display();
  }

  void GUIHandler::loadSquareTextures()
  {
    sf::Image square;

    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(255, 255, 255));
    squareTextures[WHITE_SQUARE].loadFromImage(square);
    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(216, 181, 149));
    squareTextures[BLACK_SQUARE].loadFromImage(square);
    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(255, 255, 0, 127));
    squareTextures[YELLOW_HIGHLIGHT].loadFromImage(square);
    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(255, 0, 0, 200));
    squareTextures[RED_HIGHLIGHT].loadFromImage(square);
    square.create(SQUARE_SIZE, SQUARE_SIZE, sf::Color(127, 127, 127, 200));
    squareTextures[GRAY_HIGHLIGHT].loadFromImage(square);
  }

  void GUIHandler::loadBoardSquares()
  {
    Square squareIndex = 0;
    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        boardSquares[squareIndex].setTexture(squareTextures[(i + j) % 2]);
        redHighlightsSprites[squareIndex].setTexture(squareTextures[RED_HIGHLIGHT]);
        yellowHighlightsSprites[squareIndex].setTexture(squareTextures[YELLOW_HIGHLIGHT]);
        grayHighlightsSprites[squareIndex].setTexture(squareTextures[GRAY_HIGHLIGHT]);
        yellowOutlineSprites[squareIndex].setTexture(resourceManager.m_yellowOutlineTexture);

        boardSquares[squareIndex].setPosition(getSquareCoordinates(j, i));
        redHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        yellowHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        grayHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        yellowOutlineSprites[squareIndex].setPosition(getSquareCoordinates(j, i));

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
        pieceSprites[j][i].setTexture(resourceManager.m_pieceTextures[j]);
        pieceSprites[j][i].setPosition(getSquareCoordinates(i));
        pieceSprites[j][i].setScale(SQUARE_SIZE / SPRITE_SIZE, SQUARE_SIZE / SPRITE_SIZE);
      }

      draggingPieceSprite.setScale(SQUARE_SIZE / SPRITE_SIZE, SQUARE_SIZE / SPRITE_SIZE);
    }
  }

  void GUIHandler::loadPromotionPieces()
  {
    for (int i = 0; i < 4; i++)
    {
      whitePromotionPieces[i].setTexture(resourceManager.m_pieceTextures[WHITE_QUEEN - i]);
      blackPromotionPieces[i].setTexture(resourceManager.m_pieceTextures[BLACK_QUEEN - i]);

      whitePromotionPieces[i].setPosition(getSquareCoordinates(10 + i));
      blackPromotionPieces[i].setPosition(getSquareCoordinates(50 + i));

      whitePromotionPieces[i].setScale(SQUARE_SIZE / SPRITE_SIZE, SQUARE_SIZE / SPRITE_SIZE);
      blackPromotionPieces[i].setScale(SQUARE_SIZE / SPRITE_SIZE, SQUARE_SIZE / SPRITE_SIZE);
    }
  }

  void GUIHandler::drawBoardSquares()
  {
    for (Square i = 0; i < 64; i++)
    {
      window->draw(boardSquares[i]);
    }
  }

  void GUIHandler::drawPieces()
  {
    for (Square i = 0; i < 64; i++)
    {
      if (draggingPieceIndex == i || (awaitingPromotion && (promotionMove & FROM) == i))
        continue;

      if (!isThinking)
        window->draw(pieceSprites[board[i]][i]);
      else
        window->draw(pieceSprites[bufferBoard[i]][i]);
    }

    if (isThinking)
      return;

    if (draggingPieceIndex != NO_SQUARE)
    {
      draggingPieceSprite.setTexture(resourceManager.m_pieceTextures[board[draggingPieceIndex]]);
      draggingPieceSprite.setPosition(Mouse::getPosition(*window).x - SQUARE_SIZE / 2, Mouse::getPosition(*window).y - SQUARE_SIZE / 2);
      window->draw(draggingPieceSprite);
    }
  }

  void GUIHandler::drawHighlights()
  {
    for (Square i = 0; i < 64; i++)
    {
      if (Bitboards::hasBit(redHighlightsBitboard, i))
        window->draw(redHighlightsSprites[i]);
      else if (Bitboards::hasBit(grayHighlightsBitboard, i))
        window->draw(grayHighlightsSprites[i]);
      else if (Bitboards::hasBit(yellowHighlightsBitboard, i) || draggingPieceIndex == i)
        window->draw(yellowHighlightsSprites[i]);

      if (yellowOutlineIndex == i)
        window->draw(yellowOutlineSprites[i]);
    }
  }

  void GUIHandler::drawPromotionPieces()
  {
    for (int i = 0; i < 4; i++)
    {
      if (board.sideToMove() == WHITE)
      {
        window->draw(whitePromotionPieces[i]);
      }
      else
      {
        window->draw(blackPromotionPieces[i]);
      }
    }
  }

  void GUIHandler::clearHighlights()
  {
    redHighlightsBitboard = 0;
    yellowHighlightsBitboard = 0;
    grayHighlightsBitboard = 0;
  }

  void GUIHandler::clearHighlights(Highlight highlight)
  {
    if (highlight == RED_HIGHLIGHT)
      redHighlightsBitboard = 0;
    else if (highlight == YELLOW_HIGHLIGHT)
      yellowHighlightsBitboard = 0;
    else if (highlight == GRAY_HIGHLIGHT)
      grayHighlightsBitboard = 0;
  }

  void GUIHandler::makeMove(Move move)
  {
    uint8_t from = move & FROM;
    uint8_t to = (move & TO) >> 6;

    if (from == to)
      return;

    board.makeMove(move);

    clearHighlights();

    if (board.isInCheck(board.sideToMove()))
      Bitboards::addBit(redHighlightsBitboard, board.kingIndex(board.sideToMove() | KING));

    GameStatus gameStatus = board.getGameStatus(board.sideToMove());

    if (gameStatus)
    {
      gameOver = true;

      if (gameStatus == LOSE)
      {
        std::cout << "Checkmate" << std::endl;
      }
      else
      {
        std::cout << "Stalemate" << std::endl;
      }
    }

    Bitboards::addBit(yellowHighlightsBitboard, from);
    Bitboards::addBit(yellowHighlightsBitboard, to);

    boardUpdated.set();
  }

  void GUIHandler::makeBotMove()
  {
    Move move = (board.sideToMove() == WHITE) ? whiteBot.generateBotMove() : blackBot.generateBotMove();

    makeMove(move);

    stopThinking();
  }

  void GUIHandler::saveBufferBoard()
  {
    for (Square i = 0; i < 64; i++)
    {
      bufferBoard[i] = board[i];
    }
  }

  void GUIHandler::startThinking()
  {
    isThinking = true;

    if (THREADING)
    {
      saveBufferBoard();

      thinkingThread = std::thread(&GUIHandler::makeBotMove, this);

      thinkingThread.detach();
    }
    else
    {
      makeBotMove();
    }
  }

  void GUIHandler::stopThinking()
  {
    isThinking = false;
  }
}