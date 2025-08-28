#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

using namespace std;

// Find the index of the matching closing parenthesis for '(' at 'start'
int findClosingParen(const string& pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1; // no matching ')'
}

// Split top-level alternations separated by un-nested '|'
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

// Match a single input character 'c' against pattern 'patt', set consumed length of pattern matched
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

// Recursive function: returns length matched if input matches pattern, or -1 if no match
int matchCore(const string& input, const string& pattern) {
    if (pattern.empty()) return 0;
    if (pattern[0] == '$') return input.empty() ? 0 : -1;
    if (input.empty()) return -1;

    // Handle group with alternation
    if (pattern[0] == '(') {
        int close = findClosingParen(pattern, 0);
        if (close == -1) throw runtime_error("Unmatched '(' in pattern");
        string group = pattern.substr(1, close - 1);
        string rest = (close + 1 < pattern.size()) ? pattern.substr(close + 1) : "";

        char quant = (!rest.empty() && (rest[0] == '+' || rest[0] == '?')) ? rest[0] : 0;
        if (quant) rest = rest.substr(1);

        vector<string> options = splitAlternation(group);
        for (const string& alt : options) {
            int firstMatch = matchCore(input, alt);
            if (firstMatch == -1) continue;

            int totalMatch = firstMatch;
            int pos = firstMatch;

            if (quant == '+') {
                while (true) {
                    int nextMatch = matchCore(input.substr(pos), alt);
                    if (nextMatch == -1) break;
                    pos += nextMatch;
                    totalMatch += nextMatch;
                }
            }
            else if (quant == '?') {
                // zero or one match allowed, no loop needed
            }

            int restMatch = matchCore(input.substr(pos), rest);
            if (restMatch != -1) return totalMatch + restMatch;
        }
        return -1;
    }

    // Single character with quantifiers '?', '+'
    if (pattern.size() > 1 && (pattern[1] == '?' || pattern[1] == '+')) {
        char quant = pattern[1];
        if (input.empty()) return -1;
        size_t consumed = 0;
        if (!matchChar(input[0], pattern.substr(0,1), consumed)) {
            if (quant == '?') {
                int restMatch = pattern.size() > 2 ? matchCore(input, pattern.substr(2)) : matchCore(input, "");
                return restMatch;
            }
            return -1;
        }
        int total = consumed;
        int pos = consumed;
        if (quant == '+') {
            while ((size_t)pos < input.size()) {
                size_t c = 0;
                if (!matchChar(input[pos], pattern.substr(0,1), c)) break;
                pos += c;
                total += c;
            }
        }
        int restMatch = pattern.size() > 2 ? matchCore(input.substr(pos), pattern.substr(2)) : matchCore(input.substr(pos), "");
        if (restMatch == -1) return -1;
        return total + restMatch;
    }

    // Single character match
    size_t consumed = 0;
    if (!matchChar(input[0], pattern, consumed)) return -1;
    int restMatch = pattern.size() >= consumed ? matchCore(input.substr(1), pattern.substr(consumed)) : matchCore(input.substr(1), "");
    if (restMatch == -1) return -1;
    return 1 + restMatch;
}

// Top-level match with optional '^' anchor
bool match(const string& input, const string& pattern) {
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

int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    if (string(argv[1]) != "-E") return 1;

    string input;
    getline(cin, input);
    string pattern = argv[2];

    return match(input, pattern) ? 0 : 1;
}
