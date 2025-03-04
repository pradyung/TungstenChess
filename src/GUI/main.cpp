#include "GUI.hpp"

using namespace sf;

int main()
{
  RenderWindow window(VideoMode(SQUARE_SIZE * 8, SQUARE_SIZE * 8), "TungstenChess", Style::Titlebar | Style::Close);

  TungstenChess::GUIHandler gui(window);
  gui.runMainLoop();
}