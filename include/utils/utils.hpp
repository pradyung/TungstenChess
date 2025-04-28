#pragma once

#include <vector>
#include <mutex>
#include <chrono>
#include <iostream>

#define TIME_TEST(n, x)                                                                                          \
  {                                                                                                              \
    auto start = std::chrono::high_resolution_clock::now();                                                      \
    for (int i = 0; i < n; i++)                                                                                  \
      x;                                                                                                         \
    auto end = std::chrono::high_resolution_clock::now();                                                        \
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);                           \
    std::cout << (duration.count() / n) << " ns elapsed on average across " << #n << " iterations" << std::endl; \
  }

namespace TungstenChess
{
  namespace utils
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

    template <typename T, size_t R, size_t C>
    struct array2d // 2d array implemented using flat std::array
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
      std::vector<T> m_data;
      const size_t m_rows = R;
      const size_t m_cols = C;
    };

    template <typename T_Element>
    class auxiliary_stack
    {
    private:
      T_Element *m_data;
      size_t m_top = 0;

    public:
      auxiliary_stack(size_t size) : m_data(new T_Element[size]) {}
      ~auxiliary_stack() { delete[] m_data; }

      void push(T_Element value) { m_data[m_top++] = value; }
      T_Element pop() { return m_data[--m_top]; }

      class dynamic_top_allocation
      {
      private:
        size_t m_base;
        auxiliary_stack &m_stack;

      public:
        using value_type = T_Element;
        using pointer = T_Element *;
        using reference = T_Element &;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        dynamic_top_allocation(auxiliary_stack &stack) : m_stack(stack), m_base(stack.m_top) {}

        ~dynamic_top_allocation() { free(); }

        void free() { m_stack.m_top = m_base; }

        void push(T_Element value) { m_stack.push(value); }

        T_Element operator[](size_t index) const { return m_stack.m_data[m_base + index]; }
        T_Element &top() const { return m_stack.m_data[m_stack.m_top - 1]; }

        size_t size() const { return m_stack.m_top - m_base; }

        T_Element *begin() { return &m_stack.m_data[m_base]; }
        T_Element *end() { return &m_stack.m_data[m_stack.m_top]; }

        const T_Element *begin() const { return &m_stack.m_data[m_base]; }
        const T_Element *end() const { return &m_stack.m_data[m_stack.m_top]; }
      };
    };
  }
}