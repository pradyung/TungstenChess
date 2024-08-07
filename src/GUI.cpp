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

    bot.loadOpeningBook(resourceManager.m_openingBookPath, resourceManager.m_openingBookSize);

    loadSquareTextures();
    loadBoardSquares();

    loadPieces();
    loadPromotionPieces();

    window.setIcon(resourceManager.m_icon.getSize().x, resourceManager.m_icon.getSize().y, resourceManager.m_icon.getPixelsPtr());
  }

  void GUIHandler::runMainLoop()
  {
    while (window->isOpen())
    {
      Event event;

      if (!(board.sideToMove() & PLAYER_COLOR) && !gameOver)
      {
        if (!isThinking)
          startThinking();
      }

      while (window->pollEvent(event))
      {
        if (event.type == Event::Closed)
        {
          window->close();
          return;
        }

        if (isThinking)
          continue;

        if (event.type == Event::MouseButtonPressed)
        {
          if (event.mouseButton.button == Mouse::Left && !gameOver)
          {
            int index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

            if (!(board.sideToMove() & board[index]) && !awaitingPromotion)
              continue;

            if (awaitingPromotion)
            {
              int promotionPiece = getPromotionPiece(event.mouseButton.x, event.mouseButton.y);

              if (promotionPiece == EMPTY || !(promotionPiece & board.sideToMove()))
                continue;

              promotionMove.promotionPieceType = promotionPiece & TYPE;

              draggingPieceIndex = NO_SQUARE;

              awaitingPromotion = false;

              makeMove(promotionMove);
            }
            else
            {
              if (!(board.sideToMove() & board[index]))
                continue;

              grayHighlightsBitboard = board.getLegalPieceMovesBitboard(index);

              draggingPieceIndex = index;
            }
          }
        }

        if (event.type == Event::MouseButtonReleased)
        {
          if (event.mouseButton.button == Mouse::Left)
          {
            if (draggingPieceIndex == NO_SQUARE)
              continue;

            if (!Bitboards::hasBit(grayHighlightsBitboard, GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y)))
            {
              draggingPieceIndex = NO_SQUARE;
              clearHighlights(GRAY_HIGHLIGHT);
              continue;
            }

            int index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

            Move move(draggingPieceIndex, index, board[draggingPieceIndex], board[index], board.castlingRights(), board.enPassantFile(), board.halfmoveClock());

            if (!(move.flags & PROMOTION))
            {
              makeMove(move);
            }
            else
            {
              awaitingPromotion = true;
              promotionMove = move;
            }

            draggingPieceIndex = NO_SQUARE;
          }
        }
      }

      int x = Mouse::getPosition(*window).x;
      int y = Mouse::getPosition(*window).y;
      if (x >= 0 && x <= 8 * SQUARE_SIZE && y >= 0 && y <= 8 * SQUARE_SIZE)
        yellowOutlineIndex = getSquareIndex(x, y);
      else
        yellowOutlineIndex = NO_SQUARE;

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
    int squareIndex = 0;
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
    for (int i = 0; i < 64; i++)
    {
      for (int j = 0; j < PIECE_NUMBER; j++)
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
    for (int i = 0; i < 64; i++)
    {
      window->draw(boardSquares[i]);
    }
  }

  void GUIHandler::drawPieces()
  {
    for (int i = 0; i < 64; i++)
    {
      if (draggingPieceIndex == i || (awaitingPromotion && promotionMove.from == i))
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
    for (int i = 0; i < 64; i++)
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

  void GUIHandler::clearHighlights(int highlight)
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
    if (move.from == move.to)
      return;

    board.makeMove(move);

    clearHighlights();

    if (board.isInCheck(board.sideToMove()))
      Bitboards::addBit(redHighlightsBitboard, board.kingIndex(board.sideToMove() | KING));

    int gameStatus = board.getGameStatus(board.sideToMove());

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

    Bitboards::addBit(yellowHighlightsBitboard, move.from);
    Bitboards::addBit(yellowHighlightsBitboard, move.to);
  }

  void GUIHandler::makeBotMove()
  {
    Move move = bot.generateBotMove();

    makeMove(move);

    stopThinking();
  }

  void GUIHandler::saveBufferBoard()
  {
    for (int i = 0; i < 64; i++)
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