#include <benchmark/benchmark.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>

std::string naive
(std::string str, std::vector<std::vector<int>> pairs)
{
  std::unordered_set<std::string> candidates;
  candidates.insert(str);
  std::string best(std::move(str));
    
  int old_size;
  int new_size(1);
    
  do
    {
      std::unordered_set<std::string> base(candidates);
      old_size = new_size;
        
      for (std::string s : base)
        for (const std::vector<int>& p : pairs)
          {
            std::string ref(s);
            std::swap(ref[p[0]-1], ref[p[1]-1]);
                
            if (ref > best)
              best = ref;
                
            candidates.insert(ref);
          }
        
      new_size = candidates.size();
    }
  while (new_size != old_size);
    
  return best;
}

std::string naive_vector_in_loop
(std::string str, std::vector<std::vector<int>> pairs)
{
  std::unordered_set<std::string> candidates;
  candidates.insert(str);
  std::string best(std::move(str));
    
  int old_size;
  int new_size(1);
    
  do
    {
      std::vector<std::string> base(candidates.begin(), candidates.end());
      old_size = new_size;
        
      for (std::string s : base)
        for (const std::vector<int>& p : pairs)
          {
            std::string ref(s);
            std::swap(ref[p[0]-1], ref[p[1]-1]);
                
            if ((candidates.insert(ref).second) && (ref > best))
              best = std::move(ref);
          }
        
      new_size = candidates.size();
    }
  while (new_size != old_size);
    
  return best;
}

std::string reduce
(std::string str, std::vector<std::vector<int>> pairs)
{
  std::unordered_set<std::string> all;
  all.insert(str);
  
  std::vector<std::string> candidates;
  candidates.emplace_back(str);
  
  std::string best(std::move(str));
    
  while (!candidates.empty())
    {
      std::vector<std::string> base;
      base.swap(candidates);
        
      for (const std::string& s : base)
        for (const std::vector<int>& p : pairs)
          {
            std::string ref(s);
            std::swap(ref[p[0]-1], ref[p[1]-1]);
                
            if (all.insert(ref).second)
              {
                if (ref > best)
                  best = ref;

                candidates.emplace_back(ref);
              }
          }
    }
    
  return best;
}

struct node
{
  std::string from_string;
  std::vector<std::vector<int>> pairs;
};

std::string scan_edges_iter
(std::string str, const std::vector<std::vector<int>>& pairs)
{
  std::string best(str);
  std::vector<node> pending;
  pending.emplace_back(node{str, pairs});

  while (!pending.empty())
    {
      node next(std::move(pending.back()));
      pending.pop_back();

      const auto begin(next.pairs.begin());
      const auto end(next.pairs.end());
      
      for (auto it(begin); it != end; ++it)
        {
          const int v((*it)[0]-1);
          const int n((*it)[1]-1);

          node new_node;
          new_node.from_string = next.from_string;
          new_node.pairs.insert(new_node.pairs.end(), begin, it);
          new_node.pairs.insert(new_node.pairs.end(), it + 1, end);
          
          pending.emplace_back(new_node);

          std::swap(new_node.from_string[v], new_node.from_string[n]);

          if (new_node.from_string > best)
            best = new_node.from_string;

          pending.emplace_back(new_node);
        }
    }

  return best;
}

void scan_edges
(std::string& best, std::string& str,
 std::unordered_set<int>& seen,
 const std::unordered_map<int, std::unordered_set<int>>& edges)
{
  const auto edges_end(edges.end());

  for (const auto& e : edges)
    {
      const int v(e.first);
      
      if (!seen.insert(v).second)
        continue;
      
      for (int n : e.second)
        {
          scan_edges(best, str, seen, edges);

          std::swap(str[v], str[n]);

          if (str > best)
            best = str;

          scan_edges(best, str, seen, edges);

          if (str > best)
            best = str;
          
          std::swap(str[v], str[n]);
          seen.erase(n);
        }

      seen.erase(v);
    }
}

std::string graph_scan
(std::string str, const std::vector<std::vector<int>>& pairs)
{
  return scan_edges_iter(str, pairs);
  
  std::unordered_map<int, std::unordered_set<int>> edges;

  for (const std::vector<int>& pair : pairs)
    {
      const int v1(pair[0] - 1);
      const int v2(pair[1] - 1);
      
      edges[v1].insert(v2);
    }

  std::unordered_set<int> seen;
  std::string best(str);
  
  scan_edges(best, str, seen, edges);

  return best;
}

