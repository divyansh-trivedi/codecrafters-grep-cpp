#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

// Recursive match function: returns true if pattern matches text starting at pos with captures state
bool match_recursive(const string& pattern, const string& text, size_t p_pos, size_t t_pos, unordered_map<int,pair<size_t,size_t>>& captures, int& group_num) {
    if (p_pos == pattern.size())
        return t_pos == text.size();

    // Handle end anchor
    if (pattern[p_pos] == '$' && p_pos+1 == pattern.size())
        return t_pos == text.size();

    // Handle start of group
    if (pattern[p_pos] == '(') {
        size_t close = p_pos;
        int depth = 0;
        while (close < pattern.size()) {
            if (pattern[close] == '(') depth++;
            else if (pattern[close] == ')') {
                depth--;
                if (depth == 0) break;
            }
            close++;
        }
        if (close == pattern.size()) return false; // unmatched
        int this_group = group_num++;
        for (size_t len = 0; t_pos + len <= text.size(); len++) {
            unordered_map<int,pair<size_t,size_t>> new_captures = captures;
            new_captures[this_group] = {t_pos, t_pos + len};
            if (match_recursive(pattern, text, p_pos + 1, t_pos, new_captures, group_num)) {
                if (match_recursive(pattern, text, close + 1, t_pos + len, new_captures, group_num))
                    return true;
            }
        }
        return false;
    }

    // Handle backreference
    if (pattern[p_pos] == '\\' && p_pos+1 < pattern.size() && isdigit(pattern[p_pos+1])) {
        int ref_idx = pattern[p_pos+1] - '0';
        if (!captures.count(ref_idx)) return false;
        auto [start, end] = captures[ref_idx];
        size_t cap_len = end - start;
        if (t_pos + cap_len > text.size()) return false;
        if (text.compare(t_pos, cap_len, text, start, cap_len) == 0) {
            return match_recursive(pattern, text, p_pos + 2, t_pos + cap_len, captures, group_num);
        } 
        return false;
    }

    // Handle literal, '.' and escapes \d \w
    if (t_pos >= text.size()) return false;
    if (pattern[p_pos] == '.') 
        return match_recursive(pattern, text, p_pos + 1, t_pos + 1, captures, group_num);

    if (p_pos+1 < pattern.size()) {
        if (pattern[p_pos] == '\\' && pattern[p_pos+1] == 'd' && isdigit(text[t_pos]))
            return match_recursive(pattern, text, p_pos + 2, t_pos + 1, captures, group_num);
        if (pattern[p_pos] == '\\' && pattern[p_pos+1] == 'w' && (isalnum(text[t_pos]) || text[t_pos]=='_'))
            return match_recursive(pattern, text, p_pos + 2, t_pos + 1, captures, group_num);
    }

    if (pattern[p_pos] == text[t_pos])
        return match_recursive(pattern, text, p_pos + 1, t_pos + 1, captures, group_num);

    return false;
}

bool match_pattern(const string& text, const string& pattern) {
    int group_num = 1; // at most 1 capture group per stage
    unordered_map<int,pair<size_t,size_t>> captures;
    return match_recursive(pattern, text, 0, 0, captures, group_num);
}

int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    if (string(argv[1]) != "-E") return 1;

    string input;
    getline(cin, input);
    string pattern = argv[2];
    
    return match_pattern(input, pattern) ? 0 : 1;
}
