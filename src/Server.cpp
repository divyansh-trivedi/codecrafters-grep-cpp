#include <iostream>
#include <string>
#include <cctype>

using namespace std;

// Forward declaration
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx);

// Match a single element (char, ., \d, \w, character class)
bool match_element(const string& pattern, int& p_idx, const string& text, int t_idx) {
    if (p_idx >= pattern.size() || t_idx >= text.size()) return false;

    char c = pattern[p_idx];

    // Escaped sequences
    if (c == '\\' && p_idx + 1 < pattern.size()) {
        char next = pattern[p_idx + 1];
        if (next == 'd' && isdigit(text[t_idx])) { p_idx += 2; return true; }
        if (next == 'w' && (isalnum(text[t_idx]) || text[t_idx] == '_')) { p_idx += 2; return true; }
        return false;
    }

    // Character class
    if (c == '[') {
        int end = pattern.find(']', p_idx);
        if (end == string::npos) return false;
        bool neg = (pattern[p_idx + 1] == '^');
        int start = p_idx + (neg ? 2 : 1);
        string group = pattern.substr(start, end - start);
        bool found = group.find(text[t_idx]) != string::npos;
        p_idx = end + 1;
        return neg ? !found : found;
    }

    // Normal char or .
    p_idx++;
    return (c == '.' || c == text[t_idx]);
}

// Match a group with optional quantifier
bool match_group(const string& pattern, int& p_idx, const string& text, int t_idx) {
    if (pattern[p_idx] != '(') return false;
    int close = pattern.find(')', p_idx);
    if (close == string::npos) return false;

    string group = pattern.substr(p_idx + 1, close - p_idx - 1);
    int next_idx = close + 1;

    // Check for quantifiers after group
    char quant = (next_idx < pattern.size()) ? pattern[next_idx] : 0;
    if (quant == '+' || quant == '?' || quant == '*') next_idx++;

    // Repeat matching for +/* quantifiers
    int start_t = t_idx;
    do {
        if (match_here(pattern, next_idx, text, t_idx)) return true;
        if (!match_here(group, 0, text, t_idx)) break;
        t_idx += 1;
    } while (quant == '+' || quant == '*');

    // ? quantifier: try skipping
    if (quant == '?') {
        if (match_here(pattern, next_idx, text, start_t)) return true;
    }

    // No quantifier, match once
    if (!quant) {
        if (match_here(group, 0, text, t_idx))
            return match_here(pattern, next_idx, text, t_idx + 1);
    }

    return false;
}

// Core matcher
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx) {
    if (p_idx == pattern.size()) return t_idx == text.size();

    // End anchor
    if (pattern[p_idx] == '$' && p_idx == pattern.size() - 1)
        return t_idx == text.size();

    // Group
    if (pattern[p_idx] == '(')
        return match_group(pattern, p_idx, text, t_idx);

    // Quantifiers for single element
    if (p_idx + 1 < pattern.size()) {
        char q = pattern[p_idx + 1];
        if (q == '+' || q == '?' || q == '*') {
            int start_p = p_idx;
            int start_t = t_idx;
            if (q == '+') {
                if (!match_element(pattern, p_idx, text, t_idx)) return false;
                t_idx++;
                while (t_idx <= text.size()) {
                    if (match_here(pattern, start_p + 2, text, t_idx)) return true;
                    if (!match_element(pattern, start_p, text, t_idx)) break;
                    t_idx++;
                }
            } else if (q == '?') {
                // skip
                if (match_here(pattern, start_p + 2, text, start_t)) return true;
                if (match_element(pattern, p_idx, text, t_idx))
                    return match_here(pattern, start_p + 2, text, t_idx + 1);
            } else if (q == '*') {
                int temp_t = t_idx;
                while (temp_t <= text.size()) {
                    if (match_here(pattern, start_p + 2, text, temp_t)) return true;
                    if (!match_element(pattern, start_p, text, temp_t)) break;
                    temp_t++;
                }
            }
            return false;
        }
    }

    // Normal match
    if (t_idx < text.size() && match_element(pattern, p_idx, text, t_idx))
        return match_here(pattern, p_idx, text, t_idx + 1);

    return false;
}

// Entry function
bool match_pattern(const string& text, const string& pattern) {
    if (pattern.empty()) return true;
    if (pattern[0] == '^') return match_here(pattern, 1, text, 0);

    for (int i = 0; i <= (int)text.size(); i++)
        if (match_here(pattern, 0, text, i)) return true;

    return false;
}

// Main
int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    string flag = argv[1], pattern = argv[2];
    if (flag != "-E") return 1;

    string text;
    getline(cin, text);

    return match_pattern(text, pattern) ? 0 : 1;
}
