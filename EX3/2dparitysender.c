#include <stdio.h>
#include <stdlib.h>

#define COLS      7          /* number of data columns, as in the slides */
#define MAX_BITS  100000

int main(void) {
    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        printf("Error: could not open input.txt\n");
        return 1;
    }

    /* ---- Step 1 & 2: read file and convert every char to 8 bits ---- */
    char bits[MAX_BITS];
    int  nbits = 0;
    int  ch;

    while ((ch = fgetc(fp)) != EOF && nbits < MAX_BITS - 8) {
        for (int i = 7; i >= 0; i--)
            bits[nbits++] = ((ch >> i) & 1) ? '1' : '0';
    }
    fclose(fp);

    if (nbits == 0) {
        printf("Error: input.txt is empty\n");
        return 1;
    }

    /* ---- Step 3: pad with zeros so it fits a rectangular grid ---- */
    int rows  = (nbits + COLS - 1) / COLS;
    int total = rows * COLS;
    for (int i = nbits; i < total; i++) bits[i] = '0';

    int data[rows][COLS];
    int idx = 0;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < COLS; c++)
            data[r][c] = bits[idx++] - '0';

    /* ---- Step 4: row parity (even parity) ---- */
    int row_parity[rows];
    for (int r = 0; r < rows; r++) {
        int p = 0;
        for (int c = 0; c < COLS; c++) p ^= data[r][c];
        row_parity[r] = p;
    }

    /* ---- Step 5: column parity (even parity) ---- */
    int col_parity[COLS];
    for (int c = 0; c < COLS; c++) {
        int p = 0;
        for (int r = 0; r < rows; r++) p ^= data[r][c];
        col_parity[c] = p;
    }

    /* ---- Step 6: corner bit = parity of the row-parity column
                      (must equal parity of the col-parity row) ---- */
    int corner = 0;
    for (int r = 0; r < rows; r++) corner ^= row_parity[r];

    /* ---- Step 7a: display the matrix, same layout as the slides ---- */
    printf("========================================\n");
    printf(" SENDER : original text -> binary + 2D parity\n");
    printf("========================================\n");
    printf("Matrix size: %d rows x %d columns (+ parity row/col)\n\n", rows, COLS);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < COLS; c++) printf("%d ", data[r][c]);
        printf("| %d   <- row parity\n", row_parity[r]);
    }
    for (int c = 0; c < COLS; c++) printf("--");
    printf("----\n");
    for (int c = 0; c < COLS; c++) printf("%d ", col_parity[c]);
    printf("| %d   <- corner (parity of parities)\n", corner);
    printf("^\n|\ncolumn parities\n\n");

    /* ---- Step 7b: write everything the receiver needs ---- */
    FILE *out = fopen("transmission.dat", "w");
    if (!out) {
        printf("Error: could not create transmission.dat\n");
        return 1;
    }

    fprintf(out, "%d %d\n", rows, COLS);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < COLS; c++) fprintf(out, "%d ", data[r][c]);
        fprintf(out, "%d\n", row_parity[r]);
    }
    for (int c = 0; c < COLS; c++) fprintf(out, "%d ", col_parity[c]);
    fprintf(out, "%d\n", corner);
    fclose(out);

    printf("Sender: matrix with 2D parity written to transmission.dat\n");
    printf("Run receiver.c next to simulate bit errors and check parity.\n");
    return 0;
}
