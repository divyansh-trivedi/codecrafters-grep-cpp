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

// Split top-level alternation in a group
std::vector<std::string> split_alternation(const std::string &pattern) {
    std::vector<std::string> options;
    int depth = 0;
    size_t last = 0;
    for (size_t i = 0; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        else if (pattern[i] == '|' && depth == 0) {
            options.push_back(pattern.substr(last, i - last));
            last = i + 1;
        }
    }
    options.push_back(pattern.substr(last));
    return options;
}

// Match single character or character class
bool match_char(const char c, const std::string &pattern, size_t &consumed) {
    if (pattern.empty()) return false;
    if (pattern[0] == '.') { consumed = 1; return true; }
    if (pattern.substr(0,2) == "\\d") { consumed = 1; return isdigit(c); }
    if (pattern.substr(0,2) == "\\w") { consumed = 1; return isalnum(c); }
    if (pattern[0] == '[') {
        auto close = pattern.find(']');
        bool neg = pattern[1] == '^';
        std::string chars = neg ? pattern.substr(2, close - 2) : pattern.substr(1, close - 1);
        bool match = chars.find(c) != std::string::npos;
        consumed = close + 1;
        return (neg && !match) || (!neg && match);
    }
    consumed = 1;
    return pattern[0] == c;
}

// Recursive pattern matcher
bool match_pattern(const std::string &input, const std::string &pattern) {
    if (pattern.empty()) return true;
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    // Handle group with alternation
    if (pattern[0] == '(') {
        int close = find_closing_paren(pattern, 0);
        std::string group = pattern.substr(1, close - 1);
        std::string remaining = pattern.substr(close + 1);
        char quant = (!remaining.empty() && (remaining[0] == '+' || remaining[0] == '?')) ? remaining[0] : 0;
        if (quant) remaining = remaining.substr(1);

        std::vector<std::string> options = split_alternation(group);

        for (auto &opt : options) {
            // Try to match the option once
            for (size_t i = 1; i <= input.size(); i++) {
                if (match_pattern(input.substr(0,i), opt)) {
                    std::string rest = input.substr(i);
                    if (quant == '+') {
                        size_t pos = i;
                        while (pos <= input.size()) {
                            if (match_pattern(input.substr(pos), remaining)) return true;
                            bool matched = false;
                            for (size_t j = 1; j <= input.size()-pos; j++) {
                                if (match_pattern(input.substr(pos,pos+j-pos), opt)) { pos += j; matched = true; break; }
                            }
                            if (!matched) break;
                        }
                    } else if (quant == '?') {
                        if (match_pattern(rest, remaining) || match_pattern(input, remaining)) return true;
                    } else {
                        if (match_pattern(rest, remaining)) return true;
                    }
                }
            }
        }
        return false;
    }

    // Quantifiers for single character
    if (pattern.size() > 1 && (pattern[1] == '+' || pattern[1] == '?')) {
        char quant = pattern[1];
        size_t consumed = 0;
        if (match_char(input[0], std::string(1, pattern[0]), consumed)) {
            size_t pos = 1;
            if (quant == '+') {
                while (pos < input.size() && input[pos] == input[0]) pos++;
            }
            return match_pattern(input.substr(pos), pattern.substr(2));
        } else if (quant == '?') {
            return match_pattern(input, pattern.substr(2));
        } else {
            return false;
        }
    }

    // Single character match
    size_t consumed = 0;
    if (match_char(input[0], pattern, consumed)) return match_pattern(input.substr(consumed), pattern.substr(consumed));

    return false;
}

// Handle ^ anchor
bool match_patterns(const std::string &input, const std::string &pattern) {
    if (!pattern.empty() && pattern[0] == '^') return match_pattern(input, pattern.substr(1));
    for (size_t i = 0; i <= input.size(); i++) {
        if (match_pattern(input.substr(i), pattern)) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    if (std::string(argv[1]) != "-E") return 1;

    std::string input;
    std::getline(std::cin, input);
    std::string pattern = argv[2];

    return match_patterns(input, pattern) ? 0 : 1;
}

