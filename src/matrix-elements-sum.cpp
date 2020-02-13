#include <benchmark/benchmark.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

static constexpr const int g_max_size(16384);

#ifndef NDEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...) do {} while (false)
#endif

static int g_seed;

std::vector<std::vector<int>> random_matrix(int s)
{
  static std::unordered_map<int, std::vector<std::vector<int>>> cache;

  const auto it(cache.find(s));

  if (it != cache.end())
    return it->second;
  
  boost::random::mt19937 gen(g_seed);
  boost::random::uniform_int_distribution<int> rg;
  
  const int row_size(s);
  const int row_count(s);
  const int max_value(100);

  std::vector<std::vector<int>> matrix(row_count);

  for (int y(0); y != row_count; ++y)
    {
      std::vector<int>& row(matrix[y]);
      row.resize(row_size);

      for (int x(0); x != row_size; ++x)
        if (rg(gen) % s >= y)
          row[x] = 1 + rg(gen) % (max_value - 1);
        else
          row[x] = 0;
    }

#ifndef NDEBUG
  if (s <= 128)
    for (int y(0); y != row_count; ++y)
      {
        for (int v : matrix[y])
          fprintf(stderr, "|% 3d ", v);

        fprintf(stderr, "|\n");
      }
#endif
  
  cache[s] = matrix;
  
  return matrix;
}

std::vector<int> random_contiguous_matrix(int s)
{
  auto value_count = s * s;
  std::vector<int> matrix(value_count);
  auto it = matrix.begin();

  for(const auto& row: random_matrix(s))
    {
    for(const auto& value: row)
        {
            *it++ = value;
        }
    }

  return matrix;
}

int matrix_elements_sum_branchless(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_column(row_size, 1);
    
  int result(0);

  for (const std::vector<int>& row : matrix)
    for (int i(0); i != row_size; ++i)
      {
        const int v(row[i]);
        result += v * usable_column[i];
        usable_column[i] &= char(v != 0);
      }
    
  return result;
}

int matrix_elements_sum_branchless_2
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_column(row_size, -1);
    
  int result(0);

  for (const std::vector<int>& row : matrix)
    for (int i(0); i != row_size; ++i)
      {
        const int v(row[i]);
        result += v & usable_column[i];
        usable_column[i] &= v ? -1 : 0;
      }
    
  return result;
}
int matrix_elements_sum_branchless_return_early
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<char> usable_column(row_size, 1);
  int remaining(row_size);
  
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      for (int i(0); i != row_size; ++i)
        {
          const int v(row[i]);
          const int usable(usable_column[i]);
          
          result += v * usable;
          remaining -= (usable == 1) & (v == 0);
          usable_column[i] &= char(v != 0);
        }

      if (remaining == 0)
        return result;
    }
    
  return result;
}


int matrix_elements_sum_branchless_2_return_early
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_column(row_size, -1);
  int remaining(row_size);
  
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      for (int i(0); i != row_size; ++i)
        {
          const int v(row[i]);
          const int usable(usable_column[i]);
          
          result += v & usable;
          remaining -= (usable != 0) & (v == 0);
          usable_column[i] &= v ? -1 : 0;
        }

      if (remaining == 0)
        return result;
    }
    
  return result;
}

int matrix_elements_sum_branchless_vector_of_bool
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<bool> usable_column(row_size, true);
    
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    for (int i(0); i != row_size; ++i)
      {
        const int v(row[i]);
        auto usable(usable_column[i]);
        result += v * usable;
        usable = usable & (v != 0);
      }
    
  return result;
}

int matrix_elements_sum_branches(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<char> usable_column(row_size, 1);
    
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    for (int i(0); i != row_size; ++i)
      if (usable_column[i])
        {
          const int v(row[i]);

          if (v == 0)
            usable_column[i] = 0;
          else
            result += v;
      }
    
  return result;
}

int matrix_elements_sum_branches_vector_of_bool
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<bool> usable_column(row_size, true);
    
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    for (int i(0); i != row_size; ++i)
      if (usable_column[i])
        {
          const int v(row[i]);

          if (v == 0)
            usable_column[i] = false;
          else
            result += v;
      }
    
  return result;
}

