#include "GUI.hpp"

int main()
{
  sf::RenderWindow window(sf::VideoMode(SQUARE_SIZE * 8, SQUARE_SIZE * 8), "TungstenChess", sf::Style::Titlebar | sf::Style::Close);

  GUIHandler gui(window);
  gui.runMainLoop();
}