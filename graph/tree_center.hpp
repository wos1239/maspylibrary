#include "graph/shortest_path/bfs01.hpp"
#include "graph/shortest_path/restore_path.hpp"

// 做了一点修改
// 返回直径[a,b]和直径的长度d
// 使用Tree.jump(a,b, d/2)来得到中心点

// verify してない
// {a, b, d}：a から b に d 進む / {a, a, 0}
// 直径の長さが偶数であることを仮定している（必要なら 2 倍しておくこと）
template <typename T, typename GT>
tuple<int, int, T> tree_center(GT& G) {
  assert(G.is_prepared());
  auto [distA, parA] = bfs01<T>(G, 0);
  int a = max_element(all(distA)) - distA.begin();
  auto [dist, par] = bfs01<T>(G, A);
  int b = max_element(all(dist)) - dist.begin();
  T d = dist[b];
//   assert(d % 2 == 0);
//   vc<int> path = restore_path(par, b);
//   FOR(i, len(path)) {
//     int v = path[i];
//     if (dist[v] == d / 2) return {v, v, 0};
//   }
//   FOR(i, len(path) - 1) {
//     int a = path[i], b = path[i + 1];
//     if (dist[a] < d / 2 && d / 2 < dist[b]) { return {a, b, d / 2 - dist[a]}; }
//   }
//   assert(0);
//   return {-1, -1, 0};
  return {a, b, d};
}