int matrix_elements_sum_branches_range
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<char> usable_column(row_size, 1);

  int first(0);
  int last(row_size);
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      for (; (first != last)
             && ((usable_column[first] == 0) || (row[first] == 0));
           ++first);

      int last_non_zero(first);
          
      for (int i(first); i != last; ++i)
        if (usable_column[i])
          {
            const int v(row[i]);

            if (v == 0)
              usable_column[i] = 0;
            else
              {
                result += v;
                last_non_zero = i;
              }
          }

      last = last_non_zero + 1;
    }
    
  return result;
}

int matrix_elements_sum_branches_range_return_early
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<char> usable_column(row_size, 1);

  int first(0);
  int last(row_size);
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      for (; (first != last)
             && ((usable_column[first] == 0) || (row[first] == 0));
           ++first);

      if (first == last)
        return result;
      
      int last_non_zero(first);
          
      for (int i(first); i != last; ++i)
        if (usable_column[i])
          {
            const int v(row[i]);

            if (v == 0)
              usable_column[i] = 0;
            else
              {
                result += v;
                last_non_zero = i;
              }
          }

      last = last_non_zero + 1;
    }
    
  return result;
}

int matrix_elements_sum_indices(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  std::iota(usable_columns_begin, usable_columns_end, 0);

  int result(0);
    
  for (const std::vector<int>& row : matrix)
    for (auto it(usable_columns_begin); it != usable_columns_end;)
      {
        const int i(*it);
        const int v(row[i]);
          
        if (v == 0)
          {
            --usable_columns_end;
            *it = *usable_columns_end;
          }
        else
          {
            result += v;
            ++it;
          }
      }
    
  return result;
}

int matrix_elements_sum_indices_branchless
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());

  std::iota(usable_columns_begin, usable_columns_end, 0);

  int remaining_count(row_size);
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      for (auto it(usable_columns_begin); it != usable_columns_end; )
        {
          const int i(*it);
          const int v(row[i]);
          result += v;

          const int keep_mask((v == 0) ? 0 : -1);

          usable_columns_end += ~keep_mask;

          const int distance_to_last(usable_columns_end - it);
          const auto new_i_it(it + (distance_to_last & ~keep_mask));
          *it = *new_i_it;
          
          it += -keep_mask;
        }
    }
    
  return result;
}

int matrix_elements_sum_indices_almost_branchless
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());

  std::iota(usable_columns_begin, usable_columns_end, 0);

  int remaining_count(row_size);
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      for (auto it(usable_columns_begin); it != usable_columns_end; )
        {
          const int i(*it);
          const int v(row[i]);
          result += v;

          const int keep_mask((v == 0) ? 0 : -1);

          usable_columns_end += ~keep_mask;

          if (~keep_mask)
            *it = *usable_columns_end;
          
          it += -keep_mask;
        }
    }
    
  return result;
}

int matrix_elements_sum_indices_sort
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  std::iota(usable_columns_begin, usable_columns_end, 0);

  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      for (auto it(usable_columns_begin); it != usable_columns_end;)
        {
          const int i(*it);
          const int v(row[i]);
          
          if (v == 0)
            {
              --usable_columns_end;
              *it = *usable_columns_end;
            }
          else
            {
              result += v;
              ++it;
            }
        }

      std::sort(usable_columns_begin, usable_columns_end);
    }
    
  return result;
}

int matrix_elements_sum_indices_erase
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  std::iota(usable_columns_begin, usable_columns_end, 0);

  int result(0);
    
  for (const std::vector<int>& row : matrix)
    for (auto it(usable_columns_begin); it != usable_columns_end; )
      {
        const int i(*it);
        const int v(row[i]);
          
        if (v == 0)
          {
            it = usable_columns.erase(it);
            usable_columns_end = usable_columns.end();
          }
        else
          {
            result += v;
            ++it;
          }
      }
    
  return result;
}

