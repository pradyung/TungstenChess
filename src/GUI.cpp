#include "Headers/GUI.hpp"

using namespace Chess;

int main()
{
  RenderWindow window(VideoMode(SQUARE_SIZE * 8, SQUARE_SIZE * 8), "Chess", sf::Style::Titlebar | sf::Style::Close);

  GUIHandler gui(window);

  gui.runMainLoop();

  return 0;
}

namespace Chess
{
  GUIHandler::GUIHandler(RenderWindow &window)
  {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
      resourcePath = "";
    else
      resourcePath = std::string(path) + "/";
    CFRelease(resourcesURL);

    this->window = &window;

    board.loadOpeningBook(resourcePath + "opening_book.bin");

    loadSquareTextures();
    loadBoardSquares();

    loadPieceTextures();
    loadPieces();
    loadPromotionPieces();

    Image icon;
    icon.loadFromFile(resourcePath + "high_res_wn.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
  }

  void GUIHandler::runMainLoop()
  {
    while (window->isOpen())
    {
      Event event;

      if (!(board.sideToMove & PLAYER_COLOR) && !gameOver)
      {
        if (!isThinking)
          startThinking();
      }

      while (window->pollEvent(event))
      {
        if (event.type == Event::Closed)
        {
          window->close();
        }

        if (isThinking)
          continue;

        if (event.type == Event::MouseButtonPressed)
        {
          if (event.mouseButton.button == Mouse::Left && !gameOver)
          {
            int index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

            if (!(board.sideToMove & board[index]) && !awaitingPromotion)
              continue;

            if (awaitingPromotion)
            {
              int promotionPiece = getPromotionPiece(event.mouseButton.x, event.mouseButton.y);

              if (promotionPiece == INVALID || !(promotionPiece & board.sideToMove))
                continue;

              promotionMove.promotionPiece = promotionPiece;

              draggingPieceIndex = INVALID;

              awaitingPromotion = false;

              makeMove(promotionMove);
            }
            else
            {
              if (!(board.sideToMove & board[index]))
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
            if (draggingPieceIndex == INVALID)
              continue;

            if (!(grayHighlightsBitboard.hasBit(GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y))))
            {
              draggingPieceIndex = INVALID;
              clearHighlights(GRAY_HIGHLIGHT);
              continue;
            }

            int index = GUIHandler::getSquareIndex(event.mouseButton.x, event.mouseButton.y);

            Move move(draggingPieceIndex, index, board[draggingPieceIndex], board[index], board.castlingRights, board.enPassantFile);

            if (!(move.flags & PROMOTION))
            {
              makeMove(move);
            }
            else
            {
              awaitingPromotion = true;
              promotionMove = move;
            }

            draggingPieceIndex = INVALID;
          }
        }
      }

      int x = Mouse::getPosition(*window).x;
      int y = Mouse::getPosition(*window).y;
      if (x >= 0 && x <= 8 * SQUARE_SIZE && y >= 0 && y <= 8 * SQUARE_SIZE)
        yellowOutlineIndex = getSquareIndex(x, y);
      else
        yellowOutlineIndex = INVALID;

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

    square.create(80, 80, sf::Color(255, 255, 255));
    squares[WHITE_SQUARE].loadFromImage(square);
    square.create(80, 80, sf::Color(216, 181, 149));
    squares[BLACK_SQUARE].loadFromImage(square);
    square.create(80, 80, sf::Color(255, 255, 0, 127));
    squares[YELLOW_HIGHLIGHT].loadFromImage(square);
    square.create(80, 80, sf::Color(255, 0, 0, 200));
    squares[RED_HIGHLIGHT].loadFromImage(square);
    square.create(80, 80, sf::Color(127, 127, 127, 200));
    squares[GRAY_HIGHLIGHT].loadFromImage(square);

    squares[YELLOW_OUTLINE].loadFromFile(resourcePath + "yellow_outline.png");
  }

  void GUIHandler::loadBoardSquares()
  {
    int squareIndex = 0;
    for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
      {
        boardSquares[squareIndex].setTexture(squares[(i + j) % 2]);
        redHighlightsSprites[squareIndex].setTexture(squares[RED_HIGHLIGHT]);
        yellowHighlightsSprites[squareIndex].setTexture(squares[YELLOW_HIGHLIGHT]);
        grayHighlightsSprites[squareIndex].setTexture(squares[GRAY_HIGHLIGHT]);
        yellowOutlineSprites[squareIndex].setTexture(squares[YELLOW_OUTLINE]);

        boardSquares[squareIndex].setPosition(getSquareCoordinates(j, i));
        redHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        yellowHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        grayHighlightsSprites[squareIndex].setPosition(getSquareCoordinates(j, i));
        yellowOutlineSprites[squareIndex].setPosition(getSquareCoordinates(j, i));

        boardSquares[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
        redHighlightsSprites[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
        yellowHighlightsSprites[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
        grayHighlightsSprites[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
        yellowOutlineSprites[squareIndex].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);

        squareIndex++;
      }
    }
  }

  void GUIHandler::loadPieceTextures()
  {
    sf::Texture atlas;
    atlas.loadFromFile(resourcePath + "atlas.png");

    piecesTextures[WHITE_PAWN].loadFromImage(atlas.copyToImage(), sf::IntRect(0 * 333, 0 * 333, 333, 333));
    piecesTextures[WHITE_KNIGHT].loadFromImage(atlas.copyToImage(), sf::IntRect(1 * 333, 0 * 333, 333, 333));
    piecesTextures[WHITE_BISHOP].loadFromImage(atlas.copyToImage(), sf::IntRect(2 * 333, 0 * 333, 333, 333));
    piecesTextures[WHITE_ROOK].loadFromImage(atlas.copyToImage(), sf::IntRect(3 * 333, 0 * 333, 333, 333));
    piecesTextures[WHITE_QUEEN].loadFromImage(atlas.copyToImage(), sf::IntRect(4 * 333, 0 * 333, 333, 333));
    piecesTextures[WHITE_KING].loadFromImage(atlas.copyToImage(), sf::IntRect(5 * 333, 0 * 333, 333, 333));

    piecesTextures[BLACK_PAWN].loadFromImage(atlas.copyToImage(), sf::IntRect(0 * 333, 333, 333, 333));
    piecesTextures[BLACK_KNIGHT].loadFromImage(atlas.copyToImage(), sf::IntRect(1 * 333, 333, 333, 333));
    piecesTextures[BLACK_BISHOP].loadFromImage(atlas.copyToImage(), sf::IntRect(2 * 333, 333, 333, 333));
    piecesTextures[BLACK_ROOK].loadFromImage(atlas.copyToImage(), sf::IntRect(3 * 333, 333, 333, 333));
    piecesTextures[BLACK_QUEEN].loadFromImage(atlas.copyToImage(), sf::IntRect(4 * 333, 333, 333, 333));
    piecesTextures[BLACK_KING].loadFromImage(atlas.copyToImage(), sf::IntRect(5 * 333, 333, 333, 333));

    for (int i = 0; i < PIECE_NUMBER; i++)
    {
      piecesTextures[i].setSmooth(true);
    }
  }

  void GUIHandler::loadPieces()
  {
    for (int i = 0; i < 64; i++)
    {
      for (int j = 0; j < PIECE_NUMBER; j++)
      {
        pieceSprites[j][i].setTexture(piecesTextures[j]);
        pieceSprites[j][i].setPosition(getSquareCoordinates(i));
        pieceSprites[j][i].setScale(SQUARE_SIZE / 333.0f, SQUARE_SIZE / 333.0f);
      }

      draggingPieceSprite.setScale(SQUARE_SIZE / 333.0f, SQUARE_SIZE / 333.0f);
    }
  }

  void GUIHandler::loadPromotionPieces()
  {
    for (int i = 0; i < 4; i++)
    {
      whitePromotionPieces[i].setTexture(piecesTextures[WHITE_QUEEN - i]);
      blackPromotionPieces[i].setTexture(piecesTextures[BLACK_QUEEN - i]);

      whitePromotionPieces[i].setPosition(getSquareCoordinates(10 + i));
      blackPromotionPieces[i].setPosition(getSquareCoordinates(50 + i));

      whitePromotionPieces[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
      blackPromotionPieces[i].setScale(SQUARE_SIZE / 80.0f, SQUARE_SIZE / 80.0f);
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

    if (draggingPieceIndex != INVALID)
    {
      draggingPieceSprite.setTexture(piecesTextures[board[draggingPieceIndex]]);
      draggingPieceSprite.setPosition(Mouse::getPosition(*window).x - SQUARE_SIZE / 2, Mouse::getPosition(*window).y - SQUARE_SIZE / 2);
      window->draw(draggingPieceSprite);
    }
  }

  void GUIHandler::drawHighlights()
  {
    for (int i = 0; i < 64; i++)
    {
      if (redHighlightsBitboard.hasBit(i))
      {
        window->draw(redHighlightsSprites[i]);
      }
      else if (grayHighlightsBitboard.hasBit(i))
      {
        window->draw(grayHighlightsSprites[i]);
      }
      else if (yellowHighlightsBitboard.hasBit(i) || draggingPieceIndex == i)
      {
        window->draw(yellowHighlightsSprites[i]);
      }

      if (yellowOutlineIndex == i)
      {
        window->draw(yellowOutlineSprites[i]);
      }
    }
  }

  void GUIHandler::drawPromotionPieces()
  {
    for (int i = 0; i < 4; i++)
    {
      if (board.sideToMove == WHITE)
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
    redHighlightsBitboard.bitboard = 0;
    yellowHighlightsBitboard.bitboard = 0;
    grayHighlightsBitboard.bitboard = 0;
  }

  void GUIHandler::clearHighlights(int highlight)
  {
    if (highlight == RED_HIGHLIGHT)
    {
      redHighlightsBitboard.bitboard = 0;
    }
    else if (highlight == YELLOW_HIGHLIGHT)
    {
      yellowHighlightsBitboard.bitboard = 0;
    }
    else if (highlight == GRAY_HIGHLIGHT)
    {
      grayHighlightsBitboard.bitboard = 0;
    }
  }

  void GUIHandler::makeMove(Move move)
  {
    if (move.from == move.to)
      return;

    board.makeMove(move);

    clearHighlights();

    if (board.isInCheck(board.sideToMove))
      redHighlightsBitboard.addBit(board.kingIndices[board.sideToMove | KING]);

    int gameStatus = board.getGameStatus(board.sideToMove);

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

    yellowHighlightsBitboard.addBit(move.from);
    yellowHighlightsBitboard.addBit(move.to);
  }

  void GUIHandler::makeBotMove()
  {
    Move move = board.generateBotMove();

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