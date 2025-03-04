#pragma once

#include <vector>
#include <mutex>

namespace TungstenChess
{
  template <bool B>
  struct once
  {
    operator bool()
    {
      std::lock_guard<std::mutex> lock(mtx);
      if (value == B)
      {
        value = !B;
        return B;
      }
      return !B;
    }

    void trigger()
    {
      std::lock_guard<std::mutex> lock(mtx);
      value = !B;
    }

    bool peek()
    {
      std::lock_guard<std::mutex> lock(mtx);
      return value;
    }

  private:
    bool value = B;
    std::mutex mtx;
  };

  struct bool_flag
  {
    operator bool() const { return value; }
    void set_flag() { value = true; }
    bool pop_flag()
    {
      bool temp = value;
      value = false;
      return temp;
    }

  private:
    bool value;
  };

  template <typename T, size_t R, size_t C, typename Allocator = std::allocator<T>>
  struct array2d // 2d array implemented as flat std::array
  {
    array2d() : m_data(R * C) {}

    T &operator[](size_t r, size_t c) { return m_data[r * C + c]; }
    const T &operator[](size_t r, size_t c) const { return m_data[r * C + c]; }

    void copyRow(const array2d &other, size_t src, size_t dst)
    {
      if (src == dst)
        return;

      if (src > other.m_rows || dst > R || other.m_cols != C)
        return;

      std::copy(&other.m_data[src * C], &other.m_data[src * C + C], &m_data[dst * C]);
    }

    void copyRow(size_t src, size_t dst)
    {
      copyRow(*this, src, dst);
    }

  private:
    std::vector<T, Allocator> m_data;
    const size_t m_rows = R;
    const size_t m_cols = C;
  };
}