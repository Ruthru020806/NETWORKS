class Solution:
    def hasAlternatingBits(self, n: int) -> bool:
        x = n ^ (n >> 1)          # positions where the bits differ
        return (x & (x + 1)) == 0
