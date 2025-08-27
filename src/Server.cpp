#include <iostream>
#include <string>
#include <cctype>
#include <vector>

using namespace std;

// Forward declaration
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx);

// Match single character or group element
bool match_element(const string& pattern, int& p_idx, const string& text, int t_idx) {
    if (p_idx >= pattern.size() || t_idx >= text.size()) return false;

    char c = pattern[p_idx];

    // Escaped classes \d, \w
    if (c == '\\' && p_idx + 1 < pattern.size()) {
        char next = pattern[p_idx + 1];
        if (next == 'd' && isdigit(text[t_idx])) { p_idx += 2; return true; }
        if (next == 'w' && (isalnum(text[t_idx]) || text[t_idx] == '_')) { p_idx += 2; return true; }
        return false;
    }

    // Character class [ ... ]
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

    // Single char or .
    p_idx++;
    return (c == '.' || c == text[t_idx]);
}

// Recursive quantifier matcher
bool match_quantifier(const string& pattern, int& p_idx, const string& text, int t_idx, char quant = '+') {
    int start_idx = t_idx;
    int prev_p = p_idx;

    if (quant == '+') {
        // Must match at least once
        if (!match_here(pattern, prev_p, text, t_idx)) return false;
        t_idx++;
    }

    while (t_idx <= text.size()) {
        if (match_here(pattern, p_idx, text, t_idx)) return true;
        if (t_idx >= text.size()) break;
        t_idx++;
    }

    return false;
}

// Core recursive matcher
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx) {
    if (p_idx == pattern.size()) return t_idx == text.size();

    // Anchors
    if (pattern[p_idx] == '^') return match_here(pattern, p_idx + 1, text, t_idx == 0 ? 0 : t_idx);

    if (pattern[p_idx] == '$' && p_idx == pattern.size() - 1) return t_idx == text.size();

    // Grouping with alternation
    if (pattern[p_idx] == '(') {
        int close = pattern.find(')', p_idx);
        if (close == string::npos) return false;

        string inside = pattern.substr(p_idx + 1, close - p_idx - 1);

        // Split by |
        vector<string> options;
        size_t start = 0;
        while (true) {
            size_t bar = inside.find('|', start);
            options.push_back(inside.substr(start, bar == string::npos ? string::npos : bar - start));
            if (bar == string::npos) break;
            start = bar + 1;
        }

        for (auto& opt : options) {
            if (match_here(opt, 0, text, t_idx) && match_here(pattern, close + 1, text, t_idx + opt.size()))
                return true;
        }
        return false;
    }

    // Quantifiers: +, *, ?
    if (p_idx + 1 < pattern.size()) {
        char q = pattern[p_idx + 1];
        if (q == '+') {
            int saved = p_idx;
            return match_quantifier(pattern, p_idx, text, t_idx, '+');
        }
        if (q == '?') {
            int saved = p_idx;
            // Skip
            if (match_here(pattern, p_idx + 2, text, t_idx)) return true;
            // Take
            if (t_idx < text.size() && match_element(pattern, p_idx, text, t_idx))
                return match_here(pattern, p_idx, text, t_idx + 1);
            return false;
        }
        if (q == '*') {
            int saved = p_idx;
            while (t_idx <= text.size()) {
                if (match_here(pattern, p_idx + 2, text, t_idx)) return true;
                if (t_idx >= text.size()) break;
                if (!match_element(pattern, p_idx, text, t_idx)) break;
                t_idx++;
            }
            p_idx += 2;
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
int main() {
    string text;
    string pattern;
    getline(cin, text);
    getline(cin, pattern);

    if (match_pattern(text, pattern)) return 0;
    else return 1;
}
