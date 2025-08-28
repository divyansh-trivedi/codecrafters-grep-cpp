#include <iostream>
#include <string>
#include <vector>
#include <cctype>

// Find matching closing parenthesis
int find_closing_paren(const std::string &pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1; // invalid pattern
}

// Match a single character in a group
bool match_group(const std::string &input, const std::string &group) {
    return input.size() > 0 && group.find(input[0]) != std::string::npos;
}

// Try to match pattern and return number of characters consumed
int match_once(const std::string &input, const std::string &pattern);

// Match pattern zero or more (+ or ?) times
bool match_quantifier(const std::string &input, const std::string &pattern, char quant, const std::string &rest) {
    if (quant == '+') {
        int pos = 0;
        bool first = true;
        while (pos <= (int)input.size()) {
            int consumed = match_once(input.substr(pos), pattern);
            if (consumed <= 0) break;
            pos += consumed;
            first = false;
            if (match_once(input.substr(pos), rest) == (int)rest.size() || match_once(input.substr(pos), rest) != -1) return true;
        }
        return !first && match_once(input.substr(pos), rest) != -1;
    } else if (quant == '?') {
        return match_once(input, rest) != -1 || match_once(input, pattern + rest) != -1;
    }
    return false;
}

// Match a single occurrence of a pattern
int match_once(const std::string &input, const std::string &pattern) {
    if (pattern.empty()) return 0;
    if (pattern[0] == '$') return input.empty() ? 0 : -1;
    if (input.empty()) return -1;

    // Group with alternation
    if (pattern[0] == '(') {
        int close_paren = find_closing_paren(pattern, 0);
        std::string inside = pattern.substr(1, close_paren - 1);
        std::string remaining = pattern.substr(close_paren + 1);
        char quant = remaining.empty() ? 0 : remaining[0];
        if (quant == '+' || quant == '?') remaining = remaining.substr(1);

        // Split top-level alternation
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

        for (auto &opt : options) {
            if (quant) {
                if (match_quantifier(input, opt, quant, remaining)) return input.size(); // success
            } else {
                int consumed = match_once(input, opt);
                if (consumed >= 0) return consumed + (int)remaining.size(); // naive, could refine
            }
        }
        return -1;
    }

    // Quantifiers for single chars
    if (pattern.size() > 1 && (pattern[1] == '+' || pattern[1] == '?')) {
        return match_quantifier(input, std::string(1, pattern[0]), pattern[1], pattern.substr(2)) ? (int)input.size() : -1;
    }

    // Dot
    if (pattern[0] == '.') return 1;

    // \d
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\d") return isdigit(input[0]) ? 1 : -1;

    // \w
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\w") return isalnum(input[0]) ? 1 : -1;

    // []
    if (pattern[0] == '[') {
        auto close = pattern.find(']');
        bool neg = pattern[1] == '^';
        std::string chars = neg ? pattern.substr(2, close - 2) : pattern.substr(1, close - 1);
        bool match = match_group(input, chars);
        if ((neg && !match) || (!neg && match)) return 1;
        return -1;
    }

    // Exact char
    if (pattern[0] == input[0]) return 1;

    return -1;
}

// Wrapper function
bool match_patterns(const std::string &input, const std::string &pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        int consumed = match_once(input, pattern.substr(1));
        return consumed >= 0 && consumed == (int)input.size();
    }
    // Try match starting at each position
    for (size_t i = 0; i <= input.size(); i++) {
        int consumed = match_once(input.substr(i), pattern);
        if (consumed >= 0) return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc != 3) return 1;
    if (std::string(argv[1]) != "-E") return 1;

    std::string input;
    std::getline(std::cin, input);
    std::string pattern = argv[2];

    return match_patterns(input, pattern) ? 0 : 1;
}
