#include <stdlib.h>
#include <string.h>

typedef struct {
    int to;
    int cost; // 1 if this edge needs to be reversed when traversed from parent to child (i.e., original direction was parent->child)
    int next;
} Edge;

int minReorder(int n, int** connections, int connectionsSize, int* connectionsColSize) {
    // Build adjacency list (undirected graph with cost info)
    Edge* edges = (Edge*)malloc(sizeof(Edge) * 2 * connectionsSize);
    int* head = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) head[i] = -1;
    
    int edgeCount = 0;
    
    for (int i = 0; i < connectionsSize; i++) {
        int a = connections[i][0];
        int b = connections[i][1];
        
        // original edge a -> b, so traversing a->b needs no change (cost 0)
        // traversing b->a needs change (cost 1)
        
        edges[edgeCount].to = b;
        edges[edgeCount].cost = 1; // if we go from a to b in our traversal (away from root), this matches original direction, so to make it point TOWARD root we'd need to reverse... 
        edges[edgeCount].next = head[a];
        head[a] = edgeCount;
        edgeCount++;
        
        edges[edgeCount].to = a;
        edges[edgeCount].cost = 0; // going from b to a in traversal means original edge (a->b) already points from a to b, i.e., away from root at b's side... let's just handle logic in BFS carefully.
        edges[edgeCount].next = head[b];
        head[b] = edgeCount;
        edgeCount++;
    }
    
    // BFS from node 0
    int* visited = (int*)calloc(n, sizeof(int));
    int* queue = (int*)malloc(sizeof(int) * n);
    int qHead = 0, qTail = 0;
    
    queue[qTail++] = 0;
    visited[0] = 1;
    
    int totalCost = 0;
    
    while (qHead < qTail) {
        int cur = queue[qHead++];
        
        for (int e = head[cur]; e != -1; e = edges[e].next) {
            int nxt = edges[e].to;
            if (!visited[nxt]) {
                visited[nxt] = 1;
                totalCost += edges[e].cost;
                queue[qTail++] = nxt;
            }
        }
    }
    
    free(edges);
    free(head);
    free(visited);
    free(queue);
    
    return totalCost;
}
