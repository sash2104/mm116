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
#include <unordered_map>

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
  int data[26];
  int m; // 中央値
  int n; // 要素数
  int loss; // (中央値と各要素との差分の絶対値)の和
  MedianSet2() : m(0), n(0), loss(0) {
    for (int i = 0; i < 26; ++i) data[i] = 0;
  }

  int add(int x)  {
    ++n; ++data[x];
    int nm = (n+1)/2;
    int c = 0;
    for (int i = 0; i < 26; ++i) {
      c += data[i];
      if (c >= nm) { m = i; break; }
    }
    int nloss = 0;
    for (int i = 0; i < m; ++i) {
      nloss += data[i]*(m-i);
    }
    for (int i = m+1; i < 26; ++i) {
      nloss += data[i]*(i-m);
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
      c += data[i];
      if (c >= nm) { m = i; break; }
    }
    int nloss = 0;
    for (int i = 0; i < m; ++i) {
      nloss += data[i]*(m-i);
    }
    for (int i = m+1; i < 26; ++i) {
      nloss += data[i]*(i-m);
    }
    int dloss = nloss-loss;
    loss = nloss;
    return -dloss;
  }

  int get() { return m; }

  // void show() {
  //   cerr << m << " " << n << " " << loss << endl;
  //   for (int i = 0; i < 26; ++i) {
  //     if (data[i] == 0) continue;
  //     cerr << i << " " << data[i] << endl;
  //   }
  // }
};


