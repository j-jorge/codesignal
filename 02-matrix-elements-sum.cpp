#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

static constexpr const int g_max_size(16384);

#ifdef _DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...) do {} while (false)
#endif

std::vector<std::vector<int>> random_matrix(int s)
{
  static std::unordered_map<int, std::vector<std::vector<int>>> cache;

  const auto it(cache.find(s));

  if (it != cache.end())
    return it->second;
  
  std::random_device rd;
  static int seed(rd());
  
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> rg;
  
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

#ifdef _DEBUG
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

int matrix_elements_sum_branchless(const std::vector<std::vector<int>>& matrix)
{
  const int row_size(matrix[0].size());
  std::vector<char> usable_column(row_size, 1);
    
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
      dbg_printf("--\nrow %d\n", r);

      auto i(usable_columns_begin);
      for (; i != usable_columns_end; ++i)
        dbg_printf(" %d", *i);
      dbg_printf(" |");
      for (; i != usable_columns.end(); ++i)
        dbg_printf(" %d", *i);
      dbg_printf("\n");
      auto it(usable_columns_begin);

      dbg_printf("%d columns\n", int(usable_columns_end - usable_columns_begin));
      
      while (it + 4 <= usable_columns_end)
      {
        dbg_printf("at %d\n", int(it - usable_columns_begin));
        
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
      
      dbg_printf
        ("at %d on %d\n",
         int(it - usable_columns_begin),
         int(usable_columns_end - usable_columns_begin));
      
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
      
      dbg_printf("4 by 4 %d: %d\n", r, result);
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

template<typename F>
void run_test(benchmark::State& state, F f)
{
  const auto matrix(random_matrix(state.range(0)));

  for (auto _ : state)
    benchmark::DoNotOptimize(f(matrix));
}

#define declare_test(name)                                      \
  static void name(benchmark::State& state)                     \
  {                                                             \
    run_test(state, &matrix_elements_sum_ ## name);             \
  }                                                             \
                                                                \
  BENCHMARK(name)->RangeMultiplier(2)->Range(2, g_max_size)

//declare_test(branchless);
//declare_test(branchless_vector_of_bool);
declare_test(branchless_return_early);

//declare_test(branches);
//declare_test(branches_vector_of_bool);
declare_test(branches_range);
//declare_test(branches_range_return_early);

declare_test(indices);
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

declare_test(best_ratings);

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

void column_major(benchmark::State& state)
{
  auto matrix(random_matrix(state.range(0)));

  const int n(matrix.size());

  for (int y(1); y != n; ++y)
    for (int x(0); x != y; ++x)
      std::swap(matrix[y][x], matrix[x][y]);
      
  for (auto _ : state)
    benchmark::DoNotOptimize(matrix_elements_sum_column_major(matrix));
}

BENCHMARK(column_major)->RangeMultiplier(2)->Range(2, g_max_size);

int main(int argc, char** argv)
{
  benchmark::Initialize(&argc, argv);

  std::iota(g_indices.begin(), g_indices.end(), 0);
  
  benchmark::RunSpecifiedBenchmarks();
  
  return 0;
}
