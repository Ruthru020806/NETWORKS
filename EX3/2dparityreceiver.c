#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *fp = fopen("transmission.dat", "r");
    if (!fp) {
        printf("Error: could not open transmission.dat (run sender.c first)\n");
        return 1;
    }

    int rows, cols;
    fscanf(fp, "%d %d", &rows, &cols);

    int data[rows][cols];
    int row_parity[rows];
    int col_parity[cols];
    int corner;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) fscanf(fp, "%d", &data[r][c]);
        fscanf(fp, "%d", &row_parity[r]);
    }
    for (int c = 0; c < cols; c++) fscanf(fp, "%d", &col_parity[c]);
    fscanf(fp, "%d", &corner);
    fclose(fp);

    printf("========================================\n");
    printf(" RECEIVER : data as received\n");
    printf("========================================\n");
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) printf("%d ", data[r][c]);
        printf("| %d\n", row_parity[r]);
    }
    for (int c = 0; c < cols; c++) printf("--");
    printf("----\n");
    for (int c = 0; c < cols; c++) printf("%d ", col_parity[c]);
    printf("| %d (corner)\n\n", corner);

    /* ---- Step 2: ask user which bits to flip ---- */
    int n;
    printf("Enter number of data bits to flip (simulate transmission error): ");
    if (scanf("%d", &n) != 1 || n < 0) n = 0;

    int flips_applied = 0;
    for (int i = 0; i < n; i++) {
        int r, c;
        printf("  Bit %d -> enter row and column (1-indexed, max %d %d): ",
               i + 1, rows, cols);
        if (scanf("%d %d", &r, &c) != 2) { i--; continue; }
        r--; c--;
        if (r < 0 || r >= rows || c < 0 || c >= cols) {
            printf("  Invalid position (%d,%d) - ignored.\n", r + 1, c + 1);
            continue;
        }
        data[r][c] ^= 1;
        flips_applied++;
        printf("  -> flipped bit at row %d, column %d\n", r + 1, c + 1);
    }

    /* ---- Step 3: recompute parity on the corrupted data ---- */
    int new_row_parity[rows];
    for (int r = 0; r < rows; r++) {
        int p = 0;
        for (int c = 0; c < cols; c++) p ^= data[r][c];
        new_row_parity[r] = p;
    }
    int new_col_parity[cols];
    for (int c = 0; c < cols; c++) {
        int p = 0;
        for (int r = 0; r < rows; r++) p ^= data[r][c];
        new_col_parity[c] = p;
    }
    int new_corner = 0;
    for (int r = 0; r < rows; r++) new_corner ^= new_row_parity[r];

    printf("\n========================================\n");
    printf(" Recomputed matrix after the bit flip(s)\n");
    printf("========================================\n");
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) printf("%d ", data[r][c]);
        printf("| %d\n", new_row_parity[r]);
    }
    for (int c = 0; c < cols; c++) printf("--");
    printf("----\n");
    for (int c = 0; c < cols; c++) printf("%d ", new_col_parity[c]);
    printf("| %d (corner)\n\n", new_corner);

    /* ---- Step 4: compare old parity (received) vs new (recomputed) ---- */
    int row_errors[rows], nre = 0;
    int col_errors[cols], nce = 0;

    for (int r = 0; r < rows; r++)
        if (row_parity[r] != new_row_parity[r]) row_errors[nre++] = r;

    for (int c = 0; c < cols; c++)
        if (col_parity[c] != new_col_parity[c]) col_errors[nce++] = c;

    printf("---------- Parity comparison report ----------\n");

    if (nre == 0 && nce == 0) {
        if (flips_applied == 0) {
            printf("No parity mismatch found.\n");
            printf("No bit was flipped - data arrived exactly as sent.\n");
        } else {
            printf("No parity mismatch found - but %d bit(s) WERE flipped!\n", flips_applied);
            printf("This is an undetectable error pattern: the flipped bits form\n");
            printf("a rectangle (an even number of flips in every affected row\n");
            printf("AND every affected column), so each parity bit cancels back\n");
            printf("to its original value. 2D parity is blind to this case -\n");
            printf("the data is silently corrupted and the receiver has no way\n");
            printf("to know it.\n");
        }
    } else {
        if (nre > 0) {
            printf("Row parity bit(s) affected    : ");
            for (int i = 0; i < nre; i++) printf("row %d  ", row_errors[i] + 1);
            printf("\n");
        } else {
            printf("Row parity bit(s) affected    : none\n");
        }

        if (nce > 0) {
            printf("Column parity bit(s) affected : ");
            for (int i = 0; i < nce; i++) printf("col %d  ", col_errors[i] + 1);
            printf("\n");
        } else {
            printf("Column parity bit(s) affected : none\n");
        }

        printf("\n");
        if (nre == 1 && nce == 1) {
            printf("=> Single-bit error. It can be exactly located AND corrected\n");
            printf("   at row %d, column %d.\n", row_errors[0] + 1, col_errors[0] + 1);
        } else if (nre == 0 || nce == 0) {
            printf("=> The errors cancelled out in one dimension (e.g. two flips\n");
            printf("   in the same row/column). Detected, but position cannot be\n");
            printf("   uniquely pinpointed.\n");
        } else {
            printf("=> Multiple bit errors detected (%d row / %d column parities\n",
                   nre, nce);
            printf("   changed). 2D parity can flag this but cannot always locate\n");
            printf("   every bit exactly (see 'four errors cannot be detected'\n");
            printf("   case when the pattern forms a rectangle).\n");
        }
    }

    return 0;
}
