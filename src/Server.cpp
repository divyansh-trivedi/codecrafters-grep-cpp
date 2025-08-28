#include <bits/stdc++.h>
using namespace std;

// Forward declaration
bool match_here(const string& pattern, int pi, const string& text, int ti);

// Match a group ( ... ) possibly with | alternatives
bool match_group(const string& group, const string& text, int ti, int &consumed) {
    // Split by | at top level
    int depth = 0;
    vector<string> parts;
    string cur;
    for (char c : group) {
        if (c == '(') depth++;
        else if (c == ')') depth--;
        if (c == '|' && depth == 0) {
            parts.push_back(cur);
            cur.clear();
        } else cur.push_back(c);
    }
    parts.push_back(cur);

    for (auto &alt : parts) {
        int tmp = ti;
        if ((int)alt.size() <= (int)text.size() - ti &&
            text.substr(ti, alt.size()) == alt) {
            consumed = alt.size();
            return true;
        }
    }
    return false;
}

bool match_here(const string& pattern, int pi, const string& text, int ti) {
    if (pi == (int)pattern.size()) return ti == (int)text.size();
    if (pattern[pi] == '$' && pi + 1 == (int)pattern.size()) return ti == (int)text.size();

    if (pattern[pi] == '(') {
        int depth = 1, j = pi + 1;
        while (j < (int)pattern.size() && depth > 0) {
            if (pattern[j] == '(') depth++;
            else if (pattern[j] == ')') depth--;
            j++;
        }
        string inside = pattern.substr(pi + 1, j - pi - 2);
        char quant = (j < (int)pattern.size() ? pattern[j] : '\0');

        if (quant == '+' || quant == '?') {
            int consumed = 0;
            if (!match_group(inside, text, ti, consumed)) {
                if (quant == '?') return match_here(pattern, j + 1, text, ti);
                return false;
            }
            if (quant == '+') {
                int pos = ti + consumed;
                while (true) {
                    int c2 = 0;
                    if (match_group(inside, text, pos, c2)) pos += c2;
                    else break;
                }
                return match_here(pattern, j + 1, text, pos);
            } else { // ?
                if (match_here(pattern, j + 1, text, ti + consumed)) return true;
                return match_here(pattern, j + 1, text, ti);
            }
        } else {
            int consumed = 0;
            if (!match_group(inside, text, ti, consumed)) return false;
            return match_here(pattern, j, text, ti + consumed);
        }
    }

    if (pi + 1 < (int)pattern.size() && (pattern[pi + 1] == '?' || pattern[pi + 1] == '+')) {
        char quant = pattern[pi + 1];
        if (quant == '?') {
            if (ti < (int)text.size() && pattern[pi] == text[ti])
                if (match_here(pattern, pi + 2, text, ti + 1)) return true;
            return match_here(pattern, pi + 2, text, ti);
        } else { // +
            int start = ti;
            if (ti >= (int)text.size() || pattern[pi] != text[ti]) return false;
            while (ti < (int)text.size() && pattern[pi] == text[ti]) ti++;
            return match_here(pattern, pi + 2, text, ti);
        }
    }

    if (ti < (int)text.size() && pattern[pi] == text[ti])
        return match_here(pattern, pi + 1, text, ti + 1);

    return false;
}

bool match(const string& pattern, const string& text) {
    if (!pattern.empty() && pattern[0] == '^') return match_here(pattern, 1, text, 0);
    for (int i = 0; i <= (int)text.size(); i++) if (match_here(pattern, 0, text, i)) return true;
    return false;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc != 3) return 1;
    string flag = argv[1], pattern = argv[2];
    if (flag != "-E") return 1;

    string input;
    getline(cin, input);

    return match(pattern, input) ? 0 : 1;
}
