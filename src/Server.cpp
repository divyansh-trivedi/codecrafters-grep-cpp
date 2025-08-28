#include <bits/stdc++.h>
using namespace std;

bool matchHere(const string &pattern, const string &text);

bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isSpace(char c) { return c == ' ' || c == '\t'; }
bool isWord(char c) { return isalnum(c) || c == '_'; }

bool matchChar(const string &p, int &i, char tc) {
    if (p[i] == '.') return true;
    if (p[i] == '\\') {
        i++;
        if (i >= (int)p.size()) return false;
        if (p[i] == 'd') return isDigit(tc);
        if (p[i] == 's') return isSpace(tc);
        if (p[i] == 'w') return isWord(tc);
        return p[i] == tc; // literal escape
    }
    return p[i] == tc;
}

bool matchHere(const string &pattern, const string &text) {
    if (pattern.empty()) return text.empty();
    if (pattern == "$") return text.empty();

    // Group handling
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

        // Split alternation
        vector<string> alts;
        {
            int d = 0; string cur;
            for (char c : inside) {
                if (c == '(') d++;
                if (c == ')') d--;
                if (c == '|' && d == 0) {
                    alts.push_back(cur);
                    cur.clear();
                } else cur.push_back(c);
            }
            alts.push_back(cur);
        }

        char quant = 0;
        if (!after.empty() && (after[0] == '?' || after[0] == '*' || after[0] == '+')) {
            quant = after[0];
            after = after.substr(1);
        }

        for (auto &alt : alts) {
            if (quant == '?') {
                // try with or without
                for (size_t k = 0; k <= text.size(); k++) {
                    if (matchHere(alt, text.substr(0, k)) && matchHere(after, text.substr(k)))
                        return true;
                }
                if (matchHere(after, text)) return true;
            } else if (quant == '*' || quant == '+') {
                int minRep = (quant == '+') ? 1 : 0;
                // greedy repetition with backtracking
                function<bool(int,int)> dfs = [&](int start, int rep) {
                    if (rep >= minRep && matchHere(after, text.substr(start))) return true;
                    for (int k = start + 1; k <= (int)text.size(); k++) {
                        if (matchHere(alt, text.substr(start, k - start))) {
                            if (dfs(k, rep + 1)) return true;
                        }
                    }
                    return false;
                };
                if (dfs(0, 0)) return true;
            } else {
                // no quantifier
                for (size_t k = 0; k <= text.size(); k++) {
                    if (matchHere(alt, text.substr(0, k)) && matchHere(after, text.substr(k)))
                        return true;
                }
            }
        }
        return false;
    }

    // ^ anchor
    if (pattern[0] == '^')
        return matchHere(pattern.substr(1), text);

    // Normal char
    if (!text.empty()) {
        int i = 0;
        if (matchChar(pattern, i, text[0])) {
            return matchHere(pattern.substr(i + 1), text.substr(1));
        }
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
