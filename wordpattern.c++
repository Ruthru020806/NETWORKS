class Solution {
public:
    bool wordPattern(string pattern, string s) {
        vector<string> words;
        string word;

        // Split the string into words
        stringstream ss(s);
        while (ss >> word) {
            words.push_back(word);
        }

        // Number of words must match pattern length
        if (words.size() != pattern.size())
            return false;

        unordered_map<char, string> charToWord;
        unordered_map<string, char> wordToChar;

        for (int i = 0; i < pattern.size(); i++) {
            char c = pattern[i];
            string w = words[i];

            // Existing mapping must match
            if (charToWord.count(c) && charToWord[c] != w)
                return false;

            if (wordToChar.count(w) && wordToChar[w] != c)
                return false;

            charToWord[c] = w;
            wordToChar[w] = c;
        }

        return true;
    }
};
