#include <iostream>
#include <string>
#include <cctype>
#include <vector>

// Match a single character in a group
bool match_group(const std::string& input, const std::string& group) {
    for (char c : input) {
        if (group.find(c) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Find matching closing parenthesis considering nested ()
int find_closing_paren(const std::string& pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1; // Should not happen for valid patterns
}

// Recursive matcher
bool match_pattern(const std::string& input, const std::string& pattern, bool start_of_line = true) {
    if (pattern.empty()) return true;
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    // Alternation handling with nested parentheses
    if (pattern[0] == '(') {
        int close_paren = find_closing_paren(pattern, 0);
        std::string inside = pattern.substr(1, close_paren - 1);

        // Split on top-level | only
        std::vector<std::string> options;
        int depth = 0;
        size_t last = 0;
        for (size_t i = 0; i < inside.size(); i++) {
            if (inside[i] == '(') depth++;
            else if (inside[i] == ')') depth--;
            else if (inside[i] == '|' && depth == 0) {
                options.push_back(inside.substr(last, i - last));
                last = i + 1;
            }
        }
        options.push_back(inside.substr(last));

        // Try each option with remaining pattern
        std::string remaining = pattern.substr(close_paren + 1);
        for (auto& opt : options) {
            if (match_pattern(input, opt + remaining, start_of_line)) return true;
        }
        return false;
    }

    // +
    if (pattern.size() > 1 && pattern[1] == '+') {
        char target = pattern[0];
        size_t i = 0;
        while (i < input.size() && input[i] == target) i++;
        return i > 0 && match_pattern(input.substr(i), pattern.substr(2), false);
    }

    // ?
    if (pattern.size() > 1 && pattern[1] == '?') {
        if (pattern[0] == input[0]) {
            return match_pattern(input.substr(1), pattern.substr(2), false) ||
                   match_pattern(input, pattern.substr(2), false);
        } else {
            return match_pattern(input, pattern.substr(2), false);
        }
    }

    // .
    if (pattern[0] == '.') {
        return match_pattern(input.substr(1), pattern.substr(1), false);
    }

    // \d
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\d") {
        if (isdigit(input[0])) return match_pattern(input.substr(1), pattern.substr(2), false);
        return match_pattern(input.substr(1), pattern, false);
    }

    // \w
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\w") {
        if (isalnum(input[0])) return match_pattern(input.substr(1), pattern.substr(2), false);
        return match_pattern(input.substr(1), pattern, false);
    }

    // []
    if (pattern[0] == '[') {
        auto close = pattern.find(']');
        bool neg = pattern[1] == '^';
        std::string chars = neg ? pattern.substr(2, close - 2) : pattern.substr(1, close - 1);
        bool match = match_group(input.substr(0,1), chars);
        if ((neg && !match) || (!neg && match)) {
            return match_pattern(input.substr(1), pattern.substr(close + 1), false);
        } else return false;
    }

    // Exact character
    if (pattern[0] == input[0]) return match_pattern(input.substr(1), pattern.substr(1), false);

    // If start of line, fail; else try next char
    if (start_of_line) return false;
    return match_pattern(input.substr(1), pattern, false);
}

// Wrapper for ^ anchor
bool match_patterns(const std::string& input, const std::string& pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        return match_pattern(input, pattern.substr(1), true);
    }
    return match_pattern(input, pattern, false);
}

int main(int argc, char* argv[]) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string flag = argv[1];
    std::string pattern = argv[2];

    if (flag != "-E") {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }

    std::string input;
    std::getline(std::cin, input);

    try {
        bool result = match_patterns(input, pattern);
        return result ? 0 : 1;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