int matrix_elements_sum_indices_set
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::set<int> usable_columns;

  for (int i(0); i != row_size; ++i)
    usable_columns.insert(i);

  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      const auto usable_columns_begin(usable_columns.begin());
      auto usable_columns_end(usable_columns.end());

      for (auto it(usable_columns_begin); it != usable_columns_end;)
      {
        const int i(*it);
        const int v(row[i]);
          
        if (v == 0)
          {
            it = usable_columns.erase(it);
            usable_columns_end = usable_columns.end();
          }
        else
          {
            result += v;
            ++it;
          }
      }
    }
    
  return result;
}

int matrix_elements_sum_indices_unordered_set
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::unordered_set<int> usable_columns;

  for (int i(0); i != row_size; ++i)
    usable_columns.insert(i);

  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      const auto usable_columns_begin(usable_columns.begin());
      auto usable_columns_end(usable_columns.end());

      for (auto it(usable_columns_begin); it != usable_columns_end;)
      {
        const int i(*it);
        const int v(row[i]);
          
        if (v == 0)
          {
            it = usable_columns.erase(it);
            usable_columns_end = usable_columns.end();
          }
        else
          {
            result += v;
            ++it;
          }
      }
    }
    
  return result;
}

int matrix_elements_sum_indices_return_early
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  std::iota(usable_columns_begin, usable_columns_end, 0);

  int result(0);
    
  for (const std::vector<int>& row : matrix)
    {
      for (auto it(usable_columns_begin); it != usable_columns_end;)
        {
          const int i(*it);
          const int v(row[i]);
          
          if (v == 0)
            {
              --usable_columns_end;
              *it = *usable_columns_end;
            }
          else
            {
              result += v;
              ++it;
            }
        }

      if (usable_columns_begin == usable_columns_end)
        return result;
    }
    
  return result;
}

static std::array<int, g_max_size> g_indices;

int matrix_elements_sum_indices_no_iota
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns
    (g_indices.begin(), g_indices.begin() + row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  int result(0);
    
  for (const std::vector<int>& row : matrix)
    for (auto it(usable_columns_begin); it != usable_columns_end;)
      {
        const int i(*it);
        const int v(row[i]);
          
        if (v == 0)
          {
            --usable_columns_end;
            *it = *usable_columns_end;
          }
        else
          {
            result += v;
            ++it;
          }
      }
    
  return result;
}

int matrix_elements_sum_indices_16(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<std::int16_t> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  std::iota(usable_columns_begin, usable_columns_end, 0);

  int result(0);
    
  for (const std::vector<int>& row : matrix)
    for (auto it(usable_columns_begin); it != usable_columns_end;)
      {
        const std::int16_t i(*it);
        const int v(row[i]);
          
        if (v == 0)
          {
            --usable_columns_end;
            *it = *usable_columns_end;
          }
        else
          {
            result += v;
            ++it;
          }
      }
    
  return result;
}

int matrix_elements_sum_indices_4_by_4
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  std::iota(usable_columns_begin, usable_columns_end, 0);
  
  int result(0);

  int r(0);
  for (const std::vector<int>& row : matrix)
    {
      auto it(usable_columns_begin);

      while (it + 4 <= usable_columns_end)
      {
        const auto it_1(it + 1);
        const auto it_2(it + 2);
        const auto it_3(it + 3);

        const int i_0(*it);
        const int i_1(*it_1);
        const int i_2(*it_2);
        const int i_3(*it_3);
        
        const int v_0(row[i_0]);
        const int v_1(row[i_1]);
        const int v_2(row[i_2]);
        const int v_3(row[i_3]);

        result += v_0 + v_1 + v_2 + v_3;

        const bool is_zero_0(v_0 == 0);
        const bool is_zero_1(v_1 == 0);
        const bool is_zero_2(v_2 == 0);
        const bool is_zero_3(v_3 == 0);

        *it = !is_zero_0 * i_0
          + is_zero_0 * !is_zero_1 * i_1
          + is_zero_0 * is_zero_1 * !is_zero_2 * i_2
          + is_zero_0 * is_zero_1 * is_zero_2 * !is_zero_3 * i_3;

        *it_1 =
          is_zero_0
          * (!is_zero_1
             * (!is_zero_2 * i_2 + is_zero_2 * i_3)
             + is_zero_1 * (!is_zero_2 * i_3))
          + !is_zero_0
          * (!is_zero_1 * i_1
             + is_zero_1 * (!is_zero_2 * i_2 + is_zero_2 * i_3));
        
        *it_2 =
          is_zero_0
          * (!is_zero_1 * (!is_zero_2 * i_3))
          + !is_zero_0
          * (is_zero_1 * i_3
             + !is_zero_1 * (!is_zero_2 * i_2 + is_zero_2 * i_3));

        const int zero_count(is_zero_0 + is_zero_1 + is_zero_2 + is_zero_3);
        
        const auto end(it + 4);
        auto first(it + 4 - zero_count);
        it = first;
        
        while (first != end)
          {
            --usable_columns_end;
            *first = *usable_columns_end;
            ++first;
          }
      }
      
      while (it < usable_columns_end)
        {
          const int i(*it);
          const int v(row[i]);
          
          if (v == 0)
            {
              --usable_columns_end;
              *it = *usable_columns_end;
            }
          else
            {
              result += v;
              ++it;
            }
        }
      
      ++r;
    }
  
  return result;
}

