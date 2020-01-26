#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <iostream>

std::vector<int> perform_query
(const std::vector<int>& a, std::vector<int> b,
 const std::vector<std::vector<int>>& queries)
{
  const auto a_begin(a.begin());
  const auto a_end(a.end());
  std::vector<int> result;
  result.reserve(queries.size());
  
  for (const auto& q : queries)
    {
      if (q.size() == 3)
        {
          b[q[1]] = q[2];
          continue;
        }

      const int sum(q[1]);
      int r(0);

      for (int bj : b)
        r += std::count(a_begin, a_end, sum - bj);

      result.emplace_back(r);
    }
  
  return result;
}

std::vector<int> perform_query_sorted_a
(std::vector<int> a, std::vector<int> b,
 const std::vector<std::vector<int>>& queries)
{
  const auto a_begin(a.begin());
  const auto a_end(a.end());
  std::sort(a_begin, a_end);
  
  std::vector<int> result;
  result.reserve(queries.size());
  
  for (const auto& q : queries)
    {
      if (q.size() == 3)
        {
          b[q[1]] = q[2];
          continue;
        }

      const int sum(q[1]);
      int r(0);

      for (int bj : b)
        {
          const int s = sum - bj;
          const auto it(std::lower_bound(a_begin, a_end, s));
          const auto eit
            (std::find_if(it, a_end, [s](int v) -> bool { return v != s; }));
          
          r += eit - it;
        }

      result.emplace_back(r);
    }
  
  return result;
}

std::vector<int> perform_query_scan_a
(const std::vector<int>& a, std::vector<int> b,
 const std::vector<std::vector<int>>& queries)
{
  const auto b_begin(b.begin());
  const auto b_end(b.end());
  std::vector<int> result;
  result.reserve(queries.size());
  
  for (const auto& q : queries)
    {
      if (q.size() == 3)
        {
          b[q[1]] = q[2];
          continue;
        }

      const int sum(q[1]);
      int r(0);

      for (int ai : a)
        r += std::count(b_begin, b_end, sum - ai);

      result.emplace_back(r);
    }
  
  return result;
}

std::vector<int> perform_query_cached
(const std::vector<int>& a, std::vector<int> b,
 const std::vector<std::vector<int>>& queries)
{
  const int a_size(a.size());
  const int b_size(b.size());

  std::vector<std::vector<int>> a_plus_b(a_size, std::vector<int>());

  for (int i(0); i!=a_size; ++i)
    {
      const int ai(a[i]);
      std::vector<int>& sums(a_plus_b[i]);
      sums.reserve(b_size);
      
      for (int v : b)
        sums.emplace_back(ai + v);
    }
  
  std::vector<int> result;
  result.reserve(queries.size());
  
  for (const auto& q : queries)
    {
      if (q.size() == 3)
        {
          const int j(q[1]);
          const int v(q[2]);
          const int diff(v - b[j]);
          
          for (std::vector<int>& sums : a_plus_b)
            sums[j] += diff;

          continue;
        }

      const int sum(q[1]);
      int r(0);

      for (const std::vector<int>& sums : a_plus_b)
        r += std::count(sums.begin(), sums.end(), sum);

      result.emplace_back(r);
    }
  
  return result;
}

template<typename F>
std::vector<int> run_test
(const char* name, const std::vector<int> a, std::vector<int> b,
 std::vector<std::vector<int>> q, F&& f)
{
  const std::chrono::steady_clock::time_point start
    (std::chrono::steady_clock::now());

  const std::vector<int> result(f(a, b, q));
  
  const std::chrono::steady_clock::time_point end
    (std::chrono::steady_clock::now());

  std::cout
    << name << ": "
    << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
       .count()
    << " ms.\n";

  return result;
}

void test()
{
  std::mt19937 gen(0);
  std::uniform_int_distribution<int> rg;
  
  const int a_size(rg(gen) % 10000);
  const int b_size(rg(gen) % 100000);
  const int q_size(rg(gen) % 1000);
  const int max_value(1000000000);
  
  std::vector<int> a(a_size);
  std::vector<int> b(b_size);

  for (int& va : a)
    va = rg(gen) % max_value;

  for (int& vb : b)
    vb = rg(gen) % max_value;

  std::vector<std::vector<int>> q(q_size);

  for (std::vector<int>& query : q)
    if(rg(gen) % 2 == 0)
      query = std::vector<int>{{0, rg(gen) % b_size, rg(gen) % max_value}};
    else
      query = std::vector<int>{{1, rg(gen) % max_value}};

  const std::vector<int> perform_query_result
    (run_test("perform_query", a, b, q, &perform_query));
  const std::vector<int> perform_query_sorted_a_result
    (run_test("perform_query_sorted_a", a, b, q, &perform_query_sorted_a));
  const std::vector<int> perform_query_scan_a_result
    (run_test("perform_query_scan_a", a, b, q, &perform_query_scan_a));
  const std::vector<int> perform_query_cached_result
    (run_test("perform_query_cached", a, b, q, &perform_query_cached));

  std::cout << "cached ok: "
            << (perform_query_result == perform_query_cached_result)
            << '\n';
  std::cout << "sorted_a ok: "
            << (perform_query_result == perform_query_sorted_a_result)
            << '\n';
  std::cout << "scan_a ok: "
            << (perform_query_result == perform_query_scan_a_result)
            << '\n';
}

int main()
{
  test();
  return 0;
}
