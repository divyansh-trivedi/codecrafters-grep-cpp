#include <bits/stdc++.h>
using namespace std;

bool matchHere(const string &pattern, const string &text);

bool matchChar(char pc, char tc) {
    return pc == '.' || pc == tc;
}

// Try to match an expression with * or + repetition
bool repeatMatch(const string &inside, const string &after, const string &text, bool atLeastOnce) {
    // Try matching the inside group multiple times
    if (atLeastOnce) {
        // must match once
        for (size_t i = 0; i <= text.size(); i++) {
            if (matchHere(inside, text.substr(0, i))) {
                string rest = text.substr(i);
                // option 1: match more
                if (repeatMatch(inside, after, rest, false)) return true;
                // option 2: stop and continue
                if (matchHere(after, rest)) return true;
            }
        }
        return false;
    } else {
        // zero or more
        // try consuming 0 chars
        if (matchHere(after, text)) return true;
        // try consuming more
        for (size_t i = 0; i <= text.size(); i++) {
            if (matchHere(inside, text.substr(0, i))) {
                if (repeatMatch(inside, after, text.substr(i), false)) return true;
            }
        }
        return false;
    }
}

// Main matcher
bool matchHere(const string &pattern, const string &text) {
    if (pattern.empty()) return text.empty();

    // Handle $
    if (pattern == "$") return text.empty();

    // Handle group ( ... )
    if (pattern[0] == '(') {
        int depth = 0, j = 0;
        for (; j < (int)pattern.size(); j++) {
            if (pattern[j] == '(') depth++;
            else if (pattern[j] == ')') {
                depth--;
                if (depth == 0) break;
            }
        }
        string inside = pattern.substr(1, j - 1);
        string after = (j + 1 < pattern.size() ? pattern.substr(j + 1) : "");

        // Handle alternation
        vector<string> alts;
        {
            int d = 0; string cur;
            for (char c : inside) {
                if (c == '(') d++;
                if (c == ')') d--;
                if (c == '|' && d == 0) {
                    alts.push_back(cur);
                    cur.clear();
                } else {
                    cur.push_back(c);
                }
            }
            alts.push_back(cur);
        }

        char quant = 0;
        if (!after.empty() && (after[0] == '?' || after[0] == '*' || after[0] == '+')) {
            quant = after[0];
            after = after.substr(1);
        }

        if (quant == '?') {
            // try with group or without
            for (auto &alt : alts) {
                for (size_t k = 0; k <= text.size(); k++) {
                    if (matchHere(alt, text.substr(0, k)) && matchHere(after, text.substr(k)))
                        return true;
                }
            }
            return matchHere(after, text);
        }
        else if (quant == '*') {
            for (auto &alt : alts) {
                if (repeatMatch(alt, after, text, false)) return true;
            }
            return false;
        }
        else if (quant == '+') {
            for (auto &alt : alts) {
                if (repeatMatch(alt, after, text, true)) return true;
            }
            return false;
        } else {
            // no quantifier, must match once
            for (auto &alt : alts) {
                for (size_t k = 0; k <= text.size(); k++) {
                    if (matchHere(alt, text.substr(0, k)) &&
                        matchHere(after, text.substr(k))) return true;
                }
            }
            return false;
        }
    }

    // Handle ^ anchor
    if (pattern[0] == '^') {
        return matchHere(pattern.substr(1), text);
    }

    // Match first char
    if (!text.empty() && matchChar(pattern[0], text[0])) {
        return matchHere(pattern.substr(1), text.substr(1));
    }

    return false;
}

bool matchRegex(const string &pattern, const string &text) {
    if (!pattern.empty() && pattern[0] == '^')
        return matchHere(pattern.substr(1), text);
    for (size_t i = 0; i <= text.size(); i++) {
        if (matchHere(pattern, text.substr(i))) return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string regex;
    if (argc > 2 && string(argv[1]) == "-E") {
        regex = argv[2];
    } else {
        cerr << "Usage: ./your_program.sh -E <pattern>\n";
        return 1;
    }

    string input, text;
    while (getline(cin, input)) text += input;

    bool ok = matchRegex(regex, text);
    return ok ? 0 : 1;
}
