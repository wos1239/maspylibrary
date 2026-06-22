#pragma once
#include "ds/node_pool.hpp"

// 通用の伸展树 (Splay Tree) 模板
// Node 类型需由外部定义，并提供以下接口：
//   - Node* l, r, p;
//   - u32 size;
//   - value_type x;
//   - X prod;                 // 区间汇总信息
//   - static void new_node(Node* n, const X& x);
//   - void update();
//   - void push();
//   - X get();
//   - void set(const X&);
//   - void multiply(const X&);
//   - void apply(const A&);
//   - void reverse();
//   - Monoid_X 结构体用于 prod 的单位元和合并操作
// 该类提供通用的 splay 树操作：节点管理、merge/split、区间访问与区间操作。

// Node 型を別に定義して使う

template <typename Node>
struct SplayTree {
  Node_Pool<Node> pool;
  using np = Node *;
  using X = typename Node::value_type;
  using A = typename Node::operator_type;

  // 释放以 c 为根的整棵子树的所有节点(包括c)
  void free_subtree(np c) {
    if (!c) return;
    auto dfs = [&](auto &dfs, np c) -> void {
      if (c->l) dfs(dfs, c->l);
      if (c->r) dfs(dfs, c->r);
      c->p = c->l = c->r = nullptr;
      pool.destroy(c);
    };
    dfs(dfs, c);
  }

  // 复位节点池，丢弃所有节点并重置内部状态
  void reset() { pool.reset(); }

  // 返回一个空树的根指针
  np new_root() { return nullptr; }

  // 创造一个值为 x 的新节点
  np new_node(const X &x) {
    np n = pool.create();
    Node::new_node(n, x);
    return n;
  }

    // 从数组 dat 构造平衡树
    // 适用于一次性批量建树，不会执行 splay 操作
  np new_node(const vc<X> &dat) {
    auto dfs = [&](auto &dfs, int l, int r) -> np {
      if (l == r) return nullptr;
      if (r == l + 1) return new_node(dat[l]);
      int m = (l + r) / 2;
      np l_root = dfs(dfs, l, m);
      np r_root = dfs(dfs, m + 1, r);
      np root = new_node(dat[m]);
      root->l = l_root, root->r = r_root;
      if (l_root) l_root->p = root;
      if (r_root) r_root->p = root;
      root->update();
      return root;
    };
    return dfs(dfs, 0, len(dat));
  }

  // 获取树的大小
  u32 get_size(np root) { return (root ? root->size : 0); }

  // 合并两棵 splay 树，假设所有 l_root 的值都小于 r_root 的值
  np merge(np l_root, np r_root) {
    if (!l_root) return r_root;
    if (!r_root) return l_root;
    assert((!l_root->p) && (!r_root->p));
    // splay_kth(r_root, 0);  // splay したので push 済
    splay_kth(r_root, 0);  // 将 r_root 的最小节点旋转到根
    r_root->l = l_root;
    l_root->p = r_root;
    r_root->update();
    return r_root;
  }
  np merge3(np a, np b, np c) { return merge(merge(a, b), c); }
  np merge4(np a, np b, np c, np d) { return merge(merge(merge(a, b), c), d); }

  // 将树按位置分割为 [0,k) 和 [k,n)
  // k是第k小的元素，不是数值
  pair<np, np> split(np root, u32 k) {
    assert(!root || !root->p);
    if (k == 0) return {nullptr, root};
    if (k == (root->size)) return {root, nullptr};
    splay_kth(root, k - 1);
    np right = root->r;
    root->r = nullptr, right->p = nullptr;
    root->update();
    return {root, right};
  }
  tuple<np, np, np> split3(np root, u32 l, u32 r) {
    np nm, nr;
    tie(root, nr) = split(root, r);
    tie(root, nm) = split(root, l);
    return {root, nm, nr};
  }
  tuple<np, np, np, np> split4(np root, u32 i, u32 j, u32 k) {
    // 分割成 [0,i), [i,j), [j,k), [k,n)
    np d;
    tie(root, d) = split(root, k);
    auto [a, b, c] = split3(root, i, j);
    return {a, b, c, d};
  }

