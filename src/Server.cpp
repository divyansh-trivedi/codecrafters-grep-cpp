#include <iostream>
#include <string>
#include <vector>
#include <cctype>

// Find the matching closing parenthesis for '(' at position start
int findClosingParen(const std::string &pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); ++i) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1;
}

// Split top-level alternation '|' in a group
std::vector<std::string> splitAlternation(const std::string &group) {
    std::vector<std::string> options;
    int depth = 0;
    size_t last = 0;
    for (size_t i = 0; i < group.size(); ++i) {
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

// Match a single character against pattern, return how many chars consumed
int matchChar(const std::string &input, const std::string &pattern) {
    if (input.empty() || pattern.empty()) return -1;

    if (pattern[0] == '.') return 1;
    if (pattern.substr(0,2) == "\\d") return isdigit(input[0]) ? 1 : -1;
    if (pattern.substr(0,2) == "\\w") return isalnum(input[0]) ? 1 : -1;
    if (pattern[0] == '[') {
        size_t close = pattern.find(']');
        if (close == std::string::npos) return -1;
        bool neg = pattern[1] == '^';
        std::string chars = neg ? pattern.substr(2, close-2) : pattern.substr(1, close-1);
        bool matched = chars.find(input[0]) != std::string::npos;
        return ((neg && !matched) || (!neg && matched)) ? (int)(close + 1) : -1;
    }
    return pattern[0] == input[0] ? 1 : -1;
}

// Core recursive matcher: returns number of characters matched, -1 if no match
int matchCore(const std::string &input, const std::string &pattern) {
    if (pattern.empty()) return 0;
    if (pattern[0] == '$') return input.empty() ? 0 : -1;
    if (input.empty()) return -1;

    // Handle parenthesis groups
    if (pattern[0] == '(') {
        int close = findClosingParen(pattern, 0);
        if (close == -1) return -1;
        std::string group = pattern.substr(1, close-1);
        std::string rest = pattern.substr(close+1);

        char quant = (!rest.empty() && (rest[0]=='+' || rest[0]=='?')) ? rest[0] : 0;
        if (quant) rest = rest.substr(1);

        std::vector<std::string> options = splitAlternation(group);
        for (auto &opt : options) {
            size_t consumed = 0;
            while (true) {
                int c = matchCore(input.substr(consumed), opt);
                if (c == -1) break;
                consumed += c;
                // If quantifier is '+', repeat the group
                if (quant == '+') continue;
                // Else check rest of pattern
                int restMatch = matchCore(input.substr(consumed), rest);
                if (restMatch != -1) return (int)consumed + restMatch;
                if (quant != '+') break;
            }
            // If quantifier is '?' we can skip this group
            if (quant == '?') {
                int restMatch = matchCore(input, rest);
                if (restMatch != -1) return restMatch;
            }
        }
        return -1;
    }

    // Single character quantifiers
    if (pattern.size() > 1 && (pattern[1] == '+' || pattern[1] == '?')) {
        char quant = pattern[1];
        int consumed = 0;
        int c = matchChar(input, std::string(1, pattern[0]));
        if (c == -1) {
            if (quant == '?') return matchCore(input, pattern.substr(2));
            return -1;
        }
        consumed += c;
        size_t pos = consumed;
        if (quant == '+') {
            while ((int)pos < (int)input.size() && input[pos] == input[0]) pos++;
            consumed = (int)pos;
        }
        int restMatch = matchCore(input.substr(consumed), pattern.substr(2));
        if (restMatch != -1) return consumed + restMatch;
        return -1;
    }

    // Single character match
    int c = matchChar(input, pattern);
    if (c == -1) return -1;
    int restMatch = matchCore(input.substr(c), pattern.substr(c));
    return restMatch == -1 ? -1 : c + restMatch;
}

// Entry point: returns true if entire input matches pattern
bool match(const std::string &input, const std::string &pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        int r = matchCore(input, pattern.substr(1));
        return r == (int)input.size();
    }
    for (size_t i = 0; i <= input.size(); i++) {
        int r = matchCore(input.substr(i), pattern);
        if (r != -1) return true;
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
