#include <bits/stdc++.h>
using namespace std;

bool matchRegex(const string& pattern, const string& text);

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// Forward recursive matcher
bool matchHere(const string& pat, int pi, const string& txt, int ti);

// Match a group ( ... ) possibly with | alternatives
bool matchGroup(const string& group, const string& txt, int ti, int &consumed) {
    int depth = 0;
    vector<string> parts;
    string cur;
    for (char c : group) {
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

    for (auto &alt : parts) {
        int tmp = ti;
        if (matchHere(alt, 0, txt, tmp)) {
            consumed = tmp - ti;
            return true;
        }
    }
    return false;
}

// Core recursive engine
bool matchHere(const string& pat, int pi, const string& txt, int ti) {
    if (pi == (int)pat.size()) return ti == (int)txt.size();
    if (pat[pi] == '$' && pi + 1 == (int)pat.size()) return ti == (int)txt.size();

    if (pat[pi] == '(') {
        int depth = 1, j = pi + 1;
        while (j < (int)pat.size() && depth > 0) {
            if (pat[j] == '(') depth++;
            else if (pat[j] == ')') depth--;
            j++;
        }
        string inside = pat.substr(pi + 1, j - pi - 2);
        char quant = (j < (int)pat.size() ? pat[j] : '\0');

        if (quant == '+' || quant == '?') {
            int consumed = 0;
            if (!matchGroup(inside, txt, ti, consumed)) {
                if (quant == '?') return matchHere(pat, j + 1, txt, ti);
                return false;
            }
            if (quant == '+') {
                int pos = ti + consumed;
                while (true) {
                    int c2 = 0;
                    if (matchGroup(inside, txt, pos, c2)) {
                        pos += c2;
                    } else break;
                }
                return matchHere(pat, j + 1, txt, pos);
            } else {
                if (matchHere(pat, j + 1, txt, ti + consumed)) return true;
                return matchHere(pat, j + 1, txt, ti);
            }
        } else {
            int consumed = 0;
            if (!matchGroup(inside, txt, ti, consumed)) return false;
            return matchHere(pat, j, txt, ti + consumed);
        }
    }

    if (pi + 1 < (int)pat.size() && (pat[pi + 1] == '?' || pat[pi + 1] == '+')) {
        char quant = pat[pi + 1];
        if (quant == '?') {
            if (ti < (int)txt.size() && 
                (pat[pi] == '.' || (pat[pi] == '\\' && pi + 1 < (int)pat.size() && pat[pi+1]=='d' && isDigit(txt[ti])) || 
                pat[pi] == txt[ti])) {
                if (matchHere(pat, pi + 2, txt, ti + 1)) return true;
            }
            return matchHere(pat, pi + 2, txt, ti);
        } else { // +
            int start = ti;
            if (ti < (int)txt.size() && 
                (pat[pi] == '.' || (pat[pi] == '\\' && pi + 1 < (int)pat.size() && pat[pi+1]=='d' && isDigit(txt[ti])) || 
                pat[pi] == txt[ti])) {
                ti++;
            } else return false;
            while (ti < (int)txt.size() && 
                   (pat[pi] == '.' || (pat[pi] == '\\' && pi + 1 < (int)pat.size() && pat[pi+1]=='d' && isDigit(txt[ti])) || 
                   pat[pi] == txt[ti])) {
                ti++;
            }
            return matchHere(pat, pi + 2, txt, ti);
        }
    }

    if (pat[pi] == '.') {
        if (ti < (int)txt.size()) return matchHere(pat, pi + 1, txt, ti + 1);
        return false;
    }
    if (pat[pi] == '\\' && pi + 1 < (int)pat.size() && pat[pi+1] == 'd') {
        if (ti < (int)txt.size() && isDigit(txt[ti])) return matchHere(pat, pi + 2, txt, ti + 1);
        return false;
    }
    if (ti < (int)txt.size() && pat[pi] == txt[ti]) {
        return matchHere(pat, pi + 1, txt, ti + 1);
    }

    return false;
}

bool matchRegex(const string& pattern, const string& text) {
    if (!pattern.empty() && pattern[0] == '^') {
        return matchHere(pattern, 1, text, 0);
    }
    for (int i = 0; i <= (int)text.size(); i++) {
        if (matchHere(pattern, 0, text, i)) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string flag, pattern;
    if (argc >= 3) {
        flag = argv[1];
        pattern = argv[2];
    }

    string input;
    getline(cin, input);

    bool ok = matchRegex(pattern, input);
    return ok ? 0 : 1;
}
