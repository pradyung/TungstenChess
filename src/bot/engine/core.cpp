#include "bot/engine.hpp"

namespace TungstenChess
{
  Bot::Bot(Board &board, const BotSettings &settings) : m_board(board), m_openingBook(board.wasDefaultStartPosition()), m_botSettings(settings)
  {
    if (m_botSettings.maxSearchTime > 0)
    {
      m_searchTimerThread = std::thread(
          [this]()
          {
            std::unique_lock<std::mutex> lock(m_searchTimerMutex);
            m_searchTimerEvent.wait(lock);
            while (!m_searchTimerTerminated)
            {
              m_searchTimerReset = false;

              if (m_searchTimerEvent.wait_for(lock, std::chrono::milliseconds(m_maxSearchTime), [this]
                                              { return m_searchTimerReset.load(); }))
                continue;

              m_searchCancelled = true;
            }
          });

      m_searchTimerThread.detach();
    }
  }

  Bot::~Bot()
  {
    m_searchCancelled = true;
    m_searchTimerTerminated = true;
    m_searchTimerReset = true;
    if (m_searchTimerThread.joinable())
      m_searchTimerThread.join();
  }

  void Bot::loadOpeningBook(const std::filesystem::path path, uint openingBookSize)
  {
    if (!m_openingBookLoaded)
      m_openingBook.loadOpeningBook(path, openingBookSize);
  }
}