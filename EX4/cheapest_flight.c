#include <stdlib.h>
#include <limits.h>
#include <string.h>

int findCheapestPrice(int n, int** flights, int flightsSize, int* flightsColSize, int src, int dst, int k) {
    int INF = INT_MAX;
    
    int* dist = (int*)malloc(sizeof(int) * n);
    int* temp = (int*)malloc(sizeof(int) * n);
    
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
    }
    dist[src] = 0;
    
    // Bellman-Ford: relax edges at most k+1 times (k stops means k+1 edges)
    for (int i = 0; i <= k; i++) {
        memcpy(temp, dist, sizeof(int) * n);
        
        for (int j = 0; j < flightsSize; j++) {
            int u = flights[j][0];
            int v = flights[j][1];
            int w = flights[j][2];
            
            if (dist[u] != INF && dist[u] + w < temp[v]) {
                temp[v] = dist[u] + w;
            }
        }
        
        memcpy(dist, temp, sizeof(int) * n);
    }
    
    int result = (dist[dst] == INF) ? -1 : dist[dst];
    
    free(dist);
    free(temp);
    
    return result;
}
