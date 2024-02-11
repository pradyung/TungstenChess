#pragma once

#include <fstream>
#include <string>
#include <iostream>

namespace Chess
{
  class OpeningBook
  {
  public:
    void loadBook(std::string book_path)
    {
      std::ifstream file(book_path);

      // read the file 4 bytes at a time into the book array
      for (int i = 0; i < 16552; i++)
      {
        file.read((char *)&book[i], sizeof(int));
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