/*
str="aze";
pairs=
  {
   {1,3},
   {2,3}
  }

// can't bring the best for each i:
// best for each i = zeaz

// if we keep a set of candidates:
aezz/1 -> zeaz, zeza
…/2 -> zaez, zzea

str="aezz";
pairs=
  {
   {1,3},
   {1,4},
   {2,3}
  }
*/
#define TEST_VALUES 1
template<typename F>
static void run_test(benchmark::State& state, F&& f)
{
#if TEST_VALUES == 0
  const std::string str("lvvyfrbhgiyexoirhunnuejzhesylojwbyatfkrv");
  const std::vector<std::vector<int>> pairs =
    {
     {13,23}, 
     {13,28}, 
     {15,20}, 
     {24,29}, 
     {6,7}, 
     {3,4}, 
     {21,30}, 
     {2,13}, 
     {12,15}, 
     {19,23}, 
     {10,19}, 
     {13,14}, 
     {6,16}, 
     {17,25}, 
     {6,21}, 
     {17,26}, 
     {5,6}, 
     {12,24}
    };
#elif TEST_VALUES == 1
  const std::string str("fixmfbhyutghwbyezkveyameoamqoi");
  const std::vector<std::vector<int>> pairs =
    {
     {8,5}, 
     {10,8}, 
     {4,18}, 
     {20,12}, 
     {5,2}, 
     {17,2}, 
     {13,25}, 
     {29,12}, 
     {22,2}, 
     {17,11}
    };
#endif
  
  for (auto _ : state) {
    benchmark::DoNotOptimize(f(str, pairs));
  }
}

#if 0
BENCHMARK_CAPTURE(run_test, naive, &naive);
BENCHMARK_CAPTURE(run_test, naive, &naive_vector_in_loop);
BENCHMARK_CAPTURE(run_test, naive, &reduce);
BENCHMARK_CAPTURE(run_test, naive, &graph_scan);

BENCHMARK_MAIN();
#else
void test
(const std::string& str, const std::vector<std::vector<int>>& pairs,
 const std::string& expected)
{
    const std::string result(graph_scan(str, pairs));
    
    std::cout << (result == expected) << " " << str << " -> " << expected
              << "\n";
}

int main()
{
  {
    const std::string str("abdc");
    const std::vector<std::vector<int>> pairs =
      {
       {1,4},
       {3,4}
      };

    test(str, pairs, "dbca");
  }
  
  {
    const std::string str("abcdefgh");
    const std::vector<std::vector<int>> pairs =
      {
       {1,4},
       {7,8}
      };

    test(str, pairs, "dbcaefhg");
  }

  {
    const std::string str("aezz");
    const std::vector<std::vector<int>> pairs =
      {
       {1, 3},
       {1, 4},
       {2, 3}
      };

    test(str, pairs, "zzea");
  }

  {
    const std::string str("fixmfbhyutghwbyezkveyameoamqoi");
    const std::vector<std::vector<int>> pairs =
      {
       {8,5}, 
       {10,8}, 
       {4,18}, 
       {20,12}, 
       {5,2}, 
       {17,2}, 
       {13,25}, 
       {29,12}, 
       {22,2}, 
       {17,11}
      };

    test(str, pairs, "fzxmybhtuigowbyefkvhyameoamqei");
  }

  {
    const std::string str("lvvyfrbhgiyexoirhunnuejzhesylojwbyatfkrv");
    const std::vector<std::vector<int>> pairs =
      {
       {13,23}, 
       {13,28}, 
       {15,20}, 
       {24,29}, 
       {6,7}, 
       {3,4}, 
       {21,30}, 
       {2,13}, 
       {12,15}, 
       {19,23}, 
       {10,19}, 
       {13,14}, 
       {6,16}, 
       {17,25}, 
       {6,21}, 
       {17,26}, 
       {5,6}, 
       {12,24}
      };

    test(str, pairs, "lyyvurrhgxyzvonohunlfejihesiebjwbyatfkrv");
  }
  
  return 0;
}
#endif
