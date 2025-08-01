#pragma once

#include <vector>
#include <mutex>
#include <chrono>
#include <print>

#define TIME_TEST(n, x)                                                                         \
  do                                                                                            \
  {                                                                                             \
    auto start = std::chrono::high_resolution_clock::now();                                     \
    for (int i = 0; i < n; i++)                                                                 \
      x;                                                                                        \
    auto end = std::chrono::high_resolution_clock::now();                                       \
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);          \
    std::println("{:d} ns elapsed on average across {:d} iterations", duration.count() / n, n); \
  } while (0)

namespace TungstenChess
{
  namespace utils
  {
    /**
     * @brief A simple flag that can be used to store a boolean value that will take a value (B) once
     *        and then toggle to the opposite value permanently. This can be used to trigger an event
     *        once, such as a one-time static class initialization. The flag is also thread-safe.
     * @tparam B The value to take once.
     *           If true, the flag will take the value true once and then toggle to false, and vice versa.
     */
    template <bool B>
    struct once
    {
      /**
       * @brief Casting the once flag to a bool will return the current value of the flag and toggle it.
       */
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

      /**
       * @brief Triggering the once flag will toggle its value immediately.
       */
      void trigger()
      {
        std::lock_guard<std::mutex> lock(mtx);
        value = !B;
      }

      /**
       * @brief Peek at the current value of the flag without toggling it.
       * @return The current value of the flag.
       */
      bool peek()
      {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
      }

    private:
      bool value = B;
      std::mutex mtx;
    };

    /**
     * @brief A simple boolean flag that can be used to store a boolean value that can be set and
     *        unset. This is useful for flags that need to be set and unset multiple times, but it
     *        is not thread-safe.
     */
    struct bool_flag
    {
      /**
       * @brief Casting the bool_flag to a bool will return the current value of the flag.
       */
      operator bool() const { return value; }

      /**
       * @brief Set the flag to true.
       */
      void set_flag() { value = true; }

      /**
       * @brief Unset the flag and return its previous value.
       */
      bool pop_flag()
      {
        bool temp = value;
        value = false;
        return temp;
      }

    private:
      bool value = false;
    };

    /**
     * @brief A 2D array implementation using a flat heap allocation to reduce memory fragmentation.
     *        This is useful for storing large 2D arrays in contiguous heap memory.
     * @tparam T The type of the elements in the array.
     * @tparam R The number of rows in the array.
     * @tparam C The number of columns in the array.
     */
    template <typename T, size_t R, size_t C>
    struct array2d
    {
      array2d() : m_data(new T[R * C]) {}

      ~array2d() { delete[] m_data; }

      /**
       * @brief Access an element in the array using row and column indices.
       * @param r The row index.
       * @param c The column index.
       * @return A reference to the element at the specified row and column.
       */
      T &operator[](size_t r, size_t c) { return m_data[r * C + c]; }

      /**
       * @brief Access an element in the array using row and column indices.
       * @param r The row index.
       * @param c The column index.
       * @return A const reference to the element at the specified row and column.
       */
      const T &operator[](size_t r, size_t c) const { return m_data[r * C + c]; }

      /**
       * @brief Copy a row from another array2d to this array2d.
       * @param other The other array2d to copy from.
       * @param src The source row index in the other array2d.
       * @param dst The destination row index in this array2d.
       * @note If the source and destination are the same, and other is the same as this, no operation is performed.
       * @note If the source or destination indices are out of bounds, no operation is performed.
       * @note If the two arrays have different column sizes, no operation is performed.
       */
      void copyRow(const array2d &other, size_t src, size_t dst)
      {
        if (this == &other && src == dst)
          return;

        if (src > other.m_rows || dst > R || other.m_cols != C)
          return;

        std::copy(&other.m_data[src * C], &other.m_data[src * C + C], &m_data[dst * C]);
      }

      /**
       * @brief Copy a row from this array2d to itself.
       * @param src The source row index in this array2d.
       * @param dst The destination row index in this array2d.
       * @note If the source and destination are the same, no operation is performed.
       * @note If the source or destination indices are out of bounds, no operation is performed.
       */
      void copyRow(size_t src, size_t dst)
      {
        copyRow(*this, src, dst);
      }

    private:
      T *m_data;
      const size_t m_rows = R;
      const size_t m_cols = C;
    };

    /**
     * @brief This class simulates stack memory, but is allocated on the heap. It supports
     *        dynamic top allocation, which can be used to allocate memory at the top of the stack
     *        in a self-managed way. This can be used for recursive algorithms that need to allocate
     *        dynamic memory for each recursive call without the added overhead of allocating new heap memory
     *        for each call. It also has random access and iterator (bottom-to-top) support.
     * @tparam T_Element The type of the elements in the stack.
     * @note This class is not thread-safe, and should be used in a single-threaded context.
     * @note This class can be used as a replacement for std::stack, but this is not recommended.
     */
    template <typename T_Element>
    class auxiliary_stack
    {
    private:
      T_Element *m_data;
      size_t m_top = 0;

    public:
      auxiliary_stack(size_t size) : m_data(new T_Element[size]) {}
      ~auxiliary_stack() { delete[] m_data; }

      auxiliary_stack(const auxiliary_stack &other) : m_data(new T_Element[other.m_top]), m_top(other.m_top)
      {
        std::copy(other.m_data, other.m_data + other.m_top, m_data);
      }

      auxiliary_stack(auxiliary_stack &&other) noexcept
          : m_data(other.m_data), m_top(other.m_top)
      {
        other.m_data = nullptr;
        other.m_top = 0;
      }

      void push(T_Element value) { m_data[m_top++] = value; }
      T_Element pop() { return m_data[--m_top]; }

      void clear() { m_top = 0; }

      T_Element &at(size_t index) { return m_data[index]; }
      const T_Element &at(size_t index) const { return m_data[index]; }

      T_Element &operator[](size_t index) { return m_data[index]; }
      const T_Element &operator[](size_t index) const { return m_data[index]; }

      T_Element &top() { return m_data[m_top - 1]; }
      const T_Element &top() const { return m_data[m_top - 1]; }

      size_t size() const { return m_top; }

      T_Element *begin() { return m_data; }
      T_Element *end() { return m_data + m_top; }

      const T_Element *begin() const { return m_data; }
      const T_Element *end() const { return m_data + m_top; }

      /**
       * @brief This class can be used to manage memory at the top of an auxiliary_stack.
       *        It maintains a reference to the parent stack, and will automatically "free"
       *        upon destruction or when the free() method is called.
       * @note Only the most recently allocated dynamic_top_allocation should be modified/freed at a time.
       *       Modifying multiple instances at once (e.g. pushing to two allocations) can lead to overwriting data.
       *       This pattern will likely arise naturally in recursive functions.
       * @note This class is not thread-safe, and should be used in a single-threaded context.
       */
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
        T_Element pop() { return m_stack.pop(); }

        T_Element operator[](size_t index) const { return m_stack[m_base + index]; }
        T_Element &top() const { return m_stack.top(); }

        size_t size() const { return m_stack.size() - m_base; }

        T_Element *begin() { return &m_stack[m_base]; }
        T_Element *end() { return m_stack.end(); }

        const T_Element *begin() const { return &m_stack[m_base]; }
        const T_Element *end() const { return m_stack.end(); }
      };
    };
  }
}