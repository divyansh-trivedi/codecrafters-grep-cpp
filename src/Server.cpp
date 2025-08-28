#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

using namespace std;

// Find index of matching closing ')' for '(' at start
int findClosingParen(const string& pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1;
}

// Split top-level alternation '|'
vector<string> splitAlternation(const string& group) {
    vector<string> options;
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

// Match a single character, return number of chars consumed in pattern
bool matchChar(char c, const string& patt, size_t& consumed) {
    consumed = 0;
    if (patt.empty()) return false;
    if (patt[0] == '.') {
        consumed = 1;
        return true;
    }
    if (patt.size() >= 2) {
        if (patt.substr(0,2) == "\\d") {
            consumed = 2;
            return isdigit(c);
        }
        if (patt.substr(0,2) == "\\w") {
            consumed = 2;
            return isalnum(c) || c == '_';
        }
    }
    if (patt[0] == '[') {
        size_t close = patt.find(']');
        if (close == string::npos) return false;
        bool neg = patt[1] == '^';
        size_t start = neg ? 2 : 1;
        string chars = patt.substr(start, close - start);
        bool found = chars.find(c) != string::npos;
        consumed = close + 1;
        return neg ? !found : found;
    }
    consumed = 1;
    return patt[0] == c;
}

// Recursive match; returns length matched or -1 if no match
int matchCore(const string& input, const string& pattern);

int matchStar(const string& c, const string& pattern, const string& input) {
    // Match zero or more c's followed by pattern on input
    for (size_t i = 0; i <= input.size(); i++) {
        if (i > 0 && input.compare(i - c.size(), c.size(), c) != 0) break;
        int rest = matchCore(input.substr(i), pattern);
        if (rest != -1) return i + rest;
    }
    return -1;
}

int matchCore(const string& input, const string& pattern) {
    if (pattern.empty()) return 0;
    if (pattern[0] == '$') return input.empty() ? 0 : -1;
    if (input.empty() && pattern[0] != '$') return -1;

    // Handle alternation at top level
    int depth = 0;
    for (int i = (int)pattern.size() - 1; i >= 0; i--) {
        if (pattern[i] == ')') depth++;
        else if (pattern[i] == '(') depth--;
        else if (pattern[i] == '|' && depth == 0) {
            string left = pattern.substr(0, i);
            string right = pattern.substr(i + 1);
            int leftMatch = matchCore(input, left);
            if (leftMatch != -1) return leftMatch;
            return matchCore(input, right);
        }
    }

    // Handle group
    if (pattern[0] == '(') {
        int close = findClosingParen(pattern, 0);
        if (close == -1) throw runtime_error("Unmatched '('");
        string group = pattern.substr(1, close - 1);
        string rest = (close + 1 < pattern.size()) ? pattern.substr(close + 1) : "";

        // check quantifier after group
        char quant = !rest.empty() && (rest[0] == '*' || rest[0] == '+' || rest[0] == '?') ? rest[0] : 0;
        if (quant) rest = rest.substr(1);

        vector<string> options = splitAlternation(group);
        if (quant == '*') {
            // zero or more times group then rest
            if (matchCore(input, rest) != -1) return matchCore(input, rest);
            size_t pos = 0;
            while (pos < input.size()) {
                bool matched = false;
                for (const auto& alt : options) {
                    int len = matchCore(input.substr(pos), alt);
                    if (len != -1) {
                        pos += len;
                        matched = true;
                        break;
                    }
                }
                if (!matched) break;
                int rest_match = matchCore(input.substr(pos), rest);
                if (rest_match != -1) return pos + rest_match;
            }
            return -1;
        }
        if (quant == '+') {
            // one or more times
            for (const auto& alt : options) {
                int first_len = matchCore(input, alt);
                if (first_len == -1) continue;
                size_t pos = first_len;
                while (pos < input.size()) {
                    bool matched = false;
                    for (const auto& alt2 : options) {
                        int len2 = matchCore(input.substr(pos), alt2);
                        if (len2 != -1) {
                            pos += len2;
                            matched = true;
                            break;
                        }
                    }
                    if (!matched) break;
                    int rest_match = matchCore(input.substr(pos), rest);
                    if (rest_match != -1) return pos + rest_match;
                }
                int rest_match = matchCore(input.substr(pos), rest);
                if (rest_match != -1) return pos + rest_match;
            }
            return -1;
        }
        if (quant == '?') {
            // zero or one times
            int zero_match = matchCore(input, rest);
            if (zero_match != -1) return zero_match;
            for (const auto& alt : options) {
                int one_len = matchCore(input, alt);
                if (one_len == -1) continue;
                int rest_match = matchCore(input.substr(one_len), rest);
                if (rest_match != -1) return one_len + rest_match;
            }
            return -1;
        }
        // no quantifier - must match group then rest
        for (const auto& alt : options) {
            int one_len = matchCore(input, alt);
            if (one_len == -1) continue;
            int rest_match = matchCore(input.substr(one_len), rest);
            if (rest_match != -1) return one_len + rest_match;
        }
        return -1;
    }

    // Single char quantifiers '*', '+', '?'
    if (pattern.size() > 1 && (pattern[1] == '*' || pattern[1] == '+' || pattern[1] == '?')) {
        string c = pattern.substr(0,1);
        char quant = pattern[1];
        string rest = pattern.substr(2);

        if (quant == '*') return matchStar(c, rest, input);

        if (quant == '+') {
            int first_match = matchCore(input, c);
            if (first_match == -1) return -1;
            int all_match = matchStar(c, rest, input.substr(first_match));
            return all_match == -1 ? -1 : first_match + all_match;
        }

        if (quant == '?') {
            int skip_match = matchCore(input, rest);
            if (skip_match != -1) return skip_match;
            int one_match = matchCore(input, c);
            if (one_match == -1) return -1;
            int rest_match = matchCore(input.substr(one_match), rest);
            if (rest_match == -1) return -1;
            return one_match + rest_match;
        }
    }

    // Character classes
    if (!pattern.empty() && pattern[0] == '[') {
        size_t close = pattern.find(']');
        if (close == string::npos) return -1;
        bool neg = pattern[1] == '^';
        size_t start = neg ? 2 : 1;
        string chars = pattern.substr(start, close - start);
        if (input.empty()) return -1;
        bool found = chars.find(input[0]) != string::npos;
        if (neg ? !found : found) {
            int rest_match = matchCore(input.substr(1), pattern.substr(close+1));
            if (rest_match == -1) return -1;
            return 1 + rest_match;
        }
        return -1;
    }

    // Escapes
    if (pattern.size() >= 2 && pattern[0] == '\\') {
        if (input.empty()) return -1;
        if (pattern[1] == 'd' && isdigit(input[0])) {
            int rest_match = matchCore(input.substr(1), pattern.substr(2));
            if (rest_match == -1) return -1;
            return 1 + rest_match;
        }
        if (pattern[1] == 'w' && (isalnum(input[0]) || input[0] == '_')) {
            int rest_match = matchCore(input.substr(1), pattern.substr(2));
            if (rest_match == -1) return -1;
            return 1 + rest_match;
        }
        return -1;
    }

    // Literal or dot match
    if (!pattern.empty() && !input.empty() &&
        (pattern[0] == '.' || pattern[0] == input[0])) {
        int rest_match = matchCore(input.substr(1), pattern.substr(1));
        if (rest_match == -1) return -1;
        return 1 + rest_match;
    }

    return -1;
}

bool match_pattern(const string& input_line, const string& pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        int matched = matchCore(input_line, pattern.substr(1));
        return matched == (int)input_line.size();
    }
    for (size_t i = 0; i <= input_line.size(); i++) {
        int matched = matchCore(input_line.substr(i), pattern);
        if (matched != -1) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc != 3) {
        cerr << "Expected two arguments\n";
        return 1;
    }
    if (string(argv[1]) != "-E") {
        cerr << "Expected first argument to be '-E'\n";
        return 1;
    }
    string input_line;
    getline(cin, input_line);
    string pattern = argv[2];

    return match_pattern(input_line, pattern) ? 0 : 1;
}
