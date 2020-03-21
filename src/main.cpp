// C++11
#include <cmath>
#include <string.h>
#include <sys/time.h>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define DEBUG

struct Timer {
  static constexpr int64_t CYCLES_PER_SEC = 2800000000;
  const double LIMIT;  // FIXME: 時間制限(s)
  int64_t start;
  Timer() : LIMIT(2.95) { reset(); }
  Timer(double limit) : LIMIT(limit) { reset(); }
  void reset() { start = getCycle(); }
  void plus(double a) { start -= (a * CYCLES_PER_SEC); }
  inline double get() { return (double)(getCycle() - start) / CYCLES_PER_SEC; }
  inline int64_t getCycle() {
    uint32_t low, high;
    __asm__ volatile("rdtsc"
                     : "=a"(low), "=d"(high));
    return ((int64_t)low) | ((int64_t)high << 32);
  }
};

struct XorShift {
  unsigned int x, y, z, w;
  double nL[65536];
  XorShift() { init(); }
  void init() {
    x = 314159265;
    y = 358979323;
    z = 846264338;
    w = 327950288;
    double n = 1 / (double)(2 * 65536);
    for (int i = 0; i < 65536; i++) {
      nL[i] = log(((double)i / 65536) + n);
    }
  }
  inline unsigned int next() {
    unsigned int t = x ^ x << 11;
    x = y;
    y = z;
    z = w;
    return w = w ^ w >> 19 ^ t ^ t >> 8;
  }
  inline double nextLog() { return nL[next() & 0xFFFF]; }
  inline int nextInt(int m) { return (int)(next() % m); }                       // [0, m)
  int nextInt(int min, int max) { return min + nextInt(max - min + 1); }        // [min, max]
  inline double nextDouble() { return (double)next() / ((long long)1 << 32); }  // [0, 1]
};

XorShift rnd;

namespace logger {
inline void json_() {}
template <typename Key, typename Value, typename... Rest>
void json_(const Key& key, const Value& value, const Rest&... rest) {
  std::cerr << "\"" << key << "\":\"" << value << "\"";
  if (sizeof...(Rest) > 0) std::cerr << ",";
  json_(rest...);
}

// example : json("key1", "foo", "key2", 3, "key", 4.0);
// {"key1":"foo","key2":"3","key":"4"}
template <typename... Args>
void json(const Args&... args) {
#ifdef DEBUG
  std::cerr << "{";
  json_(args...);
  std::cerr << "}\n";
#endif
}
}  // namespace logger

struct MedianSet2 {
  vector<int> data;
  int m; // 中央値
  int n; // 要素数
  int loss; // (中央値と各要素との差分の絶対値)の和
  MedianSet2() : m(0), n(0), loss(0), data(26, 0) {}

  int add(int x)  {
    ++n; ++data[x];
    int nm = (n+1)/2;
    int c = 0;
    for (int i = 0; i < 26; ++i) {
      if (data[i] == 0) continue;
      c += data[i];
      if (c >= nm) { m = i; break; }
    }
    int nloss = 0;
    for (int i = 0; i < 26; ++i) {
      nloss += data[i]*abs(i-m);
    }
    int dloss = nloss-loss;
    loss = nloss;
    return dloss;
  }

  int remove(int x) {
    --n; --data[x];
    if (n == 0) { m = loss = 0; return 0; }
    int nm = (n+1)/2;
    int c = 0;
    for (int i = 0; i < 26; ++i) {
      if (data[i] == 0) continue;
      c += data[i];
      if (c >= nm) { m = i; break; }
    }
    int nloss = 0;
    for (int i = 0; i < 26; ++i) {
      nloss += data[i]*abs(i-m);
    }
    int dloss = nloss-loss;
    loss = nloss;
    return -dloss;
  }

  int get() { return m; }
};

