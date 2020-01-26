#include <benchmark/benchmark.h>

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <iostream>

template<typename T>
struct ListNode
{
   ListNode(const T &v) : value(v), next(nullptr) {}
   T value;
   ListNode *next;
};

ListNode<int>* random_list()
{
  std::random_device rd;
  static int seed(rd());
  
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> rg;
  
  ListNode<int>* last(nullptr);

  for (int i(0); i != 10000; ++i)
    {
      ListNode<int>* const n(new ListNode<int>(rg(gen)));
      n->next = last;
      last = n;
    }

  return last;
}

struct range
{
    ListNode<int>* first;
    ListNode<int>* last;
};

range reverse(ListNode<int>*& node, int& k)
{
    range result;
    result.last = node;
    ListNode<int>* prev(nullptr);
    
    while ((node != nullptr) && (k > 0))
    {
        ListNode<int>* next = node->next;
        node->next = prev;
        prev = node;
        node = next;
        
        --k;
    }
    
    result.first = prev;
    return result;
}

ListNode<int> * mine(ListNode<int> * l, int k)
{
    int part_k(k);
    range first_group(reverse(l, part_k));
    range last_group(first_group);
    ListNode<int>* last_group_parent(nullptr);
    
    while (l != nullptr)
    {
        part_k = k;
        range new_group(reverse(l, part_k));
        last_group_parent = last_group.last;
        last_group_parent->next = new_group.first;
        last_group = new_group;
    }
    
    if (part_k != 0)
    {
        part_k = k;
        range new_group(reverse(last_group.first, part_k));
        
        if (last_group_parent == nullptr)
            return new_group.first;
        
        last_group_parent->next = new_group.first;
    }
    
    return first_group.first;
}

ListNode<int> * yashar(ListNode<int> * r, int k)
{
    int t = 0;
    ListNode<int> *d = r;
    while(t<k && d)
    {
        ++t;
        d = d->next;
    }
    if(t!=k)
        return r;
    ListNode<int> * l = 0;
    ListNode<int> * first = r;
    while(t--)
    {
        d = r;
        r = r->next;
        d->next = l;
        l = d;
    }
    first -> next = yashar(r,k);
    return d;
}

static void mine_test(benchmark::State& state)
{
  const auto list(random_list());

  for (auto _ : state) {
    benchmark::DoNotOptimize(mine(list, state.range(0)));
  }

  delete list;
}

static void yashar_test(benchmark::State& state)
{
  const auto list(random_list());

  for (auto _ : state) {
    benchmark::DoNotOptimize(yashar(list, state.range(0)));
  }

  delete list;
}

BENCHMARK(mine_test)->RangeMultiplier(2)->Range(2, 1024);
BENCHMARK(yashar_test)->RangeMultiplier(2)->Range(2, 1024);;

BENCHMARK_MAIN();
