#include <stdio.h>
#include <stdlib.h>

#define MAX_BITS 100000

/* A position is a parity position iff it is a power of two (1,2,4,8,16..) */
int is_parity_pos(int pos) {
    return pos > 0 && (pos & (pos - 1)) == 0;
}

void print_codeword_table(int *code, int total, int d) {
    printf("%-10s %-8s %-6s\n", "Position", "Type", "Value");
    printf("----------------------------------\n");
    int dcount = 0;
    for (int pos = 1; pos <= total; pos++) {
        if (is_parity_pos(pos)) {
            printf("%-10d P%-7d %-6d\n", pos, pos, code[pos]);
        } else {
            dcount++;
            printf("%-10d D%-7d %-6d\n", pos, dcount, code[pos]);
        }
    }
}

int main(void) {
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        printf("Error: could not open input.txt\n");
        return 1;
    }

    /* ---- Step 1 & 2: read file, convert every char to 8 bits ---- */
    char rawbits[MAX_BITS];
    int nbits = 0, ch;
    while ((ch = fgetc(fp)) != EOF && nbits < MAX_BITS - 8) {
        for (int i = 7; i >= 0; i--)
            rawbits[nbits++] = ((ch >> i) & 1) ? '1' : '0';
    }
    fclose(fp);

    if (nbits == 0) {
        printf("Error: input.txt is empty\n");
        return 1;
    }

    int d = nbits;

    /* ---- Step 3: find smallest p such that 2^p >= p + d + 1 ---- */
    int p = 0;
    while ((1 << p) < (p + d + 1)) p++;
    int total = d + p;

    printf("========================================================\n");
    printf(" SENDER : Hamming Code Construction\n");
    printf("========================================================\n");
    printf("Data bits (d)   : %d\n", d);
    printf("Parity bits (p) : %d   (smallest p with 2^p >= p+d+1)\n", p);
    printf("Total bits (n)  : %d   -> Hamming(%d,%d)\n\n", total, total, d);

    /* ---- Step 4: place data bits into non-parity positions ---- */
    int *code = calloc(total + 1, sizeof(int));   /* 1-indexed */
    int di = 0;
    for (int pos = 1; pos <= total; pos++) {
        if (!is_parity_pos(pos)) code[pos] = rawbits[di++] - '0';
    }

    /* ---- Step 5: compute each parity bit (even parity) ---- */
    for (int k = 0; (1 << k) <= total; k++) {
        int ppos = 1 << k;
        int parity = 0;
        for (int pos = 1; pos <= total; pos++) {
            if (pos == ppos) continue;
            if (pos & ppos) parity ^= code[pos];   /* pos belongs to this group */
        }
        code[ppos] = parity;
    }

    /* ---- Step 6: print the organised table ---- */
    printf("Codeword layout (P = parity bit, D = data bit):\n");
    print_codeword_table(code, total, d);

    printf("\nFinal transmitted codeword: ");
    for (int pos = 1; pos <= total; pos++) printf("%d", code[pos]);
    printf("\n");

    /* ---- Step 7: write everything the receiver needs ---- */
    FILE *out = fopen("transmission.dat", "w");
    if (!out) {
        printf("Error: could not create transmission.dat\n");
        free(code);
        return 1;
    }
    fprintf(out, "%d %d %d\n", total, d, p);
    for (int pos = 1; pos <= total; pos++) fprintf(out, "%d ", code[pos]);
    fprintf(out, "\n");
    fclose(out);

    printf("\nSender: Hamming codeword written to transmission.dat\n");
    printf("Run receiver.c next to simulate bit errors and check/correct them.\n");

    free(code);
    return 0;
}