// 集合の中央値の取得, 集合への要素の追加, 削除がO(logQ)程度で行えるデータ構造
// @see https://wiki.kimiyuki.net/%E4%B8%AD%E5%A4%AE%E5%80%A4
struct MedianSet {
  int l; // 中央値より小さい要素の個数
  int r; // 中央値より大きい要素の個数
  int m; // 中央値
  int n; // 要素数
  int loss; // (中央値と各要素との差分の絶対値)の和
  MedianSet() : l(0), r(0), m(0), n(0), loss(0) {}
  std::map<int, int> data; 

  void update() { // l, r, mの値を適切なものに更新する
    if (n == 0) { l = r = m = loss = 0; return; }
    if (l >= data[m]+r) {
      int nm = std::prev(data.find(m))->first;
      r += data[m];
      l -= data[nm];
      m = nm;
    }
    else if (r > data[m]+l) {
      int nm = std::next(data.find(m))->first;
      l += data[m];
      r -= data[nm];
      m = nm;
    }
  }

  int add(int x)  {
    int bm = m, bl = l, br = r;
    ++data[x]; ++n;
    if (x < m) ++l;
    else if (x > m) ++r;
    update();
    if (n == 1) return 0;
    int dloss = (br-bl)*(bm-m)+abs(m-bm)+abs(x-m);
    loss += dloss;
    return dloss;
  }

  int remove(int x) {
    int bm = m, bl = l, br = r;
    --data[x]; --n;
    if (data[x] == 0) data.erase(x);
    if (x < m) --l;
    else if (x > m) --r;
    update();
    int dloss = (br-bl)*(m-bm)+abs(x-bm);
    loss -= dloss;
    return dloss;
  }

  int get() { return m; }
};

const int MAX_H = 20;
const int MAX_W = 20;
int N;
vector<vector<vector<int>>> grids;
using grid_t = vector<vector<int>>;
MedianSet2 memo[MAX_H][MAX_W]; // memo[y][x][c] : (x,y)にあるアルファベットの集合
// MedianSet memo[MAX_H][MAX_W]; // memo[y][x][c] : (x,y)にあるアルファベットの集合
struct Pos { int x, y; };

struct State {
  int bid;
  Pos bp;
  int bloss;
  int h, w;
  int loss;
  vector<Pos> lp;
  State() : bloss(0), loss(0), h(10), w(10), lp(N, {-1, -1}) {}
  State(int h, int w) : bloss(0), loss(0), h(h), w(w), lp(N, {-1, -1}) {}
  int update() {
    int id = rnd.nextInt(N);
    grid_t& grid = grids[id];
    int gh = grid.size();
    int gw = grid[0].size();
    Pos p = {rnd.nextInt(w-gw+1), rnd.nextInt(h-gh+1)};
    // cerr << "[0]" << id << " " << p.x << " " << p.y << endl;
    return update(id, p);
  }
  int update(int id) {
    grid_t& grid = grids[id];
    int gh = grid.size();
    int gw = grid[0].size();
    Pos p = {rnd.nextInt(w-gw+1), rnd.nextInt(h-gh+1)};
    return update(id, p);
  }
  int update(int id, const Pos& p) { // idのgridをpに移動する
    bp = lp[id];
    bloss = loss;
    bid = id;
    grid_t& grid = grids[id];
    int gh = grid.size();
    int gw = grid[0].size();
    int bx = lp[id].x;
    int by = lp[id].y;
    for (int dy = 0; dy < gh; ++dy) {
      int y = dy+by;
      for (int dx = 0; dx < gw; ++dx) {
        int x = dx+bx;
        if (bx >= 0 && by >= 0) {
          loss -= memo[y][x].remove(grid[dy][dx]);
        }
      }
    }
    for (int dy = 0; dy < gh; ++dy) {
      int y = dy+p.y;
      for (int dx = 0; dx < gw; ++dx) {
        int x = dx+p.x;
        loss += memo[y][x].add(grid[dy][dx]);
      }
    }
    lp[id] = p;
    return bloss-loss;
  }
  int calcScore() { return -loss; } // 高い程よい
  void revert() {
    // cerr << "[]" << bid << " " << bp.x << " " << bp.y << endl;
    Pos p = bp;
    update(bid, p);
  }
};

