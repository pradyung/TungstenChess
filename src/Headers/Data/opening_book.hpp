#pragma once

#include <fstream>
#include <string>

namespace Chess
{
  class OpeningBook
  {
  public:
    void loadBook(std::string book_path)
    {
      std::ifstream file(book_path);

      for (int i = 0; i < 16552; i++)
      {
        file >> book[i];
      }

      file.close();
    }

    const int operator[](int index) const
    {
      return book[index];
    }

    int book[16552];
  };
}