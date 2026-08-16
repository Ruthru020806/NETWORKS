#include <stdlib.h>
#include <string.h>

int rowsG, colsG;
int** heightsG;

int canReach(int mid, int rows, int cols) {
    int* visited = (int*)calloc(rows * cols, sizeof(int));
    int* queue = (int*)malloc(sizeof(int) * rows * cols);
    int qHead = 0, qTail = 0;
    
    queue[qTail++] = 0; // (0,0) encoded as row*cols + col
    visited[0] = 1;
    
    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};
    
    while (qHead < qTail) {
        int cur = queue[qHead++];
        int r = cur / cols;
        int c = cur % cols;
        
        if (r == rows - 1 && c == cols - 1) {
            free(visited);
            free(queue);
            return 1;
        }
        
        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                int nidx = nr * cols + nc;
                if (!visited[nidx]) {
                    int diff = abs(heightsG[r][c] - heightsG[nr][nc]);
                    if (diff <= mid) {
                        visited[nidx] = 1;
                        queue[qTail++] = nidx;
                    }
                }
            }
        }
    }
    
    free(visited);
    free(queue);
    return 0;
}

int minimumEffortPath(int** heights, int heightsSize, int* heightsColSize) {
    int rows = heightsSize;
    int cols = heightsColSize[0];
    
    heightsG = heights;
    rowsG = rows;
    colsG = cols;
    
    if (rows == 1 && cols == 1) {
        return 0;
    }
    
    int lo = 0, hi = 1000000;
    int result = hi;
    
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        
        if (canReach(mid, rows, cols)) {
            result = mid;
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    
    return result;
}
