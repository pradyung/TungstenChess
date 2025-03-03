#include "GUI.hpp"

int main()
{
  RenderWindow window(VideoMode(SQUARE_SIZE * 8, SQUARE_SIZE * 8), "TungstenChess", sf::Style::Titlebar | sf::Style::Close);

  TungstenChess::GUIHandler gui(window);
  gui.runMainLoop();
}