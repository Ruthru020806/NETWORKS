int totalHammingDistance(int* nums, int numsSize) {
    int total_distance = 0;
    
    // Loop through each bit position (0 to 30 is enough since nums[i] <= 10^9)
    for (int i = 0; i < 32; i++) {
        int count_ones = 0;
        
        // Count how many numbers have the i-th bit set
        for (int j = 0; j < numsSize; j++) {
            if ((nums[j] >> i) & 1) {
                count_ones++;
            }
        }
        
        // Number of zeros at the i-th bit position
        int count_zeros = numsSize - count_ones;
        
        // Total pairs with different bits at this position
        total_distance += count_ones * count_zeros;
    }
    
    return total_distance;
}
