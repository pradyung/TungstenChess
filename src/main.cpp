#include "Headers/GUI.hpp"

using namespace Chess;

int main()
{
  RenderWindow window(VideoMode(SQUARE_SIZE * 8, SQUARE_SIZE * 8), "Chess", sf::Style::Titlebar | sf::Style::Close);

  GUIHandler gui(window);

  gui.runMainLoop();

  return 0;
}