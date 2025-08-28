#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>

using namespace std;

// Only one capturing group for this stage
unordered_map<int, string> captured_group;

// Forward declaration
int match_recursive(const string &pattern, const string &text, int &group_count);

// Main entry for matching
bool match_pattern(const string &input_line, const string &pattern) {
    if (pattern.empty()) return true;

    // Anchored at start
    if (pattern[0] == '^') {
        int group_count = 0;
        return match_recursive(pattern.substr(1), input_line, group_count) == (int)input_line.size();
    }

    // Unanchored
    for (size_t i = 0; i <= input_line.size(); ++i) {
        captured_group.clear();
        int group_count = 0;
        if (match_recursive(pattern, input_line.substr(i), group_count) != -1)
            return true;
    }
    return false;
}

// Recursive engine
int match_recursive(const string &pattern, const string &text, int &group_count) {
    if (pattern.empty()) return 0;
    if (pattern[0] == '$' && pattern.size() == 1) return text.empty() ? 0 : -1;

    // Handle alternation
    int depth = 0;
    for (int i = (int)pattern.size() - 1; i >= 0; --i) {
        if (pattern[i] == ')') depth++;
        else if (pattern[i] == '(') depth--;
        else if (pattern[i] == '|' && depth == 0) {
            string left = pattern.substr(0, i);
            string right = pattern.substr(i + 1);

            auto original_captures = captured_group;
            int original_group_count = group_count;

            int len = match_recursive(left, text, group_count);
            if (len != -1) return len;

            group_count = original_group_count;
            captured_group = original_captures;
            return match_recursive(right, text, group_count);
        }
    }

    // Determine atom
    size_t atom_len = 1;
    if (pattern[0] == '\\' && pattern.size() > 1) atom_len = 2;
    else if (pattern[0] == '[') {
        size_t end_bracket = pattern.find(']', 1);
        if (end_bracket != string::npos) atom_len = end_bracket + 1;
    } else if (pattern[0] == '(') {
        int level = 0;
        for (size_t i = 1; i < pattern.size(); ++i) {
            if (pattern[i] == '(') level++;
            else if (pattern[i] == ')') {
                if (level-- == 0) { atom_len = i + 1; break; }
            }
        }
    }

    char quantifier = (atom_len < pattern.size()) ? pattern[atom_len] : 0;
    string atom = pattern.substr(0, atom_len);
    string rest = pattern.substr(atom_len + ((quantifier == '?' || quantifier == '+') ? 1 : 0));

    // '?' quantifier
    if (quantifier == '?') {
        // Path A: skip atom
        int len = match_recursive(rest, text, group_count);
        if (len != -1) return len;

        // Path B: match atom
        int atom_len_matched = match_recursive(atom, text, group_count);
        if (atom_len_matched != -1) {
            int rest_len = match_recursive(rest, text.substr(atom_len_matched), group_count);
            if (rest_len != -1) return atom_len_matched + rest_len;
        }
        return -1;
    }

    // '+' quantifier
    if (quantifier == '+') {
        int first_match = match_recursive(atom, text, group_count);
        if (first_match == -1) return -1;

        string remaining = text.substr(first_match);
        int more_len = match_recursive(pattern.substr(0, atom_len + 1), remaining, group_count);
        if (more_len != -1) return first_match + more_len;

        int rest_len = match_recursive(rest, remaining, group_count);
        if (rest_len != -1) return first_match + rest_len;
        return -1;
    }

    // Capturing group
    if (pattern[0] == '(') {
        string group_content = pattern.substr(1, atom_len - 2);
        int current_capture_index = ++group_count;

        for (int len_try = (int)text.size(); len_try >= 0; --len_try) {
            int group_len = match_recursive(group_content, text.substr(0, len_try), group_count);
            if (group_len != -1) {
                captured_group[current_capture_index] = text.substr(0, group_len);
                int rest_len = match_recursive(rest, text.substr(group_len), group_count);
                if (rest_len != -1) return group_len + rest_len;
                captured_group.erase(current_capture_index);
            }
        }
        return -1;
    }

    if (text.empty()) return -1;

    // Backreference
    if (pattern[0] == '\\' && isdigit(pattern[1])) {
        int group_num = pattern[1] - '0';
        if (captured_group.count(group_num)) {
            string captured = captured_group[group_num];
            if (text.rfind(captured, 0) == 0) {
                int rest_len = match_recursive(rest, text.substr(captured.size()), group_count);
                if (rest_len != -1) return (int)captured.size() + rest_len;
            }
        }
        return -1;
    }

    // Escapes
    if (pattern[0] == '\\' && pattern[1] == 'd' && isdigit(text[0])) {
        int len = match_recursive(rest, text.substr(1), group_count);
        if (len != -1) return 1 + len;
    }
    if (pattern[0] == '\\' && pattern[1] == 'w' && (isalnum(text[0]) || text[0] == '_')) {
        int len = match_recursive(rest, text.substr(1), group_count);
        if (len != -1) return 1 + len;
    }

    // Character class
    if (pattern[0] == '[') {
        bool neg = pattern[1] == '^';
        string group = pattern.substr(neg ? 2 : 1, atom_len - (neg ? 2 : 1) - 1);
        bool found = group.find(text[0]) != string::npos;
        if (neg != found) {
            int len = match_recursive(rest, text.substr(1), group_count);
            if (len != -1) return 1 + len;
        }
        return -1;
    }

    // Literal match
    if (pattern[0] == '.' || pattern[0] == text[0]) {
        int len = match_recursive(rest, text.substr(1), group_count);
        if (len != -1) return 1 + len;
    }

    return -1;
}

int main(int argc, char *argv[]) {
    cout << unitbuf;
    cerr << unitbuf;

    if (argc != 3) { cerr << "Expected two arguments\n"; return 1; }
    string flag = argv[1], pattern = argv[2];
    if (flag != "-E") { cerr << "Expected first argument to be '-E'\n"; return 1; }

    string input_line;
    getline(cin, input_line);

    return match_pattern(input_line, pattern) ? 0 : 1;
}
