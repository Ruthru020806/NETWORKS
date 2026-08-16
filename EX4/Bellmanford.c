#include <stdio.h>
#include <limits.h>

#define MAX_VERTICES 100
#define MAX_EDGES 100
#define INF INT_MAX

/* An edge (u -> v) with weight w */
struct Edge {
    int u, v, w;
};

int V, E; /* number of vertices, number of edges */
struct Edge edges[MAX_EDGES]; /* list of all edges */
int d[MAX_VERTICES]; /* shortest distance from source */
int parent[MAX_VERTICES]; /* parent of each vertex in shortest path */

/* Step 1: Initialize distances and parents */
void initializeSingleSource(int source) {
    for (int v = 0; v < V; v++) {
        d[v] = INF;
        parent[v] = -1; /* -1 represents NIL */
    }
    d[source] = 0;
}

/* Relax a single edge (u, v) with weight w */
void relax(int u, int v, int w) {
    if (d[u] != INF && d[u] + w < d[v]) {
        d[v] = d[u] + w;
        parent[v] = u;
    }
}

/* Step 2 + 3: Run Bellman-Ford, return 1 if no negative cycle, 0 if found */
int bellmanFord(int source) {
    initializeSingleSource(source);

    /* Main body: relax all edges, |V| - 1 times */
    for (int i = 1; i <= V - 1; i++) {
        for (int j = 0; j < E; j++) {
            relax(edges[j].u, edges[j].v, edges[j].w);
        }
    }

    /* Check for negative weight cycles: one extra pass */
    for (int j = 0; j < E; j++) {
        int u = edges[j].u, v = edges[j].v, w = edges[j].w;
        if (d[u] != INF && d[u] + w < d[v]) {
            return 0; /* negative weight cycle exists */
        }
    }

    return 1; /* no negative weight cycle */
}

/* Print the path from source to v using the parent array */
void printPath(int v) {
    if (v == -1) return;
    printPath(parent[v]);
    if (parent[v] != -1) printf(" -> ");
    printf("%d", v);
}

/* Simple, easy to read output of results */
void printResults(int source, int hasNoNegativeCycle) {
    if (!hasNoNegativeCycle) {
        printf("Negative weight cycle exists. Shortest paths are not defined.\n");
        return;
    }

    printf("Shortest distances from source vertex %d:\n\n", source);
    // Added "Next Step" column to the table header
    printf("Vertex\tDistance\tNext Step\tPath\n");

    for (int v = 0; v < V; v++) {
        printf("%d\t", v);
        if (d[v] == INF) {
            printf("INF\t\t-\t\tNo path\n");
        } else {
            printf("%d\t\t", d[v]);
            
            /* --- LOGIC TO FIND THE NEXT STEP FROM SOURCE --- */
            if (v == source) {
                printf("Self\t\t"); // Source node is already at the destination
            } else {
                int curr = v;
                // Trace backward from vertex v until we reach the node right after the source
                while (parent[curr] != source && parent[curr] != -1) {
                    curr = parent[curr];
                }
                printf("%d\t\t", curr);
            }
            /* ----------------------------------------------- */

            printPath(v);
            printf("\n");
        }
    }
}

int main() {
    int n; /* number of source vertices to run Bellman-Ford from */
    int sources[MAX_VERTICES];

    printf("Enter number of vertices: ");
    scanf("%d", &V);
    printf("Enter number of edges: ");
    scanf("%d", &E);

    printf("Enter each edge as: source destination weight\n");
    for (int i = 0; i < E; i++) {
        printf("Edge %d: ", i + 1);
        scanf("%d %d %d", &edges[i].u, &edges[i].v, &edges[i].w);
    }

    printf("How many source vertices do you want to run Bellman-Ford from? ");
    scanf("%d", &n);
    printf("Enter %d source vertex/vertices:\n", n);
    for (int i = 0; i < n; i++) {
        printf("Source %d: ", i + 1);
        scanf("%d", &sources[i]);
    }

    /* Run Bellman-Ford once for each chosen source */
    for (int i = 0; i < n; i++) {
        int result = bellmanFord(sources[i]);
        printf("\n============================================\n");
        printResults(sources[i], result);
    }

    return 0;
}
