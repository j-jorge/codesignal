#include <benchmark/benchmark.h>

#include <algorithm>
#include <bitset>
#include <chrono>
#include <map>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

bool g_dbg(false);

const int dishes_count(5);
const int total_ingredient_count(3);

std::vector<std::vector<std::string>> random_dishes()
{
  std::random_device rd;
  static int seed(rd());
  
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> rg;
  
  const int n(dishes_count);
  const int ingredient_count_min(1);
  
  std::vector<std::vector<std::string>> result(n);
  int i(0);
  
  for (std::vector<std::string>& dish : result)
    {
      dish.emplace_back("dish_" + std::to_string(i));
      ++i;

      const int m
        (ingredient_count_min
         + rg(gen) % (total_ingredient_count - ingredient_count_min));

      std::size_t available( total_ingredient_count );
      std::size_t needed( m );
      std::size_t j( 0 );
    
      for ( int v( 0 ); needed != 0; ++v, --available )
        if ( rg(gen) % available < needed )
        {
          dish.emplace_back("ingredient_" + std::to_string(v));
          --needed;
          ++j;
        }

      if (std::unordered_set<std::string>(dish.begin(), dish.end()).size()
          != dish.size())
        std::cerr << "duplicates.\n";
    }

  return result;
}

std::vector<std::vector<std::string>> mine
(std::vector<std::vector<std::string>> dishes)
{
    std::vector<std::vector<std::string>> result;
    
    for (std::vector<std::string>& dish : dishes)
    {
        std::string dish_name(std::move(dish[0]));
        
        const int n(dish.size());
        
        for (int i(1); i != n; ++i)
        {
            std::string ingredient(std::move(dish[i]));
            
            const auto find_ingredient
                ([&](const std::vector<std::string>& r) -> bool
                 {
                     return r[0] == ingredient;
                 });
            const auto end(result.end());
            auto it(std::find_if(result.begin(), end, find_ingredient));
            
            if (it == end)
            {
              std::vector<std::string>& r(result.emplace_back(2));
              r[0] = std::move(ingredient);
              r[1] = dish_name;
            }
            else
                it->emplace_back(dish_name);
        }
    }
    
    // remove ingredients with a single dish
    auto begin(result.begin());
    const auto result_end(result.end());
    
    for (auto end(result_end);
         begin != end; )
        if (begin->size() == 2)
        {
            --end;
            end->swap(*begin);
        }
        else
            ++begin;
    
    result.erase(begin, result_end);
    
    for (std::vector<std::string>& r : result)
        std::sort(r.begin() + 1, r.end());
    
    std::sort
        (result.begin(), result.end(),
        [](const std::vector<std::string>& lhs,
           const std::vector<std::string>& rhs) -> bool
         {
             return lhs[0] < rhs[0];
         });
    
    return result;
}

std::vector<std::vector<std::string>> mine_reserve
(std::vector<std::vector<std::string>> dishes)
{
    std::vector<std::vector<std::string>> result;
    result.reserve(total_ingredient_count);
    
    for (std::vector<std::string>& dish : dishes)
    {
        std::string dish_name(std::move(dish[0]));
        
        const int n(dish.size());
        
        for (int i(1); i != n; ++i)
        {
            std::string ingredient(std::move(dish[i]));
            
            const auto find_ingredient
                ([&](const std::vector<std::string>& r) -> bool
                 {
                     return r[0] == ingredient;
                 });
            const auto end(result.end());
            auto it(std::find_if(result.begin(), end, find_ingredient));
            
            if (it == end)
            {
              std::vector<std::string>& r(result.emplace_back());
              r.reserve(dishes_count);
              r.emplace_back(std::move(ingredient));
              r.emplace_back(dish_name);
            }
            else
                it->emplace_back(dish_name);
        }
    }
    
    // remove ingredients with a single dish
    auto begin(result.begin());
    const auto result_end(result.end());
    
    for (auto end(result_end);
         begin != end; )
        if (begin->size() == 2)
        {
            --end;
            end->swap(*begin);
        }
        else
            ++begin;
    
    result.erase(begin, result_end);
    
    for (std::vector<std::string>& r : result)
        std::sort(r.begin() + 1, r.end());
    
    std::sort
        (result.begin(), result.end(),
        [](const std::vector<std::string>& lhs,
           const std::vector<std::string>& rhs) -> bool
         {
             return lhs[0] < rhs[0];
         });
    
    return result;
}

