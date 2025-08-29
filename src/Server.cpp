#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

bool match_recursive(const std::string &pattern, const std::string &text,
                     size_t p_pos, size_t t_pos,
                     std::unordered_map<int, std::string> captures, int group_num) {
    if (p_pos == pattern.size())
        return t_pos == text.size();

    if (pattern[p_pos] == '$' && p_pos + 1 == pattern.size())
        return t_pos == text.size();

    // Capturing group with quantifier support right after
    if (pattern[p_pos] == '(') {
        // Find closing ')'
        int depth = 1;
        size_t close = p_pos + 1;
        while (close < pattern.size() && depth > 0) {
            if (pattern[close] == '(')
                depth++;
            else if (pattern[close] == ')')
                depth--;
            close++;
        }
        if (depth != 0)
            return false;

        std::string group_content = pattern.substr(p_pos + 1, close - p_pos - 2);
        std::string after_group = pattern.substr(close);
        // Support quantifier after group
        char quant = 0;
        if (!after_group.empty() && (after_group[0] == '+' || after_group[0] == '?' || after_group[0] == '*')) {
            quant = after_group[0];
            after_group = after_group.substr(1);
        }

        // Greedy matching for '+' inside a group
        if (quant == '+') {
            for (size_t len = 1; t_pos + len <= text.size(); ++len) {
                auto captures_copy = captures;
                if (match_recursive(group_content, text.substr(t_pos, len), 0, 0, captures_copy, group_num + 1)) {
                    captures_copy[group_num] = text.substr(t_pos, len);
                    size_t next_pos = t_pos + len;
                    // Try more groups (greedy)
                    while (true) {
                        bool extended = false;
                        for (size_t sublen = 1; next_pos + sublen <= text.size(); ++sublen) {
                            auto captures_temp = captures_copy;
                            if (match_recursive(group_content, text.substr(next_pos, sublen), 0, 0, captures_temp, group_num + 1)) {
                                next_pos += sublen;
                                captures_copy = captures_temp;
                                extended = true;
                                break;
                            }
                        }
                        if (!extended)
                            break;
                    }
                    // Try after group
                    if (match_recursive(after_group, text, 0, next_pos, captures_copy, group_num + 1))
                        return true;
                }
            }
            return false;
        }
        // Zero or one group '?'
        if (quant == '?') {
            // Option 1: skip group
            if (match_recursive(after_group, text, 0, t_pos, captures, group_num))
                return true;
            // Option 2: match one group
            for (size_t len = 1; t_pos + len <= text.size(); ++len) {
                auto captures_copy = captures;
                if (match_recursive(group_content, text.substr(t_pos, len), 0, 0, captures_copy, group_num + 1)) {
                    captures_copy[group_num] = text.substr(t_pos, len);
                    if (match_recursive(after_group, text, 0, t_pos + len, captures_copy, group_num + 1))
                        return true;
                }
            }
            return false;
        }
        // Zero or more group '*'
        if (quant == '*') {
            if (match_recursive(after_group, text, 0, t_pos, captures, group_num))
                return true;
            for (size_t len = 1; t_pos + len <= text.size(); ++len) {
                auto captures_copy = captures;
                if (match_recursive(group_content, text.substr(t_pos, len), 0, 0, captures_copy, group_num + 1)) {
                    captures_copy[group_num] = text.substr(t_pos, len);
                    size_t next_pos = t_pos + len;
                    while (true) {
                        bool extended = false;
                        for (size_t sublen = 1; next_pos + sublen <= text.size(); ++sublen) {
                            auto captures_temp = captures_copy;
                            if (match_recursive(group_content, text.substr(next_pos, sublen), 0, 0, captures_temp, group_num + 1)) {
                                next_pos += sublen;
                                captures_copy = captures_temp;
                                extended = true;
                                break;
                            }
                        }
                        if (!extended)
                            break;
                    }
                    if (match_recursive(after_group, text, 0, next_pos, captures_copy, group_num + 1))
                        return true;
                }
            }
            return false;
        }
        // No quantifier - match group exactly once
        for (size_t len = 0; t_pos + len <= text.size(); ++len) {
            auto captures_copy = captures;
            if (match_recursive(group_content, text.substr(t_pos, len), 0, 0, captures_copy, group_num + 1)) {
                captures_copy[group_num] = text.substr(t_pos, len);
                if (match_recursive(after_group, text, 0, t_pos + len, captures_copy, group_num + 1))
                    return true;
            }
        }
        return false;
    }

    // Backreference
    if (pattern[p_pos] == '\\' && p_pos +1 < pattern.size() && isdigit(pattern[p_pos+1])) {
        int ref = pattern[p_pos+1] - '0';
        if (!captures.count(ref))
            return false;
        std::string cap = captures[ref];
        if (text.substr(t_pos, cap.size()) == cap)
            return match_recursive(pattern, text, p_pos + 2, t_pos + cap.size(), captures, group_num);
        return false;
    }

    // Literal, dot, escapes
    if (t_pos >= text.size())
        return false;
    if (pattern[p_pos] == '.') 
        return match_recursive(pattern, text, p_pos + 1, t_pos + 1, captures, group_num);
    if (p_pos+1 < pattern.size() && pattern[p_pos] == '\\' && pattern[p_pos+1] == 'd' && std::isdigit(text[t_pos]))
        return match_recursive(pattern, text, p_pos + 2, t_pos + 1, captures, group_num);
    if (p_pos+1 < pattern.size() && pattern[p_pos] == '\\' && pattern[p_pos+1] == 'w' && (std::isalnum(text[t_pos]) || text[t_pos]=='_'))
        return match_recursive(pattern, text, p_pos + 2, t_pos + 1, captures, group_num);
    if (pattern[p_pos] == text[t_pos])
        return match_recursive(pattern, text, p_pos + 1, t_pos + 1, captures, group_num);

    return false;
}

bool match_pattern(const std::string& input, const std::string& pattern) {
    std::unordered_map<int, std::string> captures;
    return match_recursive(pattern, input, 0, 0, captures, 1);
}

int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    if (std::string(argv[1]) != "-E") return 1;

    std::string input; getline(std::cin, input);
    std::string pattern = argv[2];
    return match_pattern(input, pattern) ? 0 : 1;
}
