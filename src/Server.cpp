#include <iostream>
#include <string>
#include <cctype>

using namespace std;

// Recursive matcher
bool match_pattern(const string& input, const string& pattern, bool start_of_line = true);

// Helper to handle groups with quantifiers
bool match_group_pattern(const string& input, const string& group, const string& rest_pattern) {
    size_t i = 0;
    while (true) {
        // Try to match the group at current position
        if (!match_pattern(input.substr(i), group, true)) break;

        // Move past the matched group
        size_t group_len = 0;
        while (group_len < group.size() && group[group_len] != '+'
               && group[group_len] != '?') group_len++;

        i += group_len;

        // If rest_pattern is empty, any repetition is fine
        if (rest_pattern.empty()) return true;

        // Check rest of pattern
        if (match_pattern(input.substr(i), rest_pattern, false)) return true;

        // If group does not have + quantifier, stop after first match
        if (group_len < group.size() && group[group_len] != '+') break;
    }
    return false;
}

// Main recursive matcher
bool match_pattern(const string& input, const string& pattern, bool start_of_line) {
    if (pattern.empty()) return true;
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    // ^ anchor
    if (!pattern.empty() && pattern[0] == '^') 
        return match_pattern(input, pattern.substr(1), true);

    // Handle groups ( ... )
    if (pattern[0] == '(') {
        size_t close = pattern.find(')');
        string group = pattern.substr(1, close - 1);
        string rest = pattern.substr(close + 1);

        // Check if group has quantifier + or ?
        if (!rest.empty() && rest[0] == '+') {
            return match_group_pattern(input, group, rest.substr(1));
        } else if (!rest.empty() && rest[0] == '?') {
            return match_group_pattern(input, group, rest.substr(1)) || match_pattern(input, rest.substr(1), false);
        } else {
            return match_pattern(input, group, true) && match_pattern(input.substr(group.size()), rest, false);
        }
    }

    // \d for digits
    if (pattern.size() >= 2 && pattern.substr(0,2) == "\\d") {
        if (!input.empty() && isdigit(input[0])) 
            return match_pattern(input.substr(1), pattern.substr(2), false);
        return false;
    }

    // \w for alphanumeric
    if (pattern.size() >= 2 && pattern.substr(0,2) == "\\w") {
        if (!input.empty() && isalnum(input[0]))
            return match_pattern(input.substr(1), pattern.substr(2), false);
        return false;
    }

    // Wildcard .
    if (pattern[0] == '.') return match_pattern(input.substr(1), pattern.substr(1), false);

    // Character classes [...]
    if (pattern[0] == '[') {
        size_t close = pattern.find(']');
        bool neg = pattern[1] == '^';
        string chars = neg ? pattern.substr(2, close-2) : pattern.substr(1, close-1);
        bool match = chars.find(input[0]) != string::npos;
        if (neg) match = !match;
        if (match) return match_pattern(input.substr(1), pattern.substr(close+1), false);
        return false;
    }

    // Literal match with quantifiers
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

    // Literal match
    if (input[0] == pattern[0]) return match_pattern(input.substr(1), pattern.substr(1), false);

    // If pattern can start anywhere
    if (start_of_line) return false;
    return match_pattern(input.substr(1), pattern, false);
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

    bool result = match_pattern(input_line, pattern);
    return result ? 0 : 1;
}