std::vector<std::vector<std::string>> mine_matrix
(std::vector<std::vector<std::string>> dishes)
{
  std::vector<std::vector<bool>> tags
    (total_ingredient_count, std::vector<bool>(dishes_count));
  std::unordered_map<std::string, int> ingredient_index(1);
  std::vector<std::vector<std::string>> temp_result;
  
  const int dishes_size(dishes.size());
  
  for (int i(0); i != dishes_size; ++i)
    {
      std::vector<std::string>& dish(dishes[i]);
      const int n(dish.size());
      
      for (int j(1); j != n; ++j)
        {
          std::string ingredient(std::move(dish[j]));
          auto it(ingredient_index.find(ingredient));
          int k;
          
          if (it == ingredient_index.end())
            {
              k = ingredient_index.size();
              ingredient_index[ingredient] = k;
              temp_result.emplace_back().emplace_back(std::move(ingredient));
            }
          else
            k = it->second;

          tags[k][i] = true;
        }
    }

  std::vector<std::vector<std::string>> result;
  result.reserve(temp_result.size());
  
  for (int j(0); j != total_ingredient_count; ++j)
    {
      std::vector<std::string> dish(std::move(temp_result[j]));
      const std::vector<bool>& tag(tags[j]);
      const auto tag_end(tag.end());
      int d(0);
      
      for (auto it(tag.begin()); it != tag_end; ++it, ++d)
        if (*it)
          dish.emplace_back(dishes[d][0]);

      if (dish.size() > 2)
        result.emplace_back(std::move(dish));
    }

  for (std::vector<std::string>& r : result)
    std::sort(r.begin() + 1, r.end());
    
  std::sort
    (result.begin(), result.end(),
     [](const std::vector<std::string>& lhs,
        const std::vector<std::string>& rhs) -> bool
     {
       return lhs[0] < rhs[0];
     });
    
  return result;
}

std::vector<std::vector<std::string>> mine_matrix_bitset
(std::vector<std::vector<std::string>> dishes)
{
  std::vector<std::bitset<dishes_count>> tags
    (total_ingredient_count);
  std::unordered_map<std::string, int> ingredient_index;
  std::vector<std::vector<std::string>> temp_result;
  
  const int dishes_size(dishes.size());
  
  for (int i(0); i != dishes_size; ++i)
    {
      std::vector<std::string>& dish(dishes[i]);
      const int n(dish.size());
      
      for (int j(1); j != n; ++j)
        {
          std::string ingredient(std::move(dish[j]));
          auto it(ingredient_index.find(ingredient));
          int k;
          
          if (it == ingredient_index.end())
            {
              k = ingredient_index.size();
              ingredient_index[ingredient] = k;
              temp_result.emplace_back().emplace_back(std::move(ingredient));
            }
          else
            k = it->second;

          tags[k].set(i);
        }
    }

  std::vector<std::vector<std::string>> result;
  result.reserve(result.size());
  
  for (int j(0); j != total_ingredient_count; ++j)
    {
      std::vector<std::string> dish(std::move(temp_result[j]));
      const std::bitset<dishes_count>& tag(tags[j]);

      for (int d(0); d != dishes_count; ++d)
        if(tag[d])
          dish.emplace_back(dishes[d][0]);

      if (dish.size() > 2)
        result.emplace_back(std::move(dish));
    }

  for (std::vector<std::string>& r : result)
    std::sort(r.begin() + 1, r.end());
    
  std::sort
    (result.begin(), result.end(),
     [](const std::vector<std::string>& lhs,
        const std::vector<std::string>& rhs) -> bool
     {
       return lhs[0] < rhs[0];
     });
    
  return result;
}

