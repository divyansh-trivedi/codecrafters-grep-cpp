#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

using namespace std;

// Find matching closing parenthesis index for '(' at start
int findClosingParen(const string &pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1;
}

// Split top-level alternations separated by un-nested '|'
vector<string> splitAlternation(const string &group) {
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

// Match a single input char c against pattern patt, set consumed length in patt
bool matchChar(char c, const string &patt, size_t &consumed) {
    consumed = 0;
    if (patt.empty()) return false;
    if (patt[0] == '.') {
        consumed = 1;
        return true;
    }
    if (patt.size() >= 2) {
        if (patt.substr(0, 2) == "\\d") {
            consumed = 2;
            return isdigit(c);
        }
        if (patt.substr(0, 2) == "\\w") {
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

// Recursive: returns length matched or -1 if no match
int matchCore(const string &input, const string &pattern) {
    if (pattern.empty()) return 0;
    if (pattern[0] == '$') return input.empty() ? 0 : -1;
    if (input.empty()) return -1;

    // Group '('
    if (pattern[0] == '(') {
        int close = findClosingParen(pattern, 0);
        if (close == -1) throw runtime_error("Unmatched '('");

        string group = pattern.substr(1, close - 1);
        string rest = close + 1 < pattern.size() ? pattern.substr(close + 1) : "";

        char quant = (!rest.empty() && (rest[0] == '+' || rest[0] == '?')) ? rest[0] : 0;
        if (quant) rest = rest.substr(1);

        vector<string> options = splitAlternation(group);

        // If quantifier '+'
        if (quant == '+') {
            // Try one or more repetitions of group
            for (const auto &opt : options) {
                // Try to match 1 or more groups greedily
                for (size_t len = 1; len <= input.size(); ++len) {
                    string sub = input.substr(0, len);
                    int firstMatch = matchCore(sub, opt);
                    if (firstMatch == -1 || (size_t)firstMatch != len) continue;

                    size_t pos = len;
                    while (true) {
                        int nextMatch = matchCore(input.substr(pos), opt);
                        if (nextMatch == -1) break;
                        pos += nextMatch;
                    }
                    int restMatch = matchCore(input.substr(pos), rest);
                    if (restMatch != -1) return pos + restMatch;
                }
            }
            return -1;
        }

        // If quantifier '?'
        else if (quant == '?') {
            // Try zero or one occurrence
            if (matchCore(input, rest) != -1) return matchCore(input, rest);
            for (const auto &opt : options) {
                for (size_t len = 1; len <= input.size(); ++len) {
                    string sub = input.substr(0, len);
                    int firstMatch = matchCore(sub, opt);
                    if (firstMatch == -1 || (size_t)firstMatch != len) continue;
                    int restMatch = matchCore(input.substr(len), rest);
                    if (restMatch != -1) return len + restMatch;
                }
            }
            return -1;
        }

        // No quantifier
        else {
            for (const auto &opt : options) {
                for (size_t len = 1; len <= input.size(); ++len) {
                    string sub = input.substr(0, len);
                    int firstMatch = matchCore(sub, opt);
                    if (firstMatch == -1 || (size_t)firstMatch != len) continue;
                    int restMatch = matchCore(input.substr(len), rest);
                    if (restMatch != -1) return len + restMatch;
                }
            }
            return -1;
        }
    }

    // Single char quantifiers
    if (pattern.size() > 1 && (pattern[1] == '?' || pattern[1] == '+')) {
        if (input.empty()) return -1;
        char quant = pattern[1];
        size_t consumed = 0;
        if (!matchChar(input[0], pattern.substr(0, 1), consumed)) {
            if (quant == '?') {
                return pattern.size() > 2 ? matchCore(input, pattern.substr(2)) : matchCore(input, "");
            }
            return -1;
        }
        size_t pos = consumed;
        if (quant == '+') {
            while (pos < input.size()) {
                size_t c = 0;
                if (!matchChar(input[pos], pattern.substr(0, 1), c)) break;
                pos += c;
            }
        }
        int restMatch = pattern.size() > 2 ? matchCore(input.substr(pos), pattern.substr(2)) : matchCore(input.substr(pos), "");
        if (restMatch == -1) return -1;
        return pos + restMatch;
    }

    // Match single character
    size_t consumed = 0;
    if (!matchChar(input[0], pattern, consumed)) return -1;
    int restMatch = pattern.size() >= consumed ? matchCore(input.substr(consumed), pattern.substr(consumed)) : matchCore(input.substr(consumed), "");
    if (restMatch == -1) return -1;
    return (int)consumed + restMatch;
}

bool match(const string &input, const string &pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        int m = matchCore(input, pattern.substr(1));
        return m == (int)input.size();
    }

    for (size_t i = 0; i <= input.size(); i++) {
        int m = matchCore(input.substr(i), pattern);
        if (m != -1) return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc != 3) return 1;
    if (string(argv[1]) != "-E") return 1;

    string input;
    getline(cin, input);
    string pattern = argv[2];

    return match(input, pattern) ? 0 : 1;
}
