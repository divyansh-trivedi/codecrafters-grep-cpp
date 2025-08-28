#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

using namespace std;

int findClosingParen(const string& pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1;
}

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

bool matchChar(char c, const string& patt, size_t& consumed) {
    consumed = 0;
    if (patt.empty()) return false;
    if (patt[0] == '.') { consumed = 1; return true; }
    if (patt.size() >= 2 && patt.substr(0, 2) == "\\d") { consumed = 2; return isdigit(c); }
    if (patt.size() >= 2 && patt.substr(0, 2) == "\\w") { consumed = 2; return isalnum(c) || c == '_'; }
    if (patt[0] == '[') {
        size_t closed = patt.find(']');
        if (closed == string::npos) return false;
        bool neg = patt[1] == '^';
        size_t start = neg ? 2 : 1;
        string chars = patt.substr(start, closed - start);
        bool found = chars.find(c) != string::npos;
        consumed = closed + 1;
        return neg ? !found : found;
    }
    consumed = 1;
    return patt[0] == c;
}

bool matchCore(const string& input, const string& pattern) {
    if (pattern.empty()) return true;
    if (!input.empty() && pattern[0] == '$') return false;
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    if (pattern[0] == '(') {
        int close = findClosingParen(pattern, 0);
        if (close == -1) return false;

        string group = pattern.substr(1, close - 1);
        string rest = close + 1 < pattern.size() ? pattern.substr(close + 1) : "";

        char quant = !rest.empty() && (rest[0] == '+' || rest[0] == '?') ? rest[0] : 0;
        if (quant) rest = rest.substr(1);

        vector<string> options = splitAlternation(group);

        for (const string& opt : options) {
            for (size_t len = 1; len <= input.size(); ++len) {
                string sub = input.substr(0, len);
                if (matchCore(sub, opt)) {
                    string remaining = input.size() >= len ? input.substr(len) : "";
                    if (quant == '+') {
                        size_t pos = 0;
                        while (pos < remaining.size()) {
                            bool matched = false;
                            for (size_t k = 1; k <= remaining.size() - pos; ++k) {
                                if (matchCore(remaining.substr(pos, k), opt)) {
                                    pos += k;
                                    matched = true;
                                    break;
                                }
                            }
                            if (!matched) break;
                        }
                        if (input.size() >= len + pos && matchCore(input.substr(len + pos), rest)) return true;
                    }
                    else if (quant == '?') {
                        if (matchCore(remaining, rest) || matchCore(input, rest)) return true;
                    } else {
                        if (matchCore(remaining, rest)) return true;
                    }
                }
            }
        }
        return false;
    }

    if (pattern.size() > 1 && (pattern[1] == '?' || pattern[1] == '+')) {
        if (input.empty()) return false; // input must have char to match
        char quant = pattern[1];
        size_t consumed = 0;
        if (!matchChar(input[0], pattern.substr(0, 1), consumed)) {
            if (quant == '?') return pattern.size() > 2 ? matchCore(input, pattern.substr(2)) : matchCore(input, "");
            return false;
        }
        size_t pos = 1;
        if (quant == '+') {
            while (pos < input.size() && input[pos] == input[0]) pos++;
        }
        return pattern.size() > 2 ? matchCore(input.substr(pos), pattern.substr(2)) : matchCore(input.substr(pos), "");
    }

    size_t consumed = 0;
    if (input.size() >= 1 && matchChar(input[0], pattern, consumed)) {
        return pattern.size() >= consumed ? matchCore(input.substr(1), pattern.substr(consumed)) : matchCore(input.substr(1), "");
    }
    return false;
}

bool match(const string& input, const string& pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        return matchCore(input, pattern.substr(1)) && input.size() == input.size();
    }
    for (size_t i = 0; i <= input.size(); ++i) {
        if (matchCore(input.substr(i), pattern)) return true;
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