std::vector<std::vector<std::string>> mine_matrix_no_temp_string
(std::vector<std::vector<std::string>> dishes)
{
  std::vector<std::vector<bool>> tags
    (total_ingredient_count, std::vector<bool>(dishes_count));

  const auto str_pointer_hash
    ([](const std::string* s) -> std::size_t
     {
       return std::hash<std::string>()(*s);
     });
  const auto str_pointer_equal
    ([](const std::string* lhs, const std::string* rhs) -> bool
     {
       return *lhs == *rhs;
     });
  
  std::unordered_map
    <
      const std::string*, int,
      decltype(str_pointer_hash),
      decltype(str_pointer_equal)
    > ingredient_index(1, str_pointer_hash, str_pointer_equal);
  std::vector<std::vector<std::string*>> temp_result;
  
  const int dishes_size(dishes.size());
  
  for (int i(0); i != dishes_size; ++i)
    {
      std::vector<std::string>& dish(dishes[i]);
      const int n(dish.size());
      
      for (int j(1); j != n; ++j)
        {
          std::string* ingredient(&dish[j]);
          auto it(ingredient_index.find(ingredient));
          int k;
          
          if (it == ingredient_index.end())
            {
              k = ingredient_index.size();
              ingredient_index[ingredient] = k;
              temp_result.emplace_back(std::vector<std::string*>({ingredient}));
            }
          else
            k = it->second;

          tags[k][i] = true;
        }
    }

  std::vector<std::vector<std::string>> result;
  result.reserve(temp_result.size());

  for (int j(0); j != total_ingredient_count; ++j)
    {
      std::vector<std::string*> temp_dish(std::move(temp_result[j]));

      const std::vector<bool>& tag(tags[j]);
      const auto tag_end(tag.end());
      int d(0);
      
      for (auto it(tag.begin()); it != tag_end; ++it, ++d)
        if (*it)
          temp_dish.emplace_back(&dishes[d][0]);

      const int temp_dish_size(temp_dish.size());
      
      if (temp_dish_size > 2)
        {
          std::vector<std::string>& dish(result.emplace_back(temp_dish_size));

          for (int i(0); i != temp_dish_size; ++i)
            {
              dish[i] = std::move(*temp_dish[i]);
              if (g_dbg)
                printf
                  ("ingredient %d, dish %d/%d, '%s'\n", j, i,
                   temp_dish_size, dish[i].c_str());
            }
        }
    }

  for (std::vector<std::string>& r : result)
    std::sort(r.begin() + 1, r.end());
    
  std::sort
    (result.begin(), result.end(),
     [](const std::vector<std::string>& lhs,
        const std::vector<std::string>& rhs) -> bool
     {
       return lhs[0] < rhs[0];
     });
    
  return result;
}

std::vector<std::vector<std::string>>
david_f25(std::vector<std::vector<std::string>> dishes)
{
  using namespace std;
  
  map<string,vector<string>> m;
  for (auto& d : dishes) {
    for (int i = 1; i < d.size(); ++i) {
      m[d[i]].push_back(d[0]);
    }
  }
  vector<vector<string>> ans;
  for (auto& p : m) {
    if (p.second.size() > 1) {
      sort(p.second.begin(),p.second.end());
      p.second.insert(p.second.begin(),p.first);
      ans.push_back(p.second);
    }
  }
  sort(ans.begin(),ans.end());
  return ans;
}

std::vector<std::vector<std::string>>
david_f25_no_sort(std::vector<std::vector<std::string>> dishes)
{
  using namespace std;
  
  map<string,vector<string>> m;
  for (auto& d : dishes) {
    for (int i = 1; i < d.size(); ++i) {
      m[d[i]].push_back(d[0]);
    }
  }
  vector<vector<string>> ans;
  for (auto& p : m) {
    if (p.second.size() > 1) {
      sort(p.second.begin(),p.second.end());
      p.second.insert(p.second.begin(),p.first);
      ans.push_back(p.second);
    }
  }

  return ans;
}

std::vector<std::vector<std::string>>
david_f25_explicit(std::vector<std::vector<std::string>> dishes)
{
  std::map<std::string, std::vector<std::string>> m;
  
  for (auto& d : dishes)
    {
      const int count(d.size());
      const std::string& dish(d[0]);
      
      for (int i(1); i < count; ++i)
        m[d[i]].emplace_back(dish);
    }

  std::vector<std::vector<std::string>> ans;
  ans.reserve(m.size());
  
  for (auto& p : m)
    {
      std::vector<std::string> r(std::move(p.second));
      
      if (r.size() > 1)
        {
          const auto r_begin(r.begin());
          std::sort(r_begin, r.end());
          r.insert(r_begin, p.first);
          ans.emplace_back(std::move(r));
        }
    }

  return ans;
}

