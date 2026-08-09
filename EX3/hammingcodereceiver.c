#include <stdio.h>
#include <stdlib.h>

int is_parity_pos(int pos) {
    return pos > 0 && (pos & (pos - 1)) == 0;
}

void print_codeword_table(int *code, int total) {
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
    FILE *fp = fopen("transmission.dat", "r");
    if (!fp) {
        printf("Error: could not open transmission.dat (run sender.c first)\n");
        return 1;
    }

    int total, d, p;
    fscanf(fp, "%d %d %d", &total, &d, &p);

    int *code = calloc(total + 1, sizeof(int));
    for (int pos = 1; pos <= total; pos++) fscanf(fp, "%d", &code[pos]);
    fclose(fp);

    /* build a lookup: data-bit-number (1..d) -> actual codeword position */
    int *data_pos = calloc(d + 1, sizeof(int));
    {
        int dcount = 0;
        for (int pos = 1; pos <= total; pos++)
            if (!is_parity_pos(pos)) data_pos[++dcount] = pos;
    }

    printf("========================================================\n");
    printf(" RECEIVER : Hamming codeword as received\n");
    printf("========================================================\n");
    printf("Total bits: %d   Data bits: %d   Parity bits: %d\n\n", total, d, p);
    print_codeword_table(code, total);
    printf("\nReceived codeword: ");
    for (int pos = 1; pos <= total; pos++) printf("%d", code[pos]);
    printf("\n\n");

    /* ---- Step 2: ask which DATA bits to flip ---- */
    int n;
    printf("Enter number of DATA bits to flip (simulate transmission error): ");
    if (scanf("%d", &n) != 1 || n < 0) n = 0;

    for (int i = 0; i < n; i++) {
        int dnum;
        printf("  Bit %d -> enter data bit number to flip (D1..D%d): ", i + 1, d);
        if (scanf("%d", &dnum) != 1) { i--; continue; }
        if (dnum < 1 || dnum > d) {
            printf("  Invalid data bit number D%d - ignored.\n", dnum);
            continue;
        }
        int pos = data_pos[dnum];
        code[pos] ^= 1;
        printf("  -> flipped D%d (codeword position %d): now %d\n", dnum, pos, code[pos]);
    }

    printf("\n========================================================\n");
    printf(" Codeword after bit flip(s)\n");
    printf("========================================================\n");
    print_codeword_table(code, total);

    /* ---- Step 3 & 4: recompute each parity group -> build syndrome ---- */
    int numP = 0;
    while ((1 << numP) <= total) numP++;

    int *synd_bit = calloc(numP, sizeof(int));
    int syndrome = 0;

    printf("\n---------- Parity check (recomputed vs received) ----------\n");
    for (int k = 0; k < numP; k++) {
        int ppos = 1 << k;
        if (ppos > total) break;
        int parity = 0;
        for (int pos = 1; pos <= total; pos++)
            if (pos & ppos) parity ^= code[pos];   /* includes the parity bit itself */

        synd_bit[k] = parity;
        syndrome |= (parity << k);

        if (parity == 0) {
            printf("P%-3d (position %2d) : parity OK        -> bit value = %d\n",
                   ppos, ppos, code[ppos]);
        } else {
            printf("P%-3d (position %2d) : parity MISMATCH  -> contributes %d to syndrome\n",
                   ppos, ppos, ppos);
        }
    }

    printf("\nSyndrome bits (P%d..P1 order) = ", 1 << (numP - 1));
    for (int k = numP - 1; k >= 0; k--) printf("%d", synd_bit[k]);
    printf("   =>  syndrome (decimal) = %d\n", syndrome);

    /* ---- Step 5: locate, report and correct ---- */
    printf("\n---------------- Result ----------------\n");
    if (syndrome == 0) {
        printf("No error detected. All parity bits are OK - data accepted as is.\n");
    } else if (syndrome <= total) {
        int errpos = syndrome;
        if (is_parity_pos(errpos)) {
            printf("Error located at position %d -> that is parity bit P%d.\n", errpos, errpos);
        } else {
            /* find which D-number this position corresponds to */
            int dnum = 0;
            for (int i = 1; i <= d; i++) if (data_pos[i] == errpos) { dnum = i; break; }
            printf("Error located at position %d -> that is data bit D%d.\n", errpos, dnum);
        }
        printf("Correcting: flipping bit at position %d ...\n", errpos);
        code[errpos] ^= 1;
        printf("Position %d corrected -> new value = %d\n", errpos, code[errpos]);
    } else {
        printf("Syndrome (%d) exceeds codeword length (%d) - not a correctable\n", syndrome, total);
        printf("single-bit error (likely more than one bit was corrupted).\n");
    }

    printf("\n========================================================\n");
    printf(" Final codeword (after correction, if any)\n");
    printf("========================================================\n");
    print_codeword_table(code, total);

    printf("\nExtracted %d data bits: ", d);
    for (int i = 1; i <= d; i++) printf("%d", code[data_pos[i]]);
    printf("\n");

    free(code);
    free(data_pos);
    free(synd_bit);
    return 0;
}
