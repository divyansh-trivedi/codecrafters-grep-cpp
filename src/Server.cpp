#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>
#include <vector>
#include <unordered_map>

using namespace std;

// --- Global state for captured groups ---
// Note: In a more advanced engine, this would be part of a match state object.
unordered_map<int, string> captured_groups;

// Forward declaration for the main recursive function
int match_recursive(const string& pattern, const string& text, int& group_count);

// The main entry point for the regex engine.
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.empty()) return true;

    if (pattern[0] == '^') {
        int group_count = 0;
        return match_recursive(pattern.substr(1), input_line, group_count) == (int)input_line.length();
    }

    // For unanchored patterns, loop and try to match at every position.
    for (size_t i = 0; i <= input_line.length(); ++i) {
        captured_groups.clear(); // Reset captures for each new starting position
        int group_count = 0;
        if (match_recursive(pattern, input_line.substr(i), group_count) != -1) {
            return true;
        }
    }
    return false;
}

// The core recursive engine. Returns match length or -1 on failure.
int match_recursive(const string& pattern, const string& text, int& group_count) {
    if (pattern.empty()) return 0;

    if (pattern[0] == '$' && pattern.length() == 1) {
        return text.empty() ? 0 : -1;
    }

    // Handle top-level alternation (pattern like "cat|dog")
    int depth = 0;
    for (int i = pattern.length() - 1; i >= 0; --i) {
        if (pattern[i] == ')') depth++;
        else if (pattern[i] == '(') depth--;
        else if (pattern[i] == '|' && depth == 0) {
            string left = pattern.substr(0, i);
            string right = pattern.substr(i + 1);
            int len = match_recursive(left, text, group_count);
            if (len != -1) return len;
            // Reset group count for the other branch of alternation
            int right_group_count = 0;
            return match_recursive(right, text, right_group_count);
        }
    }

    if (pattern[0] == '(') {
        int level = 0;
        size_t end_paren_pos = string::npos;
        for (size_t i = 1; i < pattern.length(); ++i) {
            if (pattern[i] == '(') level++;
            else if (pattern[i] == ')') {
                if (level-- == 0) {
                    end_paren_pos = i;
                    break;
                }
            }
        }

        if (end_paren_pos != string::npos) {
            string group_content = pattern.substr(1, end_paren_pos - 1);
            string after_group = pattern.substr(end_paren_pos + 1);
            
            // This is a capturing group
            int current_capture_index = ++group_count;

            int group_len = match_recursive(group_content, text, group_count);
            if (group_len != -1) {
                // Capture the matched text
                captured_groups[current_capture_index] = text.substr(0, group_len);
                
                int rest_len = match_recursive(after_group, text.substr(group_len), group_count);
                if (rest_len != -1) {
                    return group_len + rest_len;
                }
            }
            // Backtrack: if the rest of the pattern failed, un-capture the group
            captured_groups.erase(current_capture_index);
            return -1;
        }
    }

    // Handle quantifiers for single characters
    if (pattern.length() > 1 && (pattern[1] == '?' || pattern[1] == '+')) {
        string rest = pattern.substr(2);
        if (pattern[1] == '?') {
            int len = match_recursive(rest, text, group_count);
            if (len != -1) return len;
        }
        if (!text.empty() && (pattern[0] == '.' || pattern[0] == text[0])) {
            if (pattern[1] == '+') {
                int more_len = match_recursive(pattern, text.substr(1), group_count);
                if (more_len != -1) return 1 + more_len;
            }
            int rest_len = match_recursive(rest, text.substr(1), group_count);
            if (rest_len != -1) return 1 + rest_len;
        }
        return -1;
    }
    
    // Handle single "atom" matches
    if (text.empty()) return -1;

    // --- START OF NEW CODE: Handle backreferences ---
    if (pattern[0] == '\\' && pattern.length() > 1 && isdigit(pattern[1])) {
        int group_num = pattern[1] - '0';
        if (captured_groups.count(group_num)) {
            string captured = captured_groups[group_num];
            if (text.rfind(captured, 0) == 0) { // C++20 text.starts_with(captured)
                int rest_len = match_recursive(pattern.substr(2), text.substr(captured.length()), group_count);
                if (rest_len != -1) {
                    return captured.length() + rest_len;
                }
            }
        }
        return -1;
    }
    // --- END OF NEW CODE ---

    if (pattern[0] == '[') {
        size_t end_bracket = pattern.find(']', 1);
        if (end_bracket != string::npos) {
            bool is_negated = pattern[1] == '^';
            string group = pattern.substr(is_negated ? 2 : 1, end_bracket - (is_negated ? 2 : 1));
            bool found = group.find(text[0]) != string::npos;
            if (is_negated != found) {
                int len = match_recursive(pattern.substr(end_bracket + 1), text.substr(1), group_count);
                if (len != -1) return 1 + len;
            }
        }
        return -1;
    }

    if (pattern[0] == '\\' && pattern.length() > 1) {
        if ((pattern[1] == 'd' && isdigit(text[0])) || (pattern[1] == 'w' && (isalnum(text[0]) || text[0] == '_'))) {
            int len = match_recursive(pattern.substr(2), text.substr(1), group_count);
            if (len != -1) return 1 + len;
        }
        return -1;
    }

    if (pattern[0] == '.' || pattern[0] == text[0]) {
        int len = match_recursive(pattern.substr(1), text.substr(1), group_count);
        if (len != -1) return 1 + len;
    }

    return -1;
}

int main(int argc, char* argv[]) {
    cout << unitbuf;
    cerr << unitbuf;
    if (argc != 3) {
        cerr << "Expected two arguments" << endl;
        return 1;
    }
    string flag = argv[1];
    string pattern = argv[2];
    if (flag != "-E") {
        cerr << "Expected first argument to be '-E'" << endl;
        return 1;
    }
    string input_line;
    getline(cin, input_line);
    if (match_pattern(input_line, pattern)) {
        return 0;
    } else {
        return 1;
    }
}