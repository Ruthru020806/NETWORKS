#include <stdio.h>
#include <string.h>

int countPalindromicSubsequence(char* s) {
    int first[26];
    int last[26];
    
    // Initialize tracking arrays
    for (int i = 0; i < 26; i++) {
        first[i] = -1;
        last[i] = -1;
    }
    
    // Record the first and last occurrence index of each character
    int n = 0;
    while (s[n] != '\0') {
        int idx = s[n] - 'a';
        if (first[idx] == -1) {
            first[idx] = n;
        }
        last[idx] = n;
        n++;
    }
    
    int total_unique_palindromes = 0;
    
    // Iterate through all 26 possible outer characters
    for (int i = 0; i < 26; i++) {
        if (first[i] != -1 && first[i] < last[i]) {
            int middle_seen[26] = {0};
            int unique_count = 0;
            
            // Count unique characters between the first and last occurrence
            for (int j = first[i] + 1; j < last[i]; j++) {
                int mid_idx = s[j] - 'a';
                if (!middle_seen[mid_idx]) {
                    middle_seen[mid_idx] = 1;
                    unique_count++;
                }
            }
            total_unique_palindromes += unique_count;
        }
    }
    
    return total_unique_palindromes;
}
