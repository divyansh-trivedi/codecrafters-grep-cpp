#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

bool match_recursive(const string& pattern, const string& text, size_t p_pos, size_t t_pos, unordered_map<int, string> captures, int group_num) {
    if (p_pos == pattern.size()) {
        return t_pos == text.size();
    }
    if (pattern[p_pos] == '$' && p_pos + 1 == pattern.size()) {
        return t_pos == text.size();
    }
    if (t_pos > text.size()) return false;

    // Group capturing
    if (pattern[p_pos] == '(') {
        int depth = 1;
        size_t end_pos = p_pos + 1;
        while (end_pos < pattern.size() && depth > 0) {
            if (pattern[end_pos] == '(') depth++;
            else if (pattern[end_pos] == ')') depth--;
            end_pos++;
        }
        if (depth != 0) return false; // unmatched

        string group_content = pattern.substr(p_pos + 1, end_pos - p_pos - 2);
        string after_group = pattern.substr(end_pos);

        // Support quantifier after group
        char quant = 0;
        if (!after_group.empty() && (after_group[0] == '+' || after_group[0] == '?' || after_group[0] == '*')) {
            quant = after_group[0];
            after_group = after_group.substr(1);
        }

        if (quant == 0) {
            for (size_t len = 0; t_pos + len <= text.size(); len++) {
                unordered_map<int, string> new_captures = captures;
                if (match_recursive(group_content, text, 0, t_pos, new_captures, group_num + 1)) {
                    new_captures[group_num] = text.substr(t_pos, len);
                    if (match_recursive(after_group, text, 0, t_pos + len, new_captures, group_num + 1)) {
                        return true;
                    }
                }
            }
            return false;
        }
        else if (quant == '+') {
            for (size_t len = 1; t_pos + len <= text.size(); len++) {
                unordered_map<int, string> new_captures = captures;
                if (match_recursive(group_content, text, 0, t_pos, new_captures, group_num + 1)) {
                    new_captures[group_num] = text.substr(t_pos, len);
                    size_t pos = t_pos + len;
                    while (true) {
                        bool more = false;
                        for (size_t sublen = 1; pos + sublen <= text.size(); sublen++) {
                            unordered_map<int, string> temp_captures = new_captures;
                            if (match_recursive(group_content, text, 0, pos, temp_captures, group_num + 1)) {
                                new_captures = temp_captures;
                                pos += sublen;
                                more = true;
                                break;
                            }
                        }
                        if (!more) break;
                    }
                    if (match_recursive(after_group, text, 0, pos, new_captures, group_num + 1)) {
                        return true;
                    }
                }
            }
            return false;
        }
        else if (quant == '?') {
            if (match_recursive(after_group, text, 0, t_pos, captures, group_num)) return true;
            for (size_t len = 1; t_pos + len <= text.size(); len++) {
                unordered_map<int, string> new_captures = captures;
                if (match_recursive(group_content, text, 0, t_pos, new_captures, group_num + 1)) {
                    new_captures[group_num] = text.substr(t_pos, len);
                    if (match_recursive(after_group, text, 0, t_pos + len, new_captures, group_num + 1)) {
                        return true;
                    }
                }
            }
            return false;
        }
        else if (quant == '*') {
            if (match_recursive(after_group, text, 0, t_pos, captures, group_num)) return true;
            for (size_t len = 1; t_pos + len <= text.size(); len++) {
                unordered_map<int, string> new_captures = captures;
                if (match_recursive(group_content, text, 0, t_pos, new_captures, group_num + 1)) {
                    new_captures[group_num] = text.substr(t_pos, len);
                    size_t pos = t_pos + len;
                    while (true) {
                        bool more = false;
                        for (size_t sublen = 1; pos + sublen <= text.size(); sublen++) {
                            unordered_map<int, string> temp_captures = new_captures;
                            if (match_recursive(group_content, text, 0, pos, temp_captures, group_num + 1)) {
                                new_captures = temp_captures;
                                pos += sublen;
                                more = true;
                                break;
                            }
                        }
                        if (!more) break;
                    }
                    if (match_recursive(after_group, text, 0, pos, new_captures, group_num + 1)) {
                        return true;
                    }
                }
            }
            return false;
        }
    }

    // Backreference
    if (pattern[p_pos] == '\\' && p_pos +1 < pattern.size() && isdigit(pattern[p_pos+1])) {
        int ref = pattern[p_pos+1] - '0';
        if (!captures.count(ref)) return false;
        string cap = captures[ref];
        if (text.compare(t_pos, cap.size(), cap) == 0) {
            return match_recursive(pattern, text, p_pos + 2, t_pos + cap.size(), captures, group_num);
        }
        return false;
    }

    // Match dot or literal char or escapes
    if (t_pos >= text.size()) return false;
    if (pattern[p_pos] == '.' ||
        (pattern[p_pos] == '\\' && p_pos + 1 < pattern.size() && pattern[p_pos+1] == 'd' && isdigit(text[t_pos])) ||
        (pattern[p_pos] == '\\' && p_pos + 1 < pattern.size() && pattern[p_pos+1] == 'w' && (isalnum(text[t_pos]) || text[t_pos] == '_')) ||
        pattern[p_pos] == text[t_pos]) {
        if (pattern[p_pos] == '\\')
            return match_recursive(pattern, text, p_pos + 2, t_pos + 1, captures, group_num);
        else
            return match_recursive(pattern, text, p_pos + 1, t_pos + 1, captures, group_num);
    }

    return false;
}

bool match_pattern(const string& input, const string& pattern) {
    unordered_map<int, string> captures;
    return match_recursive(pattern, input, 0, 0, captures, 1);
}

int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    if (string(argv[1]) != "-E") return 1;

    string input; getline(cin, input);
    string pattern = argv[2];
    
    return match_pattern(input, pattern) ? 0 : 1;
}
