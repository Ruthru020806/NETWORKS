#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_STOPS 1000000

// Structure to build an adjacency list representing: Stop -> List of Bus IDs
typedef struct {
    int *buses;
    int count;
    int capacity;
} StopBuses;

// Structure for Queue elements
typedef struct {
    int stop;
    int bus_count;
} QueueNode;

int numBusesToDestination(int** routes, int routesSize, int* routesColSize, int source, int target) {
    // If you are already at the destination, no buses are needed
    if (source == target) {
        return 0;
    }


    // Allocate memory for tracking stop-to-bus connections globally dynamically
    StopBuses* stop_to_buses = (StopBuses*)calloc(MAX_STOPS, sizeof(StopBuses));

    // Fill the stop_to_buses adjacency lists
    stop_to_buses = (StopBuses*)calloc(MAX_STOPS, sizeof(StopBuses));

    // Fill the stop_to_buses adjacency lists
    for (int bus_id = 0; bus_id < routesSize; bus_id++) {
        for (int j = 0; j < routesColSize[bus_id]; j++) {
            int stop = routes[bus_id][j];
            
            // Dynamic resizing for the bus array at this stop if needed
            if (stop_to_buses[stop].count == stop_to_buses[stop].capacity) {
                stop_to_buses[stop].capacity = stop_to_buses[stop].capacity == 0 ? 4 : stop_to_buses[stop].capacity * 2;
                stop_to_buses[stop].buses = (int*)realloc(stop_to_buses[stop].buses, stop_to_buses[stop].capacity * sizeof(int));
            }
            stop_to_buses[stop].buses[stop_to_buses[stop].count++] = bus_id;
        }
    }

    // Track visited states to prevent infinite loops
    bool* visited_stops = (bool*)calloc(MAX_STOPS, sizeof(bool));
    bool* visited_buses = (bool*)calloc(routesSize, sizeof(bool));

    // Calculate maximum queue size (sum of all stops across routes + 1 starter)
    int total_stops = 1;
    for (int i = 0; i < routesSize; i++) {
        total_stops += routesColSize[i];
    }

    QueueNode* queue = (QueueNode*)malloc(total_stops * sizeof(QueueNode));
    int head = 0, tail = 0;

    // Push the initial source stop onto the queue
    queue[tail++] = (QueueNode){source, 0};
    visited_stops[source] = true;

    int result = -1;

    // Process BFS queue
    while (head < tail) {
        QueueNode current = queue[head++];
        int current_stop = current.stop;
        int bus_count = current.bus_count;

        // Iterate through all buses passing through the current stop
        for (int i = 0; i < stop_to_buses[current_stop].count; i++) {
            int bus_id = stop_to_buses[current_stop].buses[i];

            if (visited_buses[bus_id]) {
                continue;
            }
            visited_buses[bus_id] = true;

            // Look at all stops reachable by this specific bus
            for (int j = 0; j < routesColSize[bus_id]; j++) {
                int next_stop = routes[bus_id][j];

                if (next_stop == target) {
                    result = bus_count + 1;
                    goto cleanup; // Exit nested loops cleanly
                }

                if (!visited_stops[next_stop]) {
                    visited_stops[next_stop] = true;
                    queue[tail++] = (QueueNode){next_stop, bus_count + 1};
                }
            }
        }
    }

cleanup:
    // Free all dynamically allocated memory to prevent memory leaks
    for (int i = 0; i < MAX_STOPS; i++) {
        if (stop_to_buses[i].buses != NULL) {
            free(stop_to_buses[i].buses);
        }
    }
    free(stop_to_buses);
    free(visited_stops);
    free(visited_buses);
    free(queue);

    return result;
}
