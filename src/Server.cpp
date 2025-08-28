#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

using namespace std;

// Find the matching closing parenthesis for '(' at 'start'
int findClosingParen(const std::string& pattern, int start) {
    int depth = 0;
    for (size_t i = start; i < pattern.size(); i++) {
        if (pattern[i] == '(') depth++;
        else if (pattern[i] == ')') depth--;
        if (depth == 0) return i;
    }
    return -1; // no match
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
            last = i+1;
        }
    }
    options.push_back(group.substr(last));
    return options;
}

// Match one character 'c' against pattern starting at 'patt'
// consumed is set to how many pattern chars were matched (1 for literal chars, or more for classes)
bool matchChar(char c, const string& patt, size_t& consumed) {
    consumed = 0;
    if (patt.empty()) return false;
    if (patt[0] == '.') { consumed = 1; return true; }
    if (patt.size() >= 2 && patt.substr(0,2) == "\\d") { consumed = 2; return isdigit(c); }
    if (patt.size() >= 2 && patt.substr(0,2) == "\\w") { consumed = 2; return isalnum(c) || c == '_'; }
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

// Recursive core matcher: returns true if input matches pattern completely
bool matchCore(const string& input, const string& pattern) {
    if (pattern.empty()) return true;
    if (!input.empty() && pattern[0] == '$') return false; // '$' only matches empty input at end
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    // Group handling with parentheses
    if (pattern[0] == '(') {
        int close = findClosingParen(pattern,0);
        if (close == -1) return false; // malformed pattern

        string group = pattern.substr(1, close - 1);
        string rest = pattern.substr(close+1);

        // quantifier after group? '?', '+', or none
        char quant = (!rest.empty() && (rest[0] == '+' || rest[0] == '?')) ? rest[0] : 0;
        if (quant) rest = rest.substr(1);

        vector<string> options = splitAlternation(group);

        for (const string& opt : options) {
            // Try matching opt against prefixes of input
            for (size_t len = 1; len <= input.size(); ++len) {
                string sub = input.substr(0,len);
                if (matchCore(sub, opt)) {
                    string remaining = input.substr(len);
                    if (quant == '+') {
                        // match one or more group repetitions greedily
                        size_t pos = 0;
                        while (pos < remaining.size()) {
                            bool matched = false;
                            for (size_t k=1; k <= remaining.size()-pos; ++k) {
                                if (matchCore(remaining.substr(pos,k), opt)) {
                                    pos += k;
                                    matched = true;
                                    break;
                                }
                            }
                            if (!matched) break;
                        }
                        if (matchCore(input.substr(len+pos), rest)) return true;
                    } else if (quant == '?') {
                        // zero or one occurrence
                        if (matchCore(remaining, rest) || matchCore(input, rest)) return true;
                    } else {
                        if (matchCore(remaining, rest)) return true;
                    }
                }
            }
        }
        return false;
    }

    // Single char with quantifiers '?' or '+'
    if (pattern.size() > 1 && (pattern[1] == '?' || pattern[1] == '+')) {
        char quant = pattern[1];
        size_t consumed = 0;
        if (!matchChar(input[0], pattern.substr(0,1), consumed)) {
            if (quant == '?') return matchCore(input, pattern.substr(2));
            return false;
        }
        size_t pos = 1;
        if (quant == '+') {
            while (pos < input.size() && input[pos] == input[0]) pos++;
        }
        return matchCore(input.substr(pos), pattern.substr(2));
    }

    // Single character match: consume exact pattern length for classes and escapes
    size_t consumed = 0;
    if (input.size() >= consumed && matchChar(input[0], pattern, consumed))
        return matchCore(input.substr(consumed), pattern.substr(consumed));
    return false;
}

// The top-level match function supporting optional '^' anchor
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
