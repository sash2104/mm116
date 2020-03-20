import sys

class Lossy2dCompression:
    def findSolution(self, P, N, grids):

        maxHeight = 10
        maxWidth = 10
        out = []

        for r in range(maxHeight):
            row = ""
            for c in range(maxWidth): 
                row += chr(ord('A')+(c^r))
            out.append(row)
        for i in range(N):
            out.append("0 0")
        return out

P = float(input())   
N = int(input())
grids = []
for i in range(N): 
    grid = []
    H = int(input())
    for y in range(H):
        grid.append(input())
    grids.append(grid)

prog = Lossy2dCompression()
ret = prog.findSolution(P, N, grids)
print(len(ret)-N)
for st in ret:
    print(st)
sys.stdout.flush()
