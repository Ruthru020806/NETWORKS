int hammingDistance(int x, int y) {
    int xor_result = x ^ y;
    int distance = 0;
    
    // Clear the lowest set bit in each iteration
    while (xor_result > 0) {
        xor_result &= (xor_result - 1);
        distance++;
    }
    
    return distance;
}