const int MAX_H = 200;
const int MAX_W = 200;
int N;
vector<vector<vector<int>>> grids;
using grid_t = vector<vector<int>>;
MedianSet2 memo[MAX_H][MAX_W]; // memo[y][x][c] : (x,y)にあるアルファベットの集合
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
    if (gh == h && gw == w) return -1001001;
    Pos p = {rnd.nextInt(w-gw+1), rnd.nextInt(h-gh+1)};
    // cerr << "[0]" << id << " " << p.x << " " << p.y << endl;
    int diff = update(id, p);
    return diff;
  }
  int update(int id) {
    grid_t& grid = grids[id];
    int gh = grid.size();
    int gw = grid[0].size();
    Pos p = {rnd.nextInt(w-gw+1), rnd.nextInt(h-gh+1)};
    int diff = update(id, p);
    return diff;
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
    if (bx >= 0) {
      for (int dy = 0; dy < gh; ++dy) {
        int y = dy+by;
        for (int dx = 0; dx < gw; ++dx) {
          int x = dx+bx;
          int dl = memo[y][x].remove(grid[dy][dx]);
          loss -= dl;
        }
      }
    }
    for (int dy = 0; dy < gh; ++dy) {
      int y = dy+p.y;
      for (int dx = 0; dx < gw; ++dx) {
        int x = dx+p.x;
        int dl = memo[y][x].add(grid[dy][dx]);
        loss += dl;
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
  double endTemp = 0.01;
  Timer timer = Timer(0.3);
  State best;
  grid_t ogrid;
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
        if (diff == -1001001) { // 絶対に更新しない場合
          // state.revert();
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
            ogrid = grid_t(best.h, vector<int>(best.w));
            for (int y = 0; y < best.h; ++y) {
              for (int x = 0; x < best.w; ++x) {
                ogrid[y][x] = memo[y][x].get();
              }
            }
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

struct Answer { 
  int h, w;
  grid_t ogrid;
  vector<Pos> lp;
  double comp;
  double loss;
  double score;
};

struct Solver {
  Timer timer = Timer(9.8);
  grid_t ogrid;
  vector<Answer> answers;
  vector<Pos> lp;
  State out;
  int T;
  double P;
  void read() {
    cin >> P;
    cin >> N;
    lp.resize(N);
    T = 0;
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
      int W = grid[0].size();
      T += H*W;
      grids.push_back(grid);
    }
  }

  void solve_greedy(int ms=0, bool vertical=false) {
    vector<int> gids;
    for (int i = 0; i < N; ++i) {
      gids.push_back(i);
    }
    int mh, mw = 0;
    if (vertical) {
      mw = ms;
      sort(gids.begin(), gids.end(), [&](int l, int r) {
        int lh = grids[l].size();
        int lw = grids[l][0].size();
        int rh = grids[r].size();
        int rw = grids[r][0].size();
        if (lw == rw) return lh > rh;
        return lw > rw;
      });
    }
    else {
      mh = ms;
      sort(gids.begin(), gids.end(), [&](int l, int r) {
        int lh = grids[l].size();
        int lw = grids[l][0].size();
        int rh = grids[r].size();
        int rw = grids[r][0].size();
        if (lh == rh) return lw > rw;
        return lh > rh;
      });
    }
    vector<Pos> put; // おいた場所
    vector<Pos> cand;
    int ma = 500;
    vector<vector<bool>> placed(ma, vector<bool>(ma));
    cand.push_back({0, 0});
    for (int id : gids) {
      int h = grids[id].size();
      int w = grids[id][0].size();
      int best = ma*ma;
      Pos bp;
      for (Pos p : cand) {
        if (p.y+h >= ma || p.x+w >= ma) continue;
        int cnt = 0;
        for (int y = p.y; y < p.y+h; ++y) {
          for (int x = p.x; x < p.x+w; ++x) {
            if (placed[y][x]) ++cnt;
          }
        }
        if (cnt*10 > h*w) continue;
        int h2 = max(mh, (p.y+h));
        int w2 = max(mw, (p.x+w));
        int score = h2*w2+abs(h2-w2);
        if (score < best) {
          bp = p;
          best = score;
        }
      }
      put.push_back(bp);
      mh = max(mh, bp.y+h);
      mw = max(mw, bp.x+w);
      for (int y = bp.y; y < bp.y+h; ++y) {
        for (int x = bp.x; x < bp.x+w; ++x) {
          placed[y][x] = true;
        }
      }
      cand.push_back({bp.x+w, bp.y});
      cand.push_back({bp.x, bp.y+h});
      vector<Pos> ncand;
      for (Pos p : cand) {
        if (placed[p.y][p.x]) continue;
        ncand.push_back(p);
      }
      cand = ncand;
      // cerr << id << " " << mh << " " << mw << endl;
    }
    Answer ans;
    ans.h = mh;
    ans.w = mw;
    ans.lp = vector<Pos>(N);
    ans.ogrid = grid_t(mh, vector<int>(mw));
    for (int i = 0; i < N; ++i) {
      int id = gids[i];
      Pos p = put[i];
      ans.lp[id] = p;
      grid_t &g = grids[id];
      for (int y = 0; y < g.size(); ++y) {
        for (int x = 0; x < g[0].size(); ++x) {
          ans.ogrid[y+p.y][x+p.x] = g[y][x];
        }
      }
    }
    ans.loss = 0;
    ans.comp = ans.h*ans.w / (double)T;
    ans.score = ans.comp*P+ans.loss*(1-P);
    if (answers.size() > 0 && answers[0].score <= ans.score) return;
    answers.push_back(ans);
    logger::json("type","greedy_loss","c",ans.comp,"l",ans.loss,"score",ans.score, "h",ans.h,"w",ans.w);
    // cerr << ans.comp << " " << ans.loss << " " << ans.score << endl;
    // for (int y = 0; y < mh; ++y) {
    //   for (int x = 0; x < mw; ++x) {
    //     if (placed[y][x]) cerr << 1;
    //     else cerr << ".";
    //   }
    //   cerr << endl;
    // }
    // for (int y = 0; y < 20; ++y) {
    //   for (int x = 0; x < 220; ++x) {
    //     if (placed[y][x]) cerr << 1;
    //     else cerr << ".";
    //   }
    //   cerr << endl;
    // }
  }

  double solve_sa(int h, int w, double limit = 0.3) {
    State s(h, w);
    for (int i = 0; i < N; ++i) {
      int diff = s.update(i);
      // cerr << s1 << " " << diff << " " << s2 << endl;
    }
    SASolver sa(30, 1, limit);
    sa.solve(s);
    Answer ans;
    ans.h = sa.best.h;
    ans.w = sa.best.w;
    ans.lp = sa.best.lp;
    ans.ogrid = sa.ogrid;
    ans.loss = sa.best.loss/(12.5*T);
    ans.comp = ans.h*ans.w / (double)T;
    ans.score = ans.comp*P+ans.loss*(1-P);
    logger::json("type","sa","c",ans.comp,"l",ans.loss,"score",ans.score);
    answers.push_back(ans);
    return ans.score;
  }
  void solve() {
    for (int s = 0; s <= 50; ++s) {
      solve_greedy(s);
      solve_greedy(s,true);
    }
    int h = 0;
    int w = 0;
    int sum = 0;
    for (auto & grid : grids) {
      int h0 = grid.size();
      int w0 = grid[0].size();
      h = max(h, h0);
      w = max(w, w0);
      sum += h0*w0;
    }
    double s_comp = (h*w/T)*P+0.45*(1-P);
    double s_loss = P+0.02*(1-P);
    logger::json("s_comp",s_comp,"s_loss",s_loss,"T",T,"P",P);
    double best = 100;
    int bh = h, bw = w;
    if (s_comp < s_loss) {
      int bh = h, bw = w;
      // for (int oh =0; oh < 5; ++oh) {
      //   for (int ow =0; ow < 5; ++ow) {
      //     double sc = solve_sa(h+oh, w+ow, 3);
      //     if (sc < best) {
      //       best = sc;
      //       bh = h+oh;
      //       bw = w+ow;
      //     }
      //   }
      // }
      solve_sa(bh, bw, timer.LIMIT-timer.get());
    }
    else {
      int sz = sqrt(sum*1.02);
      solve_sa(max(sz,h), max(sz,w), timer.LIMIT-timer.get());
    }
  }

  void write() {
    int bid = 0;
    double best = answers[0].score;
    for (int i = 1; i < answers.size(); ++i) {
      if (answers[i].score < best) {
        bid = i;
        best = answers[i].score;
      }
    }
    Answer &ans = answers[bid];
    cout << ans.h << endl;
    int W = out.w;
    for (int y = 0; y < ans.h; ++y) {
      for (int x = 0; x < ans.w; ++x) {
        cout << (char)('A'+ans.ogrid[y][x]);
      }
      cout << endl;
    }
    for (auto p : ans.lp) {
      cout << p.y << " " << p.x << endl;
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