int matrix_elements_sum_indices_4_by_4_partition
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  std::iota(usable_columns_begin, usable_columns_end, 0);
  
  int result(0);

  for (const std::vector<int>& row : matrix)
    {
      auto it(usable_columns_begin);

      while (it + 4 <= usable_columns_end)
      {
        const auto it_1(it + 1);
        const auto it_2(it + 2);
        const auto it_3(it + 3);

        const int v_0(row[*it]);
        const int v_1(row[*it_1]);
        const int v_2(row[*it_2]);
        const int v_3(row[*it_3]);

        result += v_0 + v_1 + v_2 + v_3;

        const auto end(it + 4);
        auto split
          (std::partition
           (it, end, [&](int i) -> bool { return row[i] != 0; } ) );
        it = split;
        
        while (split != end)
          {
            --usable_columns_end;
            *split = *usable_columns_end;
            ++split;
          }
      }
      
      while (it < usable_columns_end)
        {
          const int i(*it);
          const int v(row[i]);
          
          if (v == 0)
            {
              --usable_columns_end;
              *it = *usable_columns_end;
            }
          else
            {
              result += v;
              ++it;
            }
        }
    }
  
  return result;
}

int matrix_elements_sum_indices_8_by_8_partition
(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<int> usable_columns(row_size);

  const auto usable_columns_begin(usable_columns.begin());
  auto usable_columns_end(usable_columns.end());
  
  std::iota(usable_columns_begin, usable_columns_end, 0);
  
  int result(0);

  for (const std::vector<int>& row : matrix)
    {
      auto it(usable_columns_begin);

      while (it + 8 <= usable_columns_end)
      {
        const auto it_1(it + 1);
        const auto it_2(it + 2);
        const auto it_3(it + 3);
        const auto it_4(it + 1);
        const auto it_5(it + 2);
        const auto it_6(it + 3);
        const auto it_7(it + 3);

        const int v_0(row[*it]);
        const int v_1(row[*it_1]);
        const int v_2(row[*it_2]);
        const int v_3(row[*it_3]);
        const int v_4(row[*it_4]);
        const int v_5(row[*it_5]);
        const int v_6(row[*it_6]);
        const int v_7(row[*it_7]);

        result += v_0 + v_1 + v_2 + v_3 + v_4 + v_5 + v_6 + v_7;

        const auto end(it + 8);
        auto split
          (std::partition
           (it, end, [&](int i) -> bool { return row[i] != 0; } ) );
        it = split;
        
        while (split != end)
          {
            --usable_columns_end;
            *split = *usable_columns_end;
            ++split;
          }
      }
      
      while (it < usable_columns_end)
        {
          const int i(*it);
          const int v(row[i]);
          
          if (v == 0)
            {
              --usable_columns_end;
              *it = *usable_columns_end;
            }
          else
            {
              result += v;
              ++it;
            }
        }
    }
  
  return result;
}

