// UCI functionality is not currently supported/up to date.
// Disregard this file for now.
// This file is a placeholder for future UCI support.

#include <iostream>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "bot/engine.hpp"

using namespace TungstenChess;

std::filesystem::path getResourcePath()
{
#ifdef __APPLE__
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  char path[PATH_MAX];
  if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8*)path, PATH_MAX))
  {
    return "";
  }
  CFRelease(resourcesURL);
  return path;
#else
  return TUNGSTENCHESS_RESOURCES_DIR;
#endif
}

std::vector<std::string> split(std::string str, std::string delimiter)
{
  std::vector<std::string> splitString;

  size_t pos = 0;
  std::string token;
  while ((pos = str.find(delimiter)) != std::string::npos)
  {
    token = str.substr(0, pos);
    splitString.push_back(token);
    str.erase(0, pos + delimiter.length());
  }
  splitString.push_back(str);

  return splitString;
}

int main()
{
  Board board;

  Bot bot(board);

  std::filesystem::path openingBookPath = getResourcePath() / "opening_book.dat";

  bot.loadOpeningBook(openingBookPath);

  std::cout << "TungstenChess v1.0\n";

  while (true)
  {
    std::string input;
    std::getline(std::cin, input);

    if (input == "quit")
    {
      break;
    }

    if (input == "uci")
    {
      std::cout << "id name TungstenChess" << std::endl
                << "id author Pradyun Gaddam" << std::endl
                << "uciok" << std::endl;
      continue;
    }

    if (input == "isready")
    {
      std::cout << "readyok" << std::endl;
      continue;
    }

    if (input == "ucinewgame")
    {
      board.resetBoard();
      continue;
    }

    if (input == "d")
    {
      std::cout << "\n";
      for (int i = 0; i < 64; i++)
      {
        if (i % 8 == 0)
        {
          std::cout << " +---+---+---+---+---+---+---+---+\n ";
        }

        std::cout << "| "
                  << " ........PNBRQK..pnbrqk"[board[i]] << " ";

        if (i % 8 == 7)
        {
          std::cout << "| " << 8 - (i >> 3) << "\n";
        }
      }

      std::cout << " +---+---+---+---+---+---+---+---+\n";
      std::cout << "   a   b   c   d   e   f   g   h\n\n";

      std::cout << "Side to move: " << (board.sideToMove() == WHITE ? "White" : "Black") << "\n";
      std::cout << "Zobrist key: " << board.zobristKey() << "\n";
    }

    std::vector<std::string> splitInput = split(input, " ");

    if (splitInput[0] == "go")
    {
      Move bestMove = bot.generateBotMove();
      std::cout << "bestmove " << Moves::getUCI(bestMove) << "\n";

      if (splitInput[1] == "move")
      {
        board.makeMove(bestMove);
      }

      continue;
    }

    if (splitInput[0] == "position")
    {
      if (splitInput[1] == "startpos")
      {
        board.resetBoard();
      }
      else if (splitInput[1] == "fen")
      {
        std::string fen = "";
        for (int i = 2; i < splitInput.size(); i++)
        {
          fen += splitInput[i] + " ";
        }
        board.resetBoard(fen);
      }

      if (splitInput.size() > 2 && splitInput[2] == "moves")
      {
        for (int i = 3; i < splitInput.size(); i++)
        {
          board.makeMove(board.generateMoveFromUCI(splitInput[i]));
        }
      }
    }

    if (splitInput[0] == "moves")
    {
      for (int i = 1; i < splitInput.size(); i++)
      {
        board.makeMove(board.generateMoveFromUCI(splitInput[i]));
      }
    }
  }

  return 0;
}