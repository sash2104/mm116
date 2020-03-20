#include "testlib.h"
using namespace std;
#include<cmath>
#include<iostream>
#include<iomanip>
#include<vector>

namespace logger {
inline void json_() {}
template<typename Key, typename Value, typename... Rest>
void json_(const Key& key, const Value& value, const Rest&... rest)
{
  std::cout << "\"" << key << "\":\"" << value << "\"";
  if (sizeof...(Rest) > 0) std::cout << ",";
  json_(rest...);
}

// example : json("key1", "foo", "key2", 3, "key", 4.0);
// {"key1":"foo","key2":"3","key":"4"}
template<typename... Args>
void json(const Args&... args)
{
  std::cout << "{"; json_(args...); std::cout << "}\n";
}
} // namespace logger

int main(int argc, char* argv[])
{
	setName("marathon match 116");
	registerTestlibCmd(argc, argv);

	try {
		double P = inf.readDouble();
		inf.readEoln();
		int N = inf.readInt();
		inf.readEoln();
		int T = 0; // total sum of the 2D input grid areas
		vector<vector<vector<int> > > in(N);
		for (int i = 0; i < N; ++i) {
			int H = inf.readInt();
			inf.readEoln();
			vector<vector<int> > grid(H, vector<int>());
			for (int j = 0; j < H; ++j) {
				string s = inf.readString();
				for (int k = 0; k < s.size(); ++k) {
					grid[j].push_back(s[k]-'A');
				}
			}
			T += H*grid[0].size();
			in[i] = grid;
		}

		int H = ouf.readInt();
		ouf.readEoln();
		vector<vector<int> > mout(H, vector<int>());
		for (int i = 0; i < H; ++i) {
			string s = ouf.readString();
			for (int j = 0; j < s.size(); ++j) {
				mout[i].push_back(s[j]-'A');
			}
		}
		int W = mout[0].size();
		int loss = 0;
		for (int i = 0; i < N; ++i) {
			int x = ouf.readInt();
			int y = ouf.readInt();
			ouf.readEoln();
			vector<vector<int>> &grid = in[i];
			for (int j = 0; j < grid.size(); ++j) {
				for (int k = 0; k < grid[j].size(); ++k) {
					loss += abs(grid[j][k]-mout[y+j][x+k]);
				}
			}
		}

		double compressionScore = (double)H*(double)W/(double)T;
		double lossinessScore = (double)loss/(12.5*T);
		double score = compressionScore*P+lossinessScore*(1-P);

		// calc score
		// cout << setprecision(10);
		logger::json("status", "AC", "score", score, "c", compressionScore, "l", lossinessScore, "P", P, "N", N, "T", T, "H", H, "W", W);

		quitf(_ok, "Correct");
	}
	catch (char* str) {
		logger::json("status", "WA", "message", str, "score", "-1");
		// cerr << "error: " << str << endl;
		quitf(_wa, "Something error occured.");
	}
}
