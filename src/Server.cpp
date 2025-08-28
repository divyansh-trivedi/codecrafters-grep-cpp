#include <iostream>
#include <string>
#include <vector>
#include <cctype>

// Find matching closing parenthesis
int findClosingParen(const std::string &pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1;
}

// Split top-level alternation
std::vector<std::string> splitAlternation(const std::string &group) {
    std::vector<std::string> options;
    int depth = 0;
    size_t last = 0;
    for (size_t i = 0; i < group.size(); i++) {
        if (group[i] == '(') depth++;
        else if (group[i] == ')') depth--;
        else if (group[i] == '|' && depth == 0) {
            options.push_back(group.substr(last, i - last));
            last = i + 1;
        }
    }
    options.push_back(group.substr(last));
    return options;
}

// Match a single character (or class)
bool matchChar(char c, const std::string &pattern, size_t &consumed) {
    consumed = 0;
    if (pattern.empty()) return false;

    if (pattern[0] == '.') { consumed = 1; return true; }
    if (pattern.substr(0,2) == "\\d") { consumed = 1; return isdigit(c); }
    if (pattern.substr(0,2) == "\\w") { consumed = 1; return isalnum(c); }
    if (pattern[0] == '[') {
        auto close = pattern.find(']');
        if (close == std::string::npos) return false;
        bool neg = pattern[1] == '^';
        std::string chars = neg ? pattern.substr(2, close-2) : pattern.substr(1, close-1);
        bool match = chars.find(c) != std::string::npos;
        consumed = close + 1;
        return (neg && !match) || (!neg && match);
    }
    consumed = 1;
    return pattern[0] == c;
}

// Recursive matcher: returns true if input matches pattern
bool matchCore(const std::string &input, const std::string &pattern) {
    if (pattern.empty()) return true;
    if (!input.empty() && pattern[0] == '$') return false;
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    // Handle parenthesis group
    if (pattern[0] == '(') {
        int close = findClosingParen(pattern, 0);
        if (close == -1) return false;
        std::string group = pattern.substr(1, close-1);
        std::string rest = pattern.substr(close+1);

        char quant = (!rest.empty() && (rest[0] == '+' || rest[0] == '?')) ? rest[0] : 0;
        if (quant) rest = rest.substr(1);

        std::vector<std::string> options = splitAlternation(group);
        for (auto &opt : options) {
            for (size_t len = 1; len <= input.size(); len++) {
                std::string sub = input.substr(0,len);
                if (matchCore(sub, opt)) {
                    std::string remaining = input.substr(len);
                    if (quant == '+') {
                        size_t pos = 0;
                        while (pos < remaining.size()) {
                            bool matched = false;
                            for (size_t k = 1; k <= remaining.size()-pos; k++) {
                                if (matchCore(remaining.substr(pos,k), opt)) { pos += k; matched = true; break; }
                            }
                            if (!matched) break;
                        }
                        if (matchCore(input.substr(len+pos), rest)) return true;
                    } else if (quant == '?') {
                        if (matchCore(remaining, rest) || matchCore(input, rest)) return true;
                    } else {
                        if (matchCore(remaining, rest)) return true;
                    }
                }
            }
        }
        return false;
    }

    // Single character quantifiers
    if (pattern.size() > 1 && (pattern[1] == '+' || pattern[1] == '?')) {
        char quant = pattern[1];
        size_t consumed = 0;
        if (!matchChar(input[0], std::string(1, pattern[0]), consumed)) {
            if (quant == '?') return matchCore(input, pattern.substr(2));
            return false;
        }
        size_t pos = 1;
        if (quant == '+') {
            while (pos < input.size() && input[pos] == input[0]) pos++;
        }
        return matchCore(input.substr(pos), pattern.substr(2));
    }

    // Single character match
    size_t consumed = 0;
    if (matchChar(input[0], pattern, consumed)) return matchCore(input.substr(consumed), pattern.substr(consumed));
    return false;
}

// Entry point: supports ^ anchor
bool match(const std::string &input, const std::string &pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        return matchCore(input, pattern.substr(1)) && input.size() == input.size();
    }
    for (size_t i = 0; i <= input.size(); i++) {
        if (matchCore(input.substr(i), pattern)) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    if (std::string(argv[1]) != "-E") return 1;

    std::string input;
    std::getline(std::cin, input);
    std::string pattern = argv[2];

    return match(input, pattern) ? 0 : 1;
}
