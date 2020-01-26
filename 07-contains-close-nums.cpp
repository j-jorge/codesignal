#include <benchmark/benchmark.h>

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <iostream>

std::vector<int> random_ints()
{
  std::random_device rd;
  static int seed(rd());
  
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> rg;
  int n(55000);
  
  std::vector<int> result(n);

  for (int i(0); i != n; ++i)
    result[i] = rg(gen);

  return result;
}

bool mine(std::vector<int> nums, int k)
{
  const int n(nums.size());
  std::vector<int> idx(n);
  const auto idx_begin(idx.begin());
  const auto idx_end(idx.end());
    
  std::iota(idx_begin, idx_end, 0);
    
  std::sort
    (idx_begin, idx_end,
     [&](int lhs, int rhs) -> bool
     {
       const int lhs_num(nums[lhs]);
       const int rhs_num(nums[rhs]);
             
       return
         (lhs_num == rhs_num) && (lhs < rhs)
         || (lhs_num < rhs_num);
     });
    
  for (auto it(idx_begin); it != idx_end; ++it)
    {
      auto next(it + 1);
        
      for (;
           (next != idx_end) && (nums[*next] == nums[*it]);
           ++next, ++it)
        if (*next - *it <= k)
          return true;
    }
    
  return false;
}

bool quinn_ngo(std::vector<int> nums, int k)
{
    std::unordered_map<int, int> numToIndex;
    for (int i = 0; i < nums.size(); ++i) {
        auto it = numToIndex.find(nums.at(i));
        if (it != numToIndex.end() && i - it->second <= k) return true;
        numToIndex[nums.at(i)] = i;
    }
    return false;
}

static void mine_test(benchmark::State& state)
{
  const auto values(random_ints());

  for (auto _ : state) {
    benchmark::DoNotOptimize(mine(values, state.range(0)));
  }
}

static void quinn_ngo_test(benchmark::State& state)
{
  const auto values(random_ints());

  for (auto _ : state) {
    benchmark::DoNotOptimize(quinn_ngo(values, state.range(0)));
  }
}

BENCHMARK(mine_test)->RangeMultiplier(8)->Range(2, 65536);
BENCHMARK(quinn_ngo_test)->RangeMultiplier(8)->Range(2, 65536);;

BENCHMARK_MAIN();
