int* maxSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    if (numsSize == 0 || k == 0) {
        *returnSize = 0;
        return NULL;
    }

    *returnSize = numsSize - k + 1;
    int* result = (int*)malloc((*returnSize) * sizeof(int));
    
    int* deque = (int*)malloc(numsSize * sizeof(int));
    int head = 0;
    int tail = 0;

    for (int i = 0; i < numsSize; i++) {
        if (head < tail && deque[head] < i - k + 1) {
            head++;
        }

        while (head < tail && nums[deque[tail - 1]] <= nums[i]) {
            tail--;
        }

        deque[tail++] = i;

        if (i >= k - 1) {
            result[i - k + 1] = nums[deque[head]];
        }
    }

    free(deque);
    
    return result;
}
