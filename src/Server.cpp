#include <bits/stdc++.h>
using namespace std;

// Forward declarations
bool matchRegex(const string &pattern, const string &text);
bool matchHere(const string &pattern, const string &text);

// Utility: check if char is digit
bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// Split group contents by '|' at top level
vector<string> splitAlternatives(const string &s) {
    vector<string> parts;
    string cur;
    int depth = 0;
    for (char c : s) {
        if (c == '(') depth++;
        if (c == ')') depth--;
        if (c == '|' && depth == 0) {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    parts.push_back(cur);
    return parts;
}

// Match with anchors
bool matchRegex(const string &pattern, const string &text) {
    if (!pattern.empty() && pattern[0] == '^') {
        return matchHere(pattern.substr(1), text);
    }
    for (size_t i = 0; i <= text.size(); i++) {
        if (matchHere(pattern, text.substr(i))) return true;
    }
    return false;
}

// Recursive matcher
bool matchHere(const string &pattern, const string &text) {
    if (pattern.empty()) return true;
    if (pattern == "$") return text.empty();

    // Handle groups ( ... )
    if (pattern[0] == '(') {
        int depth = 0;
        size_t i = 0;
        for (; i < pattern.size(); i++) {
            if (pattern[i] == '(') depth++;
            else if (pattern[i] == ')') {
                depth--;
                if (depth == 0) break;
            }
        }
        if (depth != 0) return false; // unbalanced parentheses

        string inside = pattern.substr(1, i - 1);
        string after = (i + 1 < pattern.size()) ? pattern.substr(i + 1) : "";

        char quant = '\0';
        if (!after.empty() && (after[0] == '+' || after[0] == '?')) {
            quant = after[0];
            after = after.substr(1);
        }

        vector<string> alternatives = splitAlternatives(inside);

        if (quant == '+') {
            // must match at least once
            for (auto &alt : alternatives) {
                if (matchHere(alt + after, text)) return true;
                for (size_t k = 1; k <= text.size(); k++) {
                    if (matchHere(alt, text.substr(0, k)) &&
                        matchHere("(" + inside + ")+" + after, text.substr(k)))
                        return true;
                }
            }
            return false;
        } else if (quant == '?') {
            // 0 or 1 times
            // Try matching once
            for (auto &alt : alternatives) {
                for (size_t k = 0; k <= text.size(); k++) {
                    if (matchHere(alt, text.substr(0, k)) &&
                        matchHere(after, text.substr(k)))
                        return true;
                }
            }
            // Or skip
            return matchHere(after, text);
        } else {
            // Plain group
            for (auto &alt : alternatives) {
                for (size_t k = 0; k <= text.size(); k++) {
                    if (matchHere(alt, text.substr(0, k)) &&
                        matchHere(after, text.substr(k)))
                        return true;
                }
            }
            return false;
        }
    }

    // Handle escape \d
    if (pattern.size() >= 2 && pattern[0] == '\\' && pattern[1] == 'd') {
        if (!text.empty() && isDigit(text[0])) {
            return matchHere(pattern.substr(2), text.substr(1));
        }
        return false;
    }

    // Handle dot
    if (pattern[0] == '.') {
        if (!text.empty()) return matchHere(pattern.substr(1), text.substr(1));
        return false;
    }

    // Normal character
    if (!text.empty() && pattern[0] == text[0]) {
        return matchHere(pattern.substr(1), text.substr(1));
    }

    return false;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string pattern, input;
    if (argc >= 3 && string(argv[1]) == "-E") {
        pattern = argv[2];
    }
    getline(cin, input);

    bool matched = matchRegex(pattern, input);
    return matched ? 0 : 1;
}
