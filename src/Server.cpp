#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>
#include <vector>

using namespace std;

// Forward declaration for the main recursive function
int match_recursive(const string& pattern, const string& text);

// The main entry point for the regex engine.
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.empty()) return true;

    if (pattern[0] == '^') {
        // For an anchored pattern, the match must consume the entire line.
        return match_recursive(pattern.substr(1), input_line) == (int)input_line.length();
    }

    // For unanchored patterns, loop and try to match at every position.
    for (size_t i = 0; i <= input_line.length(); ++i) {
        if (match_recursive(pattern, input_line.substr(i)) != -1) {
            return true;
        }
    }
    return false;
}

// The core recursive engine. Returns match length or -1 on failure.
int match_recursive(const string& pattern, const string& text) {
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
            int len = match_recursive(left, text);
            if (len != -1) return len;
            return match_recursive(right, text);
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
            char quantifier = after_group.empty() ? 0 : after_group[0];

            if (quantifier == '+') {
                // Must match the group content at least once.
                int len1 = match_recursive(group_content, text);
                if (len1 == -1) return -1;

                string remaining_text = text.substr(len1);
                
                // --- START OF THE CRITICAL FIX ---
                // Path A (Greedy): Match the quantified group `(...)+` again on the remaining text.
                // We use the substring of the original pattern that represents the quantified group.
                string quantified_group_pattern = pattern.substr(0, end_paren_pos + 2);
                int more_len = match_recursive(quantified_group_pattern, remaining_text);
                if (more_len != -1) return len1 + more_len;
                // --- END OF THE CRITICAL FIX ---

                // Path B (Backtrack): Match the rest of the pattern that comes after the `+`.
                int rest_len = match_recursive(after_group.substr(1), remaining_text);
                if (rest_len != -1) return len1 + rest_len;
                
                return -1;
            }

            if (quantifier == '?') {
                // Path A: Skip the group and match the rest.
                int len_skipped = match_recursive(after_group.substr(1), text);
                if (len_skipped != -1) return len_skipped;

                // Path B: Match the group once, then match the rest.
                int len_matched = match_recursive(group_content, text);
                if (len_matched != -1) {
                    int rest_len = match_recursive(after_group.substr(1), text.substr(len_matched));
                    if (rest_len != -1) return len_matched + rest_len;
                }
                return -1;
            }
            // If no quantifier, it's just a group: match its content then the rest.
            return match_recursive(group_content + after_group, text);
        }
    }

    // Handle quantifiers for single characters
    if (pattern.length() > 1 && (pattern[1] == '?' || pattern[1] == '+')) {
        string rest = pattern.substr(2);
        if (pattern[1] == '?') {
            int len = match_recursive(rest, text);
            if (len != -1) return len;
        }
        if (!text.empty() && (pattern[0] == '.' || pattern[0] == text[0])) {
            if (pattern[1] == '+') {
                int more_len = match_recursive(pattern, text.substr(1));
                if (more_len != -1) return 1 + more_len;
            }
            int rest_len = match_recursive(rest, text.substr(1));
            if (rest_len != -1) return 1 + rest_len;
        }
        return -1;
    }
    
    // Handle single "atom" matches
    if (text.empty()) return -1;

    if (pattern[0] == '[') {
        size_t end_bracket = pattern.find(']', 1);
        if (end_bracket != string::npos) {
            bool is_negated = pattern[1] == '^';
            string group = pattern.substr(is_negated ? 2 : 1, end_bracket - (is_negated ? 2 : 1));
            bool found = group.find(text[0]) != string::npos;
            if (is_negated != found) {
                int len = match_recursive(pattern.substr(end_bracket + 1), text.substr(1));
                if (len != -1) return 1 + len;
            }
        }
        return -1;
    }

    if (pattern[0] == '\\' && pattern.length() > 1) {
        if ((pattern[1] == 'd' && isdigit(text[0])) || (pattern[1] == 'w' && (isalnum(text[0]) || text[0] == '_'))) {
            int len = match_recursive(pattern.substr(2), text.substr(1));
            if (len != -1) return 1 + len;
        }
        return -1;
    }

    if (pattern[0] == '.' || pattern[0] == text[0]) {
        int len = match_recursive(pattern.substr(1), text.substr(1));
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