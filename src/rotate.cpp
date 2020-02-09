#include <benchmark/benchmark.h>

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <iostream>

std::vector<std::vector<int>> random_matrix()
{
  std::random_device rd;
  static int seed(rd());
  
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> rg;
  
  const int n(1000);

  std::vector<std::vector<int>> matrix(n);

  for (std::vector<int>& row : matrix)
    {
      row.resize(n);

      for (int i(0); i != n; ++i)
        row[i] = (rg(gen) % 2);
    }

  return matrix;
}

std::vector<std::vector<int>> mine(std::vector<std::vector<int>> a)
{
    const int n(a.size());
    
    for (int y(0); y != n - 1; ++y)
        for (int x(y + 1); x != n; ++x)
            std::swap(a[y][x], a[x][y]);
    
    for (int y(0); y != n; ++y)
    {
        auto& row(a[y]);
        
        for (int x(0); x != n / 2; ++x)
            std::swap(row[x], row[n - x - 1]);
    }
    
    return a;
}

std::vector<std::vector<int>> yashar(std::vector<std::vector<int>> a)
{
    int n = a.size();
    for(int i = 0; i < n; ++i)
    {
        for(int j = i; j < n-i-1; ++j)
        {
          std::swap(a[i][j],a[j][n-i-1]);
          std::swap(a[i][j],a[n-i-1][n-j-1]);
          std::swap(a[i][j],a[n-j-1][i]);
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