  // 以根节点为中间节点切成三部分：左、根、右
  tuple<np, np, np> split_L_root_R(np root) {
    u32 s = (root->l ? root->l->size : 0);
    return split3(root, s, s + 1);
  }
  // 部分木が区間 [l,r) に対応するようなノードを作って返す
  // そのノードが root になるわけではないので、
  // このノードを参照した後にすぐに splay して根に持ち上げること
  // 定位区间 [l,r) 对应的子树，返回该区间的根节点指针
  // 此函数并不将该区间的根设为整个树的根，调用后应当立即使用 splay 使其真正升到根
  void goto_between(np &root, u32 l, u32 r) {
    if (l == 0 && r == root->size) return;
    if (l == 0) {
      splay_kth(root, r);
      root = root->l;
      return;
    }
    if (r == root->size) {
      splay_kth(root, l - 1);
      root = root->r;
      return;
    }
    splay_kth(root, r);
    np rp = root;
    root = rp->l;
    root->p = nullptr;
    splay_kth(root, l - 1);
    root->p = rp;
    rp->l = root;
    rp->update();
    root = root->r;
  }

  // 中序遍历整棵树并收集所有值
  vc<X> get_all(const np &root) {
    vc<X> res;
    auto dfs = [&](auto &dfs, np root) -> void {
      if (!root) return;
      root->push();
      dfs(dfs, root->l);
      res.eb(root->get());
      dfs(dfs, root->r);
    };
    dfs(dfs, root);
    return res;
  }

  // 获取第 k 个元素的值
  X get(np &root, u32 k) {
    assert(root == nullptr || !root->p);
    splay_kth(root, k);
    return root->get();
  }

  // 更新第 k 个元素的值
  void set(np &root, u32 k, const X &x) {
    assert(root != nullptr && !root->p);
    splay_kth(root, k);
    root->set(x);
  }

  // 对第 k 个节点执行 multiply 操作
  void multiply(np &root, u32 k, const X &x) {
    assert(root != nullptr && !root->p);
    splay_kth(root, k);
    root->multiply(x);
  }

  // 计算区间 [l,r) 的聚合值
  X prod(np &root, u32 l, u32 r) {
    assert(root == nullptr || !root->p);
    using Mono = typename Node::Monoid_X;
    if (l == r) return Mono::unit();
    assert(0 <= l && l < r && r <= root->size);
    goto_between(root, l, r);
    X res = root->prod;
    splay(root, true);
    return res;
  }

  // 获取整棵树的聚合值
  X prod(np &root) {
    assert(root == nullptr || !root->p);
    using Mono = typename Node::Monoid_X;
    return (root ? root->prod : Mono::unit());
  }

  // 对区间 [l,r) 应用操作 a
  void apply(np &root, u32 l, u32 r, const A &a) {
    if (l == r) return;
    assert(0 <= l && l < r && r <= root->size);
    goto_between(root, l, r);
    root->apply(a);
    splay(root, true);
  }
  // 对子树应用操作 a
  void apply(np &root, const A &a) {
    if (!root) return;
    root->apply(a);
  }

  // 反转区间 [l,r)
  void reverse(np &root, u32 l, u32 r) {
    assert(root == nullptr || !root->p);
    if (l == r) return;
    assert(0 <= l && l < r && r <= root->size);
    goto_between(root, l, r);
    root->reverse();
    splay(root, true);
  }
  // 对整棵树标记翻转
  void reverse(np root) {
    if (!root) return;
    root->reverse();
  }

  // 将节点 n 向上旋转一步，改变父子结构
  // 仅进行结构调整，不负责 push/update
  void rotate(Node *n) {
    // n を根に近づける。push, update は rotate の外で行う。
    Node *pp, *p, *c;
    p = n->p;
    pp = p->p;
    if (p->l == n) {
      c = n->r;
      n->r = p;
      p->l = c;
    } else {
      c = n->l;
      n->l = p;
      p->r = c;
    }
    if (pp && pp->l == p) pp->l = n;
    if (pp && pp->r == p) pp->r = n;
    n->p = pp;
    p->p = n;
    if (c) c->p = p;
  }

  // 从根到节点 c 递归下推延迟标记
  void push_from_root(np c) {
    if (!c->p) {
      c->push();
      return;
    }
    push_from_root(c->p);
    c->push();
  }

  // 伸展操作：将节点 me 提升到树根
  // push_from_root_done 为 false 时先从根到 me 下推所有延迟标记
  void splay(Node *me, bool push_from_root_done) {
    // これを呼ぶ時点で、me の祖先（me を除く）は既に push 済であることを仮定
    // 特に、splay 終了時点で me は upd / push 済である
    if (!push_from_root_done) push_from_root(me);
    me->push();
    while (me->p) {
      np p = me->p;
      np pp = p->p;
      if (!pp) {
        rotate(me);
        p->update();
        break;
      }
      bool same = (p->l == me && pp->l == p) || (p->r == me && pp->r == p);
      if (same) rotate(p), rotate(me);
      if (!same) rotate(me), rotate(me);
      pp->update(), p->update();
    }
    // me の update は最後だけでよい
    me->update();
  }

