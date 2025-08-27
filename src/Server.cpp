#include <iostream>
#include <string>
#include <cctype>

using namespace std;

// Core recursive matcher
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx);

// Match a single element (char, ., \d, \w, character class)
int match_element(const string& pattern, int p_idx, const string& text, int t_idx) {
    if (t_idx >= text.size()) return 0;

    char c = pattern[p_idx];

    // Escaped sequences
    if (c == '\\' && p_idx + 1 < pattern.size()) {
        char next = pattern[p_idx + 1];
        if (next == 'd' && isdigit(text[t_idx])) return 1;
        if (next == 'w' && (isalnum(text[t_idx]) || text[t_idx] == '_')) return 1;
        return 0;
    }

    // Character class
    if (c == '[') {
        int end = pattern.find(']', p_idx);
        if (end == string::npos) return 0;
        bool neg = (pattern[p_idx + 1] == '^');
        int start = p_idx + (neg ? 2 : 1);
        string group = pattern.substr(start, end - start);
        bool found = group.find(text[t_idx]) != string::npos;
        return (neg ? !found : found) ? 1 : 0;
    }

    // Normal char or .
    return (c == '.' || c == text[t_idx]) ? 1 : 0;
}

// Match quantifier for single element
bool match_quant(const string& pattern, int p_idx, const string& text, int t_idx) {
    char q = pattern[p_idx + 1];
    int consumed = match_element(pattern, p_idx, text, t_idx);
    if (!consumed && q == '+') return false;

    int t = t_idx;

    if (q == '+') {
        // Must match at least once
        t += consumed;
        while (t <= text.size()) {
            if (match_here(pattern, p_idx + 2, text, t)) return true;
            int c = match_element(pattern, p_idx, text, t);
            if (!c) break;
            t += c;
        }
        return false;
    } else if (q == '?') {
        // Skip
        if (match_here(pattern, p_idx + 2, text, t)) return true;
        if (consumed) return match_here(pattern, p_idx + 2, text, t + consumed);
        return false;
    }
    return false;
}

// Core recursive matcher
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx) {
    if (p_idx == pattern.size()) return t_idx == text.size();
    if (pattern[p_idx] == '$' && p_idx == pattern.size() - 1) return t_idx == text.size();

    // Handle group + alternation
    if (pattern[p_idx] == '(') {
        int close = pattern.find(')', p_idx);
        if (close == string::npos) return false;
        string inside = pattern.substr(p_idx + 1, close - p_idx - 1);
        int start = 0;
        while (true) {
            int bar = inside.find('|', start);
            string option = inside.substr(start, (bar == string::npos ? string::npos : bar - start));
            for (int len = 1; len <= text.size() - t_idx; len++) {
                if (match_here(option, 0, text.substr(t_idx, len), 0) &&
                    match_here(pattern, close + 1, text, t_idx + len)) return true;
            }
            if (bar == string::npos) break;
            start = bar + 1;
        }
        return false;
    }

    // '+' or '?' quantifiers for single element
    if (p_idx + 1 < pattern.size() && (pattern[p_idx + 1] == '+' || pattern[p_idx + 1] == '?'))
        return match_quant(pattern, p_idx, text, t_idx);

    // Normal single element
    int consumed = match_element(pattern, p_idx, text, t_idx);
    if (consumed) return match_here(pattern, p_idx + (pattern[p_idx] == '\\' ? 2 : 1), text, t_idx + consumed);

    return false;
}

bool match_pattern(const string& text, const string& pattern) {
    if (pattern.empty()) return true;
    if (pattern[0] == '^') return match_here(pattern, 1, text, 0);

    for (int i = 0; i <= (int)text.size(); i++)
        if (match_here(pattern, 0, text, i)) return true;
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    string flag = argv[1], pattern = argv[2];
    if (flag != "-E") return 1;

    string text;
    getline(cin, text);

    return match_pattern(text, pattern) ? 0 : 1;
}
