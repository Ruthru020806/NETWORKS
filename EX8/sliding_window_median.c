#include <stdlib.h>
#include <string.h>

// Definition for a simple Hash Map Node to keep track of elements to be lazily deleted
typedef struct HashNode {
    long long key;
    int count;
    struct HashNode* next;
} HashNode;

#define HASH_SIZE 2053

typedef struct {
    HashNode* table[HASH_SIZE];
} HashMap;

void hash_insert(HashMap* map, long long key) {
    int idx = abs((int)(key % HASH_SIZE));
    HashNode* curr = map->table[idx];
    while (curr) {
        if (curr->key == key) {
            curr->count++;
            return;
        }
        curr = curr->next;
    }
    HashNode* node = (HashNode*)malloc(sizeof(HashNode));
    node->key = key;
    node->count = 1;
    node->next = map->table[idx];
    map->table[idx] = node;
}

int hash_remove(HashMap* map, long long key) {
    int idx = abs((int)(key % HASH_SIZE));
    HashNode* curr = map->table[idx];
    while (curr) {
        if (curr->key == key) {
            curr->count--;
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

int hash_get(HashMap* map, long long key) {
    int idx = abs((int)(key % HASH_SIZE));
    HashNode* curr = map->table[idx];
    while (curr) {
        if (curr->key == key) return curr->count;
        curr = curr->next;
    }
    return 0;
}

void hash_free(HashMap* map) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* curr = map->table[i];
        while (curr) {
            HashNode* tmp = curr->next;
            free(curr);
            curr = tmp;
        }
    }
}

// Max Heap Functions (Lower Half)
void push_max(long long* heap, int* size, long long val) {
    int i = (*size)++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (heap[p] >= val) break;
        heap[i] = heap[p];
        i = p;
    }
    heap[i] = val;
}

long long pop_max(long long* heap, int* size) {
    long long res = heap[0];
    long long val = heap[--(*size)];
    int i = 0;
    while (i * 2 + 1 < *size) {
        int left = i * 2 + 1, right = i * 2 + 2, child = left;
        if (right < *size && heap[right] > heap[left]) child = right;
        if (heap[child] <= val) break;
        heap[i] = heap[child];
        i = child;
    }
    if (*size > 0) heap[i] = val;
    return res;
}

// Min Heap Functions (Upper Half)
void push_min(long long* heap, int* size, long long val) {
    int i = (*size)++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (heap[p] <= val) break;
        heap[i] = heap[p];
        i = p;
    }
    heap[i] = val;
}

long long pop_min(long long* heap, int* size) {
    long long res = heap[0];
    long long val = heap[--(*size)];
    int i = 0;
    while (i * 2 + 1 < *size) {
        int left = i * 2 + 1, right = i * 2 + 2, child = left;
        if (right < *size && heap[right] < heap[left]) child = right;
        if (heap[child] >= val) break;
        heap[i] = heap[child];
        i = child;
    }
    if (*size > 0) heap[i] = val;
    return res;
}

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
double* medianSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    *returnSize = numsSize - k + 1;
    double* result = (double*)malloc((*returnSize) * sizeof(double));
    
    long long* small = (long long*)malloc(numsSize * sizeof(long long)); // Max-heap
    long long* large = (long long*)malloc(numsSize * sizeof(long long)); // Min-heap
    int small_size = 0, large_size = 0;
    int small_valid = 0, large_valid = 0; // Number of non-deleted elements

    HashMap delayed;
    memset(&delayed, 0, sizeof(HashMap));

    // Helper closure macros to keep the balance logic clean
    #define prune() \
        while (small_size > 0 && hash_get(&delayed, small[0]) > 0) { \
            hash_remove(&delayed, small[0]); \
            pop_max(small, &small_size); \
        } \
        while (large_size > 0 && hash_get(&delayed, large[0]) > 0) { \
            hash_remove(&delayed, large[0]); \
            pop_min(large, &large_size); \
        }

    #define update_balance() \
        if (small_valid > large_valid + 1) { \
            push_min(large, &large_size, small[0]); \
            pop_max(small, &small_size); \
            small_valid--; large_valid++; \
            prune(); \
        } else if (small_valid < large_valid) { \
            push_max(small, &small_size, large[0]); \
            pop_min(large, &large_size); \
            small_valid++; large_valid--; \
            prune(); \
        }

    // Initialize with first window elements
    for (int i = 0; i < k; i++) {
        if (small_size == 0 || nums[i] <= small[0]) {
            push_max(small, &small_size, nums[i]);
            small_valid++;
        } else {
            push_min(large, &large_size, nums[i]);
            large_valid++;
        }
        update_balance();
    }

    result[0] = (k & 1) ? small[0] : ((double)small[0] + large[0]) * 0.5;

    // Slide window across the remainder of the array
    for (int i = k; i < numsSize; i++) {
        long long out_num = nums[i - k];
        long long in_num = nums[i];

        // 1. Handle elements leaving the window boundary (Lazy Deletion)
        hash_insert(&delayed, out_num);
        if (out_num <= small[0]) small_valid--;
        else large_valid--;

        // 2. Handle new elements arriving into the window
        if (small_size == 0 || in_num <= small[0]) {
            push_max(small, &small_size, in_num);
            small_valid++;
        } else {
            push_min(large, &large_size, in_num);
            large_valid++;
        }

        // 3. Balance and discard top elements if they were lazily deleted
        prune();
        update_balance();

        // 4. Capture the true running median
        result[i - k + 1] = (k & 1) ? small[0] : ((double)small[0] + large[0]) * 0.5;
    }

    // Cleanup resources
    hash_free(&delayed);
    free(small);
    free(large);

    return result;
}
