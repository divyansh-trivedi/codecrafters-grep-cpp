#include <iostream>
#include <string>
#include <cctype>

using namespace std;

bool match_pattern(const string& input, const string& pattern, bool start_of_line = true) {
    if (pattern.empty()) return true;
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    // Alternation (a|b)
    if (pattern[0] == '(' && pattern.find('|') != string::npos) {
        size_t pipe = pattern.find('|');
        size_t close = pattern.find(')');
        string left = pattern.substr(1, pipe - 1);
        string right = pattern.substr(pipe + 1, close - pipe - 1);
        return match_pattern(input, left, start_of_line) || match_pattern(input, right, start_of_line);
    }

    // Quantifiers
    if (pattern.size() > 1 && pattern[1] == '+') {
        size_t i = 0;
        while (i < input.size() && input[i] == pattern[0]) i++;
        return i > 0 && match_pattern(input.substr(i), pattern.substr(2), false);
    }

    if (pattern.size() > 1 && pattern[1] == '?') {
        if (input[0] == pattern[0])
            return match_pattern(input.substr(1), pattern.substr(2), false) || match_pattern(input, pattern.substr(2), false);
        else
            return match_pattern(input, pattern.substr(2), false);
    }

    // Wildcard
    if (pattern[0] == '.') return match_pattern(input.substr(1), pattern.substr(1), false);

    // Character classes
    if (pattern.substr(0, 2) == "\\d") {
        return isdigit(input[0]) ? match_pattern(input.substr(1), pattern.substr(2), false)
                                  : match_pattern(input.substr(1), pattern, false);
    }

    if (pattern.substr(0, 2) == "\\w") {
        return isalnum(input[0]) ? match_pattern(input.substr(1), pattern.substr(2), false)
                                  : match_pattern(input.substr(1), pattern, false);
    }

    if (pattern[0] == '[') {
        size_t close = pattern.find(']');
        bool neg = pattern[1] == '^';
        string chars = neg ? pattern.substr(2, close - 2) : pattern.substr(1, close - 1);
        bool match = chars.find(input[0]) != string::npos;
        if (neg) match = !match;
        if (match) return match_pattern(input.substr(1), pattern.substr(close + 1), false);
        return false;
    }

    // Literal match
    if (input[0] == pattern[0]) return match_pattern(input.substr(1), pattern.substr(1), false);

    // If pattern can start anywhere
    if (start_of_line) return false;
    return match_pattern(input.substr(1), pattern, false);
}

bool match_patterns(const string& input, const string& pattern) {
    if (!pattern.empty() && pattern[0] == '^') return match_pattern(input, pattern.substr(1), true);
    return match_pattern(input, pattern, false);
}

int main(int argc, char* argv[]) {
    cout << std::unitbuf;
    cerr << std::unitbuf;

    if (argc != 3) {
        cerr << "Expected two arguments" << endl;
        return 1;
    }

    string flag = argv[1];
    string pattern = argv[2];

    if (flag != "-E") {
        cerr << "Expected first argument to be '-E'" << endl;
        return 1;
    }

    string input_line;
    getline(cin, input_line);

    bool result = match_patterns(input_line, pattern);
    return result ? 0 : 1;
}
