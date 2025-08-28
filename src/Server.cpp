#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;

// Forward declaration for the recursive helper
int match_recursive(const string& pattern, const string& text);

// Main entry point for the regex engine
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.empty()) return true;

    if (pattern[0] == '^') {
        // For an anchored pattern, the match must consume the entire line
        return match_recursive(pattern.substr(1), input_line) == (int)input_line.length();
    }

    // For unanchored patterns, loop and try to match at every position
    for (size_t i = 0; i <= input_line.length(); ++i) {
        if (match_recursive(pattern, input_line.substr(i)) != -1) {
            return true;
        }
    }
    return false;
}

// The core recursive engine. Returns match length or -1 on failure.
int match_recursive(const string& pattern, const string& text) {
    // Base Case: An empty pattern matches 0 characters of the text
    if (pattern.empty()) return 0;

    // Handle '$' anchor
    if (pattern[0] == '$' && pattern.length() == 1) {
        return text.empty() ? 0 : -1;
    }

    // Handle top-level alternation (lowest precedence)
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

    // Handle quantified groups like (...)+ and (...)?
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
            string group_pattern = pattern.substr(0, end_paren_pos + 1);
            string after_group = pattern.substr(end_paren_pos + 1);

            if (!after_group.empty()) {
                if (after_group[0] == '+') {
                    int len1 = match_recursive(group_pattern, text);
                    if (len1 == -1) return -1;
                    int len2 = match_recursive(pattern, text.substr(len1));
                    if (len2 != -1) return len1 + len2;
                    return len1;
                }
                if (after_group[0] == '?') {
                    int len1 = match_recursive(after_group.substr(1), text);
                    if (len1 != -1) return len1;
                    int len2 = match_recursive(group_pattern, text);
                    if (len2 != -1) {
                        int len3 = match_recursive(after_group.substr(1), text.substr(len2));
                        if (len3 != -1) return len2 + len3;
                    }
                    return -1;
                }
            }
             // It's a simple group, match its content
            return match_recursive(pattern.substr(1, end_paren_pos - 1) + after_group, text);
        }
    }

    // Handle quantifiers for single characters
    if (pattern.length() > 1 && (pattern[1] == '?' || pattern[1] == '+')) {
        if (!text.empty() && (pattern[0] == '.' || pattern[0] == text[0])) {
            if (pattern[1] == '?') {
                int len = match_recursive(pattern.substr(2), text.substr(1));
                if (len != -1) return 1 + len;
            }
            if (pattern[1] == '+') {
                 int len = match_recursive(pattern, text.substr(1));
                 if (len != -1) return 1 + len;
            }
        }
        if (pattern[1] == '?' || pattern[1] == '+') {
            return match_recursive(pattern.substr(2), text);
        }
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