int matrix_elements_sum_best_ratings(const std::vector<std::vector<int>>& m)
{
  int r(0);
  
  for (int j=0; j<(int)m[0].size(); j++)
    for (int i=0; i<(int)m.size(); i++)
      {
        if (m[i][j]==0)
          break;
        r += m[i][j];
      }
  
  return r;
}

int matrix_elements_sum_per_column_branchless
(const std::vector<std::vector<int>>& matrix)
{
  const int column_size(matrix.size());
  const int row_size(matrix[0].size());
  
  int r(0);
  
  for (int j = 0; j != row_size; ++j)
    {
      int still_no_zero(1);
      
      for (int i = 0; i != column_size; ++i)
        {
          const int v(matrix[i][j]);

          still_no_zero = still_no_zero * (v != 0);
          r += v * still_no_zero;
      }
    }
  
  return r;
}

int matrix_elements_sum_column_major(const std::vector<std::vector<int>>& m)
{
  int r(0);

  for (const std::vector<int>& column : m)
    for (int v : column)
      {
        if (v == 0)
          break;

        r += v;
      }
  
  return r;
}

void row_major_to_column_major(std::vector<std::vector<int>>& matrix)
{
  const int n(matrix.size());

  for (int y(1); y != n; ++y)
    for (int x(0); x != y; ++x)
      std::swap(matrix[y][x], matrix[x][y]);
}

int matrix_elements_sum_mixed(const std::vector<std::vector<int>>& m)
{
  const int n(m.size());
  
  if (n < 64)
    {
      std::vector<std::vector<int>> column_major(m);
      row_major_to_column_major(column_major);
      return matrix_elements_sum_column_major(column_major);
    }

  if (n < 2048)
    return matrix_elements_sum_best_ratings(m);
  
  return matrix_elements_sum_indices(m);
}

int contiguous_matrix_elements_sum_best_ratings(const std::vector<int>& m, int dim)
{
  int r(0);
  
  for (int j=0; j<dim; j++)
    for (int i=j; i<m.size(); i+=dim)
      {
        if (m[i]==0)
          break;
        r += m[i];
      }
  
  return r;
}

int contiguous_matrix_elements_sum_branchless(const std::vector<int>& m, int dim)
{
  std::vector<int> usable_column(dim, 1);

  int result(0);

  for (int i=0; i<m.size(); i+=dim)
    {
    for (int j=0; j<dim; j++)
      {
        const int v(m[i+j]);
        result += v * usable_column[j];
        usable_column[j] &= char(v != 0);
      }
    }

  return result;
}

int contiguous_matrix_elements_sum_branchless_2(const std::vector<int>& m, int dim)
{
  std::vector<int> usable_column(dim, -1);

  int result(0);

  for (int i=0; i<m.size(); i+=dim)
    {
    for (int j=0; j<dim; j++)
      {
        const int v(m[i+j]);
        result += v & usable_column[j];
        usable_column[j] &= v ? -1 : 0;
      }
    }

  return result;
}

template<typename F>
void run_test(benchmark::State& state, F f)
{
  const auto matrix(random_matrix(state.range(0)));

  for (auto _ : state)
    benchmark::DoNotOptimize(f(matrix));
}

template<typename F>
void run_test_contigous(benchmark::State& state, F f)
{
  int dim = state.range(0);
  const auto matrix(random_contiguous_matrix(dim));

  for (auto _ : state)
    benchmark::DoNotOptimize(f(matrix, dim));
}

