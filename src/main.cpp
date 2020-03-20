// C++11
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <set>
#include <list>
#include <string>
#include <string.h>

using namespace std;

class Lossy2dCompression
{
public:
    vector<string> findSolution(double P, int N, vector< vector<string> > grids)
    {
        int maxHeight = 10;
        int maxWidth = 10;
        vector<string> out(maxHeight+N);          
        
        for (int r=0; r<maxHeight; r++)
        {
            out[r]="";
            for (int c=0; c<maxWidth; c++)
            {
                out[r]+=(char)('A'+(c^r));
            }
        }
        for (int i=0; i<N; i++)
            out[i+maxHeight]="0 0";
        return out;
    }
};

int main()
{
    double P;
    cin >> P;
    int N;
    cin >> N;
    vector< vector<string> > grids;
    for (int i=0;i<N;i++)
    {
        int H;
        cin >> H;
        vector<string> grid(H);
        for (int y=0;y<H;y++)
        {
            cin >> grid[y];
        }
        grids.push_back(grid);
    }
    
    Lossy2dCompression prog;
    vector<string> ret = prog.findSolution(P, N, grids);
    cout << ret.size()-N << endl;
    for (int i=0;i<ret.size();i++)
    {
        cout << ret[i] << endl;
    }
    cout.flush();
}
