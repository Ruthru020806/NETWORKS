#include <stdlib.h>
#include <string.h>

static inline unsigned int hash32(unsigned int x, unsigned int mask) {
    x *= 2654435761u;          /* Knuth multiplicative */
    x ^= x >> 15;
    return x & mask;
}

int subarrayBitwiseORs(int* arr, int arrSize) {
    /* At most 31 distinct ORs end at each index */
    long long need = (long long)arrSize * 31 + 2;
    unsigned int cap = 1u;
    while ((long long)cap < 2 * need) cap <<= 1;   /* load factor <= 0.5 */
    unsigned int mask = cap - 1;

    int *table = (int *)malloc((size_t)cap * sizeof(int));
    memset(table, 0xFF, (size_t)cap * sizeof(int)); /* all slots = -1 (empty) */

    int cur[40], nxt[40];      /* ORs ending at previous / current index */
    int curSize = 0, count = 0;

    for (int i = 0; i < arrSize; i++) {
        int v = arr[i];
        int nSize = 0;

        nxt[nSize++] = v;                    /* subarray [i, i] */
        for (int j = 0; j < curSize; j++) {  /* extend all previous ones */
            int o = cur[j] | v;
            if (o != nxt[nSize - 1]) nxt[nSize++] = o;  /* dups are adjacent */
        }

        for (int j = 0; j < nSize; j++) {
            cur[j] = nxt[j];                 /* carry to next iteration */

            unsigned int idx = hash32((unsigned int)nxt[j], mask);
            while (table[idx] != -1 && table[idx] != nxt[j])
                idx = (idx + 1) & mask;      /* linear probing */
            if (table[idx] == -1) {
                table[idx] = nxt[j];
                count++;
            }
        }
        curSize = nSize;
    }

    free(table);
    return count;
}
