#include "bot/engine.hpp"

namespace TungstenChess
{
  Bot::Bot(Board &board, const BotSettings &settings) : m_board(board), m_openingBook(board.wasDefaultStartPosition()), m_moveStack(AUXILIARY_MOVE_STACK_SIZE), m_botSettings(settings), m_transpositionTable(m_botSettings.transpositionTableSizeMB)
  {
    startSearchTimerThread();
  }

  Bot::~Bot()
  {
    stopSearchTimerThread();
  }

  void Bot::loadOpeningBook(const std::filesystem::path path)
  {
    if (!m_onceOpeningBookLoaded)
      m_openingBook.loadOpeningBook(path);
  }
}