std::vector<std::vector<std::string>>
map_of_sets(std::vector<std::vector<std::string>> dishes)
{
  std::map<std::string, std::set<std::string>> m;
  
  for (auto& d : dishes)
    {
      const int count(d.size());
      const std::string& dish(d[0]);
      
      for (int i(1); i < count; ++i)
        m[d[i]].insert(dish);
    }

  std::vector<std::vector<std::string>> ans;

  for (auto& p : m)
    {
      const int count(p.second.size());
      
      if (count > 1)
        {
          std::vector<std::string>& r(ans.emplace_back());
          r.reserve(count + 1);
          r.emplace_back(p.first);
          r.insert(r.end(), p.second.begin(), p.second.end());
        }
    }

  return ans;
}

#define declare_benchmark(name)                                 \
                                                                \
  static std::vector<std::vector<std::string>> name ## _result; \
                                                                \
  static void name ## _test(benchmark::State& state)            \
  {                                                             \
    const auto dishes(random_dishes());                         \
                                                                \
    for (auto _ : state)                                        \
      benchmark::DoNotOptimize(name(dishes));                   \
                                                                \
    name ## _result = name(dishes);                             \
  }                                                             \
                                                                \
  BENCHMARK(name ## _test);

declare_benchmark(mine)
declare_benchmark(mine_reserve)
declare_benchmark(mine_matrix)
declare_benchmark(david_f25)
declare_benchmark(david_f25_no_sort)
declare_benchmark(map_of_sets)
declare_benchmark(david_f25_explicit)
declare_benchmark(mine_matrix_bitset)
declare_benchmark(mine_matrix_no_temp_string)

bool valid
(const std::vector<std::vector<std::string>>& lhs,
 const std::vector<std::vector<std::string>>& rhs)
{
  const int lhs_size(lhs.size());
  const int rhs_size(rhs.size());
  
  if (lhs_size != rhs_size)
    {
      fprintf
        (stderr,
         "Incorrect ingredient count. Expected %d, got %d.\n",
         lhs_size, rhs_size);
      return false;
    }
  
  for (int i(0); i != lhs_size; ++i)
    {
      const std::vector<std::string>& lhs_row(lhs[i]);
      const std::vector<std::string>& rhs_row(rhs[i]);

      const int lhs_row_size(lhs_row.size());
      const int rhs_row_size(rhs_row.size());

      if (lhs_row_size != rhs_row_size)
        {
          fprintf
            (stderr,
             "%d: Incorrect dish count. Expected %d, got %d.\n",
             i, lhs_row_size, rhs_row_size);

          fprintf(stderr, "Expected:");
          for (const std::string& s : lhs_row)
            fprintf(stderr, " %s", s.c_str());
          fprintf(stderr, "\n");

          fprintf(stderr, "Got:");
          for (const std::string& s : rhs_row)
            fprintf(stderr, " %s", s.c_str());
          fprintf(stderr, "\n");

          return false;
        }

      for (int j(0); j != lhs_row_size; ++j)
        {
          const std::string& lhs_dish(lhs_row[j]);
          const std::string& rhs_dish(rhs_row[j]);
          
          if (lhs_dish != rhs_dish)
            {
              fprintf
                (stderr, "%d, %d: Incorrect dish name. Expected %s, got %s.\n",
                 i, j, lhs_dish.c_str(), rhs_dish.c_str());
              return false;
            }
        }
    }

  return true;
}

int main(int argc, char** argv)
{
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();

  g_dbg = true;
  mine_matrix_no_temp_string(random_dishes());
  
  if (!valid(mine_result, mine_reserve_result))
    printf("mine_reserve: BAD.\n");
  
  if (!valid(mine_result, mine_matrix_result))
    printf("mine_matrix: BAD.\n");
  
  if (!valid(mine_result, david_f25_result))
    printf("david_f25: BAD.\n");

  if (!valid(mine_result, david_f25_no_sort_result))
    printf("david_f25_no_sort: BAD.\n");

  if (!valid(mine_result, map_of_sets_result))
    printf("map_of_sets: BAD.\n");

  if (!valid(mine_result, david_f25_explicit_result))
    printf("david_f25_explicit: BAD.\n");
  
  if (!valid(mine_result, mine_matrix_bitset_result))
    printf("mine_matrix_bitset: BAD.\n");
  
  if (!valid(mine_result, mine_matrix_no_temp_string_result))
    printf("mine_matrix_no_temp_string: BAD.\n");

  return 0;
}
