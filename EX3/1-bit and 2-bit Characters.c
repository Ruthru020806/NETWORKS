#include <stdbool.h>

bool isOneBitCharacter(int* bits, int bitsSize) {
    int i = 0;
    
    // Scan through the array up to the second-to-last element
    while (i < bitsSize - 1) {
        if (bits[i] == 1) {
            i += 2; // Jump 2 steps for a 2-bit character
        } else {
            i += 1; // Jump 1 step for a 1-bit character
        }
    }
    
    // If the pointer lands exactly on the last index, return true
    return i == bitsSize - 1;
}