  // 定位第 k 个节点并将其伸展到根
  void splay_kth(np &root, u32 k) {
    assert(0 <= k && k < (root->size));
    while (1) {
      root->push();
      u32 s1 = (root->l ? root->l->size : 0);
      u32 s2 = (root->size) - (root->r ? root->r->size : 0);
      if (k < s1) root = root->l;
      elif (k < s2) { break; }
      else {
        k -= s2;
        root = root->r;
      }
    }
    splay(root, true);
  }

  // check(x), 左側のノード全体が check を満たすように切る
  // 按 check 条件查找最右侧满足条件的前缀位置，并将其左边部分切分出来
  // check(x) 仅对单节点值做判断，返回的 pair 中左部满足条件，右部为其余部分
  template <typename F>
  pair<np, np> split_max_right(np root, F check) {
    if (!root) return {nullptr, nullptr};
    assert(!root->p);
    np c = find_max_right(root, check);
    if (!c) {
      splay(root, true);
      return {nullptr, root};
    }
    splay(c, true);
    np right = c->r;
    if (!right) return {c, nullptr};
    right->p = nullptr;
    c->r = nullptr;
    c->update();
    return {c, right};
  }

  // check(x, cnt), 左側のノード全体が check を満たすように切る
  // 按 check(x, cnt) 查找最右侧满足条件的位置，cnt 为当前节点的序号
  template <typename F>
  pair<np, np> split_max_right_cnt(np root, F check) {
    if (!root) return {nullptr, nullptr};
    assert(!root->p);
    np c = find_max_right_cnt(root, check);
    if (!c) {
      splay(root, true);
      return {nullptr, root};
    }
    splay(c, true);
    np right = c->r;
    if (!right) return {c, nullptr};
    right->p = nullptr;
    c->r = nullptr;
    c->update();
    return {c, right};
  }
  // 左側のノード全体の prod が check を満たすように切る
  // 按区间聚合值判断查找最右侧满足条件的位置
  template <typename F>
  pair<np, np> split_max_right_prod(np root, F check) {
    if (!root) return {nullptr, nullptr};
    assert(!root->p);
    np c = find_max_right_prod(root, check);
    if (!c) {
      splay(root, true);
      return {nullptr, root};
    }
    splay(c, true);
    np right = c->r;
    if (!right) return {c, nullptr};
    right->p = nullptr;
    c->r = nullptr;
    c->update();
    return {c, right};
  }

  // 从根搜索最右侧满足 check(root->x) 的节点
  template <typename F>
  np find_max_right(np root, const F &check) {
    // 最後に見つけた ok の点、最後に探索した点
    np last_ok = nullptr, last = nullptr;
    while (root) {
      last = root;
      root->push();
      if (check(root->x)) {
        last_ok = root;
        root = root->r;
      } else {
        root = root->l;
      }
    }
    splay(last, true);
    return last_ok;
  }

  // 从根搜索最右侧满足 check(root->x, idx) 的节点，idx 为当前节点在序列中的位置
  template <typename F>
  np find_max_right_cnt(np root, const F &check) {
    // 最後に見つけた ok の点、最後に探索した点
    np last_ok = nullptr, last = nullptr;
    ll n = 0;
    while (root) {
      last = root;
      root->push();
      ll k = (root->size) - (root->r ? root->r->size : 0);
      if (check(root->x, n + k)) {
        last_ok = root;
        n += k;
        root = root->r;
      } else {
        root = root->l;
      }
    }
    splay(last, true);
    return last_ok;
  }

  // 从根搜索最右侧满足区间汇总 check 的节点
  template <typename F>
  np find_max_right_prod(np root, const F &check) {
    using Mono = typename Node::Monoid_X;
    X prod = Mono::unit();
    // 最後に見つけた ok の点、最後に探索した点
    np last_ok = nullptr, last = nullptr;
    while (root) {
      last = root;
      root->push();
      np tmp = root->r;
      root->r = nullptr;
      root->update();
      X lprod = Mono::op(prod, root->prod);
      root->r = tmp;
      root->update();
      if (check(lprod)) {
        prod = lprod;
        last_ok = root;
        root = root->r;
      } else {
        root = root->l;
      }
    }
    splay(last, true);
    return last_ok;
  }
};