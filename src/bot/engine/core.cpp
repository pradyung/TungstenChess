#include "bot/engine.hpp"

namespace TungstenChess
{
  Bot::Bot(Board &board, const BotSettings &settings) : m_board(board), m_openingBook(board.wasDefaultStartPosition()), m_botSettings(settings)
  {
    startSearchTimerThread();
  }

  Bot::~Bot()
  {
    stopSearchTimerThread();
  }

  void Bot::loadOpeningBook(const std::filesystem::path path, uint openingBookSize)
  {
    if (!m_onceOpeningBookLoaded)
      m_openingBook.loadOpeningBook(path, openingBookSize);
  }
}