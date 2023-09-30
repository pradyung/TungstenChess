#include "Headers/GUI.hpp"

using namespace Chess;

int main()
{
  RenderWindow window(VideoMode(GUIHandler::SQUARE_SIZE * 8 + GUIHandler::WIDTH_PADDING * 2, GUIHandler::SQUARE_SIZE * 8 + GUIHandler::HEIGHT_PADDING * 2), "Chess", sf::Style::Titlebar | sf::Style::Close);

  Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  GUIHandler gui(window, board);

  gui.runMainLoop();

  return 0;
}