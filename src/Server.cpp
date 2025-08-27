#include <iostream>
#include <string>
#include <cctype>

using namespace std;

// Forward declaration
bool match_pattern(const string& input, const string& pattern, bool start_of_line = true);

// Helper to handle alternation in parentheses
bool match_alternation(const string& input, const string& group) {
    size_t pipe = group.find('|');
    if (pipe == string::npos) return match_pattern(input, group, true);

    string left = group.substr(0, pipe);
    string right = group.substr(pipe + 1);
    return match_pattern(input, left, true) || match_pattern(input, right, true);
}

// Main recursive matcher
bool match_pattern(const string& input, const string& pattern, bool start_of_line) {
    if (pattern.empty()) return true;
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    // ^ anchor
    if (!pattern.empty() && pattern[0] == '^') 
        return match_pattern(input, pattern.substr(1), true);

    // Handle groups with alternation ( ... | ... )
    if (pattern[0] == '(') {
        size_t close = pattern.find(')');
        string group = pattern.substr(1, close - 1);
        string rest = pattern.substr(close + 1);

        bool group_match = match_alternation(input, group);
        if (!group_match) return false;

        // Move input pointer past the matched group length
        // This is a simple approximation: consume group size characters
        return match_pattern(input.substr(group.size()), rest, false);
    }

    // \d for digits
    if (pattern.size() >= 2 && pattern.substr(0,2) == "\\d") {
        if (isdigit(input[0])) return match_pattern(input.substr(1), pattern.substr(2), false);
        return false;
    }

    // \w for alphanumeric
    if (pattern.size() >= 2 && pattern.substr(0,2) == "\\w") {
        if (isalnum(input[0])) return match_pattern(input.substr(1), pattern.substr(2), false);
        return false;
    }

    // Wildcard .
    if (pattern[0] == '.') return match_pattern(input.substr(1), pattern.substr(1), false);

    // Literal match
    if (input[0] == pattern[0]) return match_pattern(input.substr(1), pattern.substr(1), false);

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
