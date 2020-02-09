#include <benchmark/benchmark.h>

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <iostream>

std::vector<std::vector<bool>> random_matrix()
{
  std::random_device rd;
  static int seed(rd());
  
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> rg;
  
  const int row_size(10000);
  const int row_count(10000);

  std::vector<std::vector<bool>> matrix(row_count);

  for (std::vector<bool>& row : matrix)
    {
      row.reserve(row_size);

      for (int i(0); i != row_size; ++i)
        row[i] = (rg(gen) % 2);
    }

  return matrix;
}

std::vector<std::vector<int>> mine(const std::vector<std::vector<bool>>& matrix)
{
    const int rows(matrix.size());
    const int columns(matrix[0].size());
    
    std::vector<std::vector<int>> result(rows, std::vector<int>(columns, 0));
    const std::vector<bool>* prev(&matrix[0]);
    const std::vector<bool>* next(&matrix[0]);
    
    for (int y(0); y!=rows; ++y)
    {
        const bool top( y == 0 );
        const bool bottom( y == rows - 1);
        
        const std::vector<bool>* current(next);
        next = (bottom ? current : &matrix[y+1]);
        
        auto& result_row(result[y]);
        
        for (int x(0); x != columns; ++x)
        {
            const int left(std::max(x-1, 0));
            const int right(std::min(x+1, columns-1));

            result_row[x] =
                !top
                * ((x > 0) * (*prev)[left]
                    + (*prev)[x]
                    + (x < columns-1) * (*prev)[right])

                + (x > 0) * (*current)[left]
                + (x < columns-1) * (*current)[right]

                + !bottom
                    * ((x > 0) * (*next)[left]
                        + (*next)[x]
                        + (x < columns-1) * (*next)[right]);
        }
        
        prev = current;
    }
    
    return result;
}

std::vector<std::vector<int>> yashar(const std::vector<std::vector<bool>>& m)
{
  std::vector<std::vector<int>> a(m.size(), std::vector<int>(m[0].size()));
    for(int i = 0; i < a.size(); ++i)
    {
        for(int j = 0; j < a[0].size(); ++j)
        {
            for(int r = -1; r < 2; ++r)
                for(int c = -1; c < 2; ++c)
                {
                    if(!(r|c))continue;
                    if(i+r>=0 and i+r<a.size() and j+c>=0 and j+c<a[0].size() && m[i+r][j+c])
                        ++a[i][j];
                }
        }
    }
    return a;
}

static void mine_test(benchmark::State& state)
{
  const auto matrix(random_matrix());

  for (auto _ : state) {
    benchmark::DoNotOptimize(mine(matrix));
  }
}

BENCHMARK(mine_test);

static void yashar_test(benchmark::State& state)
{
  const auto matrix(random_matrix());

  for (auto _ : state) {
    benchmark::DoNotOptimize(yashar(matrix));
  }
}

BENCHMARK(yashar_test);

BENCHMARK_MAIN();