#define declare_test(name)                                      \
  static void name(benchmark::State& state)                     \
  {                                                             \
    run_test(state, &matrix_elements_sum_ ## name);             \
  }                                                             \
                                                                \
  BENCHMARK(name)->RangeMultiplier(2)->Range(2, g_max_size)

#define declare_contiguous_test(name)                                     \
  static void contiguous_##name(benchmark::State& state)                  \
  {                                                                       \
    run_test_contigous(state, &contiguous_matrix_elements_sum_ ## name);  \
  }                                                                       \
                                                                          \
  BENCHMARK(contiguous_ ## name)->RangeMultiplier(2)->Range(2, g_max_size)

declare_test(branchless);
//declare_contiguous_test(branchless);

declare_test(branchless_2);
//declare_contiguous_test(branchless_2);


//declare_test(branchless_vector_of_bool);
//declare_test(branchless_return_early);
//declare_test(branchless_2_return_early);

//declare_test(branches);
//declare_test(branches_vector_of_bool);
//declare_test(branches_range);
//declare_test(branches_range_return_early);

declare_test(indices);
declare_test(indices_branchless);
declare_test(indices_almost_branchless);
//declare_test(indices_sort);
//declare_test(indices_erase);
//declare_test(indices_set);
//declare_test(indices_unordered_set);
//declare_test(indices_return_early);
//declare_test(indices_no_iota);
//declare_test(indices_16);
//declare_test(indices_4_by_4);
//declare_test(indices_4_by_4_partition);
//declare_test(indices_8_by_8_partition);

//declare_test(per_column_branchless);

//declare_test(mixed);

declare_test(best_ratings);
//declare_contiguous_test(best_ratings);

#if 0
void column_major(benchmark::State& state)
{
  auto matrix(random_matrix(state.range(0)));

  row_major_to_column_major(matrix);
      
  for (auto _ : state)
    benchmark::DoNotOptimize(matrix_elements_sum_column_major(matrix));
}

BENCHMARK(column_major)->RangeMultiplier(2)->Range(2, g_max_size);
#endif

#if 1
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/assignment.hpp>
#include <boost/numeric/ublas/io.hpp>

boost::numeric::ublas::matrix<int> row_major_to_boost_ublas
(const std::vector<std::vector<int>>& matrix)
{
  const int size(matrix.size());
  boost::numeric::ublas::matrix<int> result(size, size);

  for (int j(0); j != size; ++j)
    for (int i(0); i != size; ++i)
      result(j, i) = matrix[j][i];

  return result;
}

int matrix_elements_sum_boost_ublas
(boost::numeric::ublas::matrix<int>& matrix)
{
  int sum(0);
  int size(matrix.size1());
  
  for (unsigned i = 0; i != size; ++i)
    sum += matrix(0, i);
  
  for (unsigned j = 1; j < size; ++j)
    for (unsigned i = 0; i != size; ++i)
      if( matrix(j-1, i) )
        sum += matrix(j, i);
      else
        matrix(j, i) = 0;

  return sum;
}

void boost_ublas(benchmark::State& state)
{
  const auto matrix(random_matrix(state.range(0)));

  const boost::numeric::ublas::matrix<int> m(row_major_to_boost_ublas(matrix));
  const auto end(std::end(state));

  for (auto it(std::begin(state)); it != end; ++it)
    {
      boost::numeric::ublas::matrix<int> matrix_copy(m);
      
      {
        auto _(*it);
        benchmark::DoNotOptimize(matrix_elements_sum_boost_ublas(matrix_copy));
      }
    }
}

BENCHMARK(boost_ublas)->RangeMultiplier(2)->Range(2, g_max_size);
#endif

int main(int argc, char** argv)
{
  if (argc == 1)
    {
      boost::random::random_device rd;
      g_seed = rd();
    }
  else
    {
      if ((argc != 2) || (std::strcmp(argv[1], "--help") == 0))
        {
          printf("Usage: %s SEED\n", argv[0]);
          return 0;
        }

      if (std::strcmp(argv[1], "--test") == 0)
        {
          g_seed = 1234;
          const int size(16);
          auto matrix(random_matrix(size));

          printf("branchless %d\n", matrix_elements_sum_branchless(matrix));
          printf("branchless_2 %d\n", matrix_elements_sum_branchless_2(matrix));
          printf("indices %d\n", matrix_elements_sum_indices(matrix));
          printf
            ("contiguous branchless %d\n",
             contiguous_matrix_elements_sum_branchless
             (random_contiguous_matrix(size), size));
          printf
            ("indices_branchless %d\n",
             matrix_elements_sum_indices_branchless(matrix));
          
          return 0;
        }
      g_seed = std::stoi(argv[1]);
    }
  
  benchmark::Initialize(&argc, argv);

  std::iota(g_indices.begin(), g_indices.end(), 0);
  
  benchmark::RunSpecifiedBenchmarks();
  
  return 0;
}
