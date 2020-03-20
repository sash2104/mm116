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

vector<vector<vector<int>>> grids;
struct Pos { int x, y; };

struct Solver {
  int N;
  Timer timer = Timer(5);
  vector<vector<int>> ogrid;
  vector<Pos> lp;
  void read() {
    double P;
    cin >> P;
    int N;
    cin >> N;
    lp.resize(N);
    for (int i = 0; i < N; i++) {
      int H;
      cin >> H;
      vector<vector<int>> grid(H);
      for (int y = 0; y < H; y++) {
        string s;
        cin >> s;
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
    ogrid.resize(h);
    for (int y = 0; y < h; ++y) {
      vector<int> row(w, 12);
      ogrid[y] = row;
    }
  }

  void write() {
    int H = ogrid.size();
    cout << H << endl;
    for (auto& row : ogrid) {
      for (int d : row) {
        cout << (char)('A'+d);
      }
      cout << endl;
    }
    for (auto p : lp) {
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