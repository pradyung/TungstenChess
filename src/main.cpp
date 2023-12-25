#include "Headers/GUI.hpp"

using namespace Chess;

int main()
{
  RenderWindow window(VideoMode(SQUARE_SIZE * 8 + WIDTH_PADDING * 2, SQUARE_SIZE * 8 + HEIGHT_PADDING * 2), "Chess", sf::Style::Titlebar | sf::Style::Close);

  GUIHandler gui(window);

  gui.runMainLoop();

  return 0;
}