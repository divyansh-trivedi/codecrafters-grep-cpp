#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

using namespace std;

// Find matching ')' for '(' at 'start'
int findClosingParen(const string& pat, int start) {
    int depth = 0;
    for (size_t i = start; i < pat.size(); i++) {
        if (pat[i] == '(') depth++;
        else if (pat[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1;
}

// Split top-level alternations separated by un-nested '|'
vector<string> splitAlternation(const string& group) {
    vector<string> res;
    int depth = 0;
    size_t last = 0;
    for (size_t i = 0; i < group.size(); i++) {
        if (group[i] == '(') depth++;
        else if (group[i] == ')') depth--;
        else if (group[i] == '|' && depth == 0) {
            res.push_back(group.substr(last, i - last));
            last = i + 1;
        }
    }
    res.push_back(group.substr(last));
    return res;
}

// Match a single input char c against pattern part patt, setting consumed chars in patt matched
bool matchChar(char c, const string& patt, size_t& consumed) {
    consumed = 0;
    if (patt.empty()) return false;
    if (patt[0] == '.') { consumed = 1; return true; }
    if (patt.size() >= 2 && patt.substr(0,2) == "\\d") { consumed = 2; return isdigit(c); }
    if (patt.size() >= 2 && patt.substr(0,2) == "\\w") { consumed = 2; return (isalnum(c) || c == '_'); }
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

// Recursive match returning number of characters matched or -1 if no match
int matchCore(const string& input, const string& pattern) {
    if (pattern.empty()) return 0;
    if (pattern[0] == '$') return input.empty() ? 0 : -1;
    if (input.empty()) return -1;

    // Group processing
    if (pattern[0] == '(') {
        int close = findClosingParen(pattern, 0);
        if (close == -1) throw runtime_error("Unmatched '('");
        string group = pattern.substr(1, close - 1);
        string rest = close + 1 < pattern.size() ? pattern.substr(close + 1) : "";

        char quant = (!rest.empty() && (rest[0] == '+' || rest[0] == '?')) ? rest[0] : 0;
        if (quant) rest = rest.substr(1);

        vector<string> options = splitAlternation(group);

        for (auto& opt : options) {
            size_t pos = 0;
            int firstMatch = matchCore(input.substr(pos), opt);
            if (firstMatch == -1) continue;

            int totalMatch = firstMatch;
            pos += firstMatch;

            if (quant == '+') {
                while (true) {
                    int nextMatch = matchCore(input.substr(pos), opt);
                    if (nextMatch == -1) break;
                    pos += nextMatch;
                    totalMatch += nextMatch;
                }
            } else if (quant == '?') {
                // zero or one, so next match optional
                // no loop needed
            }

            int restMatch = matchCore(input.substr(pos), rest);
            if (restMatch != -1) return totalMatch + restMatch;
        }
        return -1;
    }

    // Single char quantifiers
    if (pattern.size() > 1 && (pattern[1] == '?' || pattern[1] == '+')) {
        char quant = pattern[1];
        size_t consumed = 0;

        if (!matchChar(input[0], pattern.substr(0,1), consumed)) {
            if (quant == '?') {
                int restMatch = matchCore(input, pattern.size() > 2 ? pattern.substr(2) : "");
                return restMatch == -1 ? -1 : restMatch;
            }
            return -1;
        }

        size_t pos = consumed;
        int total = pos;
        if (quant == '+') {
            while (pos < (size_t)input.size()) {
                if (!matchChar(input[pos], pattern.substr(0,1), consumed)) break;
                pos += consumed;
                total = pos;
            }
        }
        int restMatch = matchCore(input.substr(pos), pattern.size() > 2 ? pattern.substr(2) : "");
        return restMatch == -1 ? -1 : total + restMatch;
    }

    // Single character matching
    size_t consumed = 0;
    if (!matchChar(input[0], pattern, consumed)) return -1;
    int restMatch = matchCore(input.substr(1), pattern.size() >= consumed ? pattern.substr(consumed) : "");
    return restMatch == -1 ? -1 : (int)1 + restMatch;
}

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
