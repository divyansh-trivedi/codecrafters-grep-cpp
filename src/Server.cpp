#include <iostream>
#include <string>
#include <cctype>

using namespace std;

// Returns number of characters matched, -1 if no match
int match_pattern_len(const string& input, const string& pattern, bool start_of_line = true);

// Handle alternation (returns matched length)
int match_alternation_len(const string& input, const string& group) {
    size_t pipe = group.find('|');
    if (pipe == string::npos) {
        return match_pattern_len(input, group, true);
    }

    string left = group.substr(0, pipe);
    string right = group.substr(pipe + 1);

    int len = match_pattern_len(input, left, true);
    if (len >= 0) return len;

    return match_pattern_len(input, right, true);
}

int match_pattern_len(const string& input, const string& pattern, bool start_of_line) {
    if (pattern.empty()) return 0;
    if (pattern[0] == '$') return input.empty() ? 0 : -1;
    if (input.empty()) return -1;

    // ^ anchor
    if (pattern[0] == '^') return match_pattern_len(input, pattern.substr(1), true);

    // Group with alternation
    if (pattern[0] == '(') {
        size_t close = pattern.find(')');
        if (close == string::npos) return -1; // malformed pattern
        string group = pattern.substr(1, close - 1);
        string rest = pattern.substr(close + 1);

        int len = match_alternation_len(input, group);
        if (len < 0) return -1;
        int rest_len = match_pattern_len(input.substr(len), rest, false);
        if (rest_len < 0) return -1;
        return len + rest_len;
    }

    // \d
    if (pattern.size() >= 2 && pattern.substr(0,2) == "\\d") {
        if (isdigit(input[0])) return 1 + match_pattern_len(input.substr(1), pattern.substr(2), false);
        return -1;
    }

    // \w
    if (pattern.size() >= 2 && pattern.substr(0,2) == "\\w") {
        if (isalnum(input[0])) return 1 + match_pattern_len(input.substr(1), pattern.substr(2), false);
        return -1;
    }

    // .
    if (pattern[0] == '.') return 1 + match_pattern_len(input.substr(1), pattern.substr(1), false);

    // Literal
    if (input[0] == pattern[0]) return 1 + match_pattern_len(input.substr(1), pattern.substr(1), false);

    // If can start anywhere
    if (start_of_line) return -1;
    int skip_len = match_pattern_len(input.substr(1), pattern, false);
    if (skip_len < 0) return -1;
    return 1 + skip_len;
}

int main(int argc, char* argv[]) {
    cout << unitbuf;
    cerr << unitbuf;

    if (argc != 3) {
        cerr << "Expected two arguments\n";
        return 1;
    }

    string flag = argv[1];
    string pattern = argv[2];

    if (flag != "-E") {
        cerr << "Expected first argument to be '-E'\n";
        return 1;
    }

    string input_line;
    getline(cin, input_line);

    int matched_len = match_pattern_len(input_line, pattern, true);
    return matched_len >= 0 ? 0 : 1;
}
