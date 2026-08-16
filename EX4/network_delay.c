#include <stdlib.h>
#include <limits.h>
#include <string.h>

int networkDelayTime(int** times, int timesSize, int* timesColSize, int n, int k) {
    // Build adjacency matrix since n <= 100, times.length <= 6000
    int INF = INT_MAX / 2;
    
    int** graph = (int**)malloc(sizeof(int*) * (n + 1));
    for (int i = 0; i <= n; i++) {
        graph[i] = (int*)malloc(sizeof(int) * (n + 1));
        for (int j = 0; j <= n; j++) {
            graph[i][j] = INF;
        }
    }
    
    for (int i = 0; i < timesSize; i++) {
        int u = times[i][0];
        int v = times[i][1];
        int w = times[i][2];
        graph[u][v] = w;
    }
    
    int* dist = (int*)malloc(sizeof(int) * (n + 1));
    int* visited = (int*)calloc(n + 1, sizeof(int));
    
    for (int i = 0; i <= n; i++) {
        dist[i] = INF;
    }
    dist[k] = 0;
    
    // Dijkstra's algorithm using simple O(n^2) approach (fine since n <= 100)
    for (int count = 0; count < n; count++) {
        // Find the unvisited node with minimum distance
        int minDist = INF;
        int u = -1;
        
        for (int i = 1; i <= n; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }
        
        if (u == -1) break; // remaining nodes are unreachable
        
        visited[u] = 1;
        
        // Relax edges from u
        for (int v = 1; v <= n; v++) {
            if (graph[u][v] < INF && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }
    
    // Find maximum distance among all nodes
    int maxDist = 0;
    for (int i = 1; i <= n; i++) {
        if (dist[i] == INF) {
            maxDist = -1;
            break;
        }
        if (dist[i] > maxDist) {
            maxDist = dist[i];
        }
    }
    
    // Cleanup
    for (int i = 0; i <= n; i++) {
        free(graph[i]);
    }
    free(graph);
    free(dist);
    free(visited);
    
    return maxDist;
}