struct SASolver {
  double startTemp = 30;
  double endTemp = 1;
  Timer timer = Timer(5.0);
  State best;
  vector<Pos> best_lp;
  SASolver() { init(); }
  SASolver(double st, double et): startTemp(st), endTemp(et) { init(); }
  SASolver(double st, double et, double limit): startTemp(st), endTemp(et), timer(limit) { init(); }
  void init() {
    logger::json("tag", "param", "type", "sa", "start_temp", startTemp, "end_temp", endTemp, "limit", timer.LIMIT);
  } // 初期化処理をここに書く

  void solve(State &state) {
    double t;
    double score = state.calcScore();
    best = state;
    double bestScore = score;
    int counter = 0;
    while ((t = timer.get()) < timer.LIMIT) // 焼きなまし終了時刻までループ
    {
      for (int i = 0; i < 1000; ++i) { // 時間計算を間引く
        int diff = state.update();
        if (diff == -1000000) { // 絶対に更新しない場合
          state.revert();
          continue;
        }

        // 最初t=0のときは、スコアが良くなろうが悪くなろうが、常にnextを使用
        // 最後t=timer.LIMITのときは、スコアが改善したときのみ、nextを使用
        double T = startTemp + (endTemp - startTemp) * t / timer.LIMIT;
        // スコアが良くなった or 悪くなっても強制遷移
        if (diff >= T*rnd.nextLog())
        {
          score += diff;
          if (bestScore < score) {
            bestScore = score;
            best = state;
            logger::json("tag", "!", "time", t, "counter", counter, "score", score);
          }
        }
        else { state.revert(); }
        ++counter;
      }
    }
    logger::json("tag", "result", "counter", counter, "score", bestScore);
  }
};

struct Solver {
  Timer timer = Timer(5);
  vector<vector<int>> ogrid;
  vector<Pos> lp;
  State out;
  void read() {
    double P; cin >> P;
    cin >> N;
    lp.resize(N);
    for (int i = 0; i < N; i++) {
      int H; cin >> H;
      vector<vector<int>> grid(H);
      for (int y = 0; y < H; y++) {
        string s; cin >> s;
        vector<int> row(s.size());
        for (int x = 0; x < s.size(); ++x) {
          row[x] = s[x]-'A';
        }
        grid[y] = row;
      }
      grids.push_back(grid);
    }
  }

  void solve() {
    int h = 0;
    int w = 0;
    for (auto & grid : grids) {
      h = max(h, (int)grid.size());
      w = max(w, (int)grid[0].size());
    }
    State s(h, w);
    for (int i = 0; i < N; ++i) {
      int s1 = s.calcScore();
      int diff = s.update(i);
      int s2 = s.calcScore();
      // cerr << s1 << " " << diff << " " << s2 << endl;
    }
    // for (int i = 0; i < 10; ++i) {
    //   int s1 = s.calcScore();
    //   int diff = s.update();
    //   int s2 = s.calcScore();
    //   s.revert();
    //   int s3 = s.calcScore();
    //   // cerr << s1 << " " << diff << " " << s2 << " " << s3 << endl;
    // }
    SASolver sa;
    sa.solve(s);
    out = sa.best;
  }

  void write() {
    int H = out.h;
    cout << H << endl;
    int W = out.w;
    for (int y = 0; y < H; ++y) {
      for (int x = 0; x < W; ++x) {
        int d = memo[y][x].get();
        cout << (char)('A'+d);
      }
      cout << endl;
    }
    for (auto p : out.lp) {
      cout << p.x << " " << p.y << endl;
    }
    cout.flush();
  }
};

int main() {
  Solver solver;
  solver.read();
  solver.solve();
  solver.write();
}