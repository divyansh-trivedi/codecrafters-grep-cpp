#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;

// Forward declaration for the recursive helper function
bool match_recursive(const string& pattern, const string& text);

// The main entry point for the regex engine.
// Handles the ^ anchor and the unanchored search loop.
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern[0] == '^') {
        return match_recursive(pattern.substr(1), input_line);
    }

    for (size_t i = 0; i <= input_line.length(); ++i) {
        if (match_recursive(pattern, input_line.substr(i))) {
            return true;
        }
    }
    return false;
}

// This is the core recursive engine.
bool match_recursive(const string& pattern, const string& text) {
    // Base Cases
    if (pattern.empty()) return true;
    if (pattern[0] == '$' && pattern.length() == 1) return text.empty();

    // Handle quantifiers `?` and `+` on the next character/group
    if (pattern.length() > 1) {
        if (pattern[1] == '?') {
            return match_recursive(pattern.substr(2), text) || 
                   (!text.empty() && (pattern[0] == '.' || pattern[0] == text[0]) && match_recursive(pattern.substr(2), text.substr(1)));
        }
        if (pattern[1] == '+') {
            return !text.empty() && (pattern[0] == '.' || pattern[0] == text[0]) && 
                   (match_recursive(pattern, text.substr(1)) || match_recursive(pattern.substr(2), text.substr(1)));
        }
    }

    // Handle groups `()` with alternation `|`
    if (pattern[0] == '(') {
        size_t end_paren_pos = string::npos;
        int level = 0;
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

            // Check for quantifiers on the group
            if (!after_group.empty()) {
                if (after_group[0] == '?') {
                    return match_recursive(after_group.substr(1), text) || 
                           (match_recursive(group_content, text) && match_recursive(after_group.substr(1), text.substr(text.length() - match_recursive_len(group_content, text))));
                }
                 if (after_group[0] == '+') {
                    if (!match_recursive(group_content, text)) return false;
                    string group_with_plus = pattern.substr(0, end_paren_pos + 1) + "+";
                    return match_recursive(group_with_plus + after_group.substr(1), text.substr(text.length() - match_recursive_len(group_content, text)));
                 }
            }


            // Handle alternation inside the group
            size_t pipe_pos = string::npos;
            level = 0;
            for (size_t i = 0; i < group_content.length(); ++i) {
                if (group_content[i] == '(') level++;
                else if (group_content[i] == ')') level--;
                else if (group_content[i] == '|' && level == 0) {
                    pipe_pos = i;
                    break;
                }
            }

            if (pipe_pos != string::npos) {
                string alt_A = group_content.substr(0, pipe_pos);
                string alt_B = group_content.substr(pipe_pos + 1);
                return match_recursive(alt_A + after_group, text) || match_recursive(alt_B + after_group, text);
            } else { // Simple group
                return match_recursive(group_content + after_group, text);
            }
        }
    }
    
    // If we've reached here, it's not a special case, so we need to handle single characters.
    if (text.empty()) return false;

    // Handle `[...]`, `\d`, `\w`, `.`, and literals
    // (This part is simplified for clarity; the full implementation would handle all cases)
    if (pattern[0] == '.' || pattern[0] == text[0]) {
        return match_recursive(pattern.substr(1), text.substr(1));
    }
    
    return false;
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

    try {
        if (match_pattern(input_line, pattern)) {
            return 0;
        } else {
            return 1;
        }
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1;
    }
}