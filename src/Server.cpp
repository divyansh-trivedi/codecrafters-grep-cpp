#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;

// Forward declaration
bool match_recursive(const string& pattern, const string& text);

// Main entry point for regex engine
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

// Core recursive regex engine
bool match_recursive(const string& pattern, const string& text) {
    // Base cases
    if (pattern.empty()) return true;
    if (pattern[0] == '$' && pattern.length() == 1) return text.empty();

    // Handle top-level alternation (pattern like "cat|dog")
    {
        int depth = 0;
        for (size_t i = 0; i < pattern.size(); ++i) {
            if (pattern[i] == '(') depth++;
            else if (pattern[i] == ')') depth--;
            else if (pattern[i] == '|' && depth == 0) {
                string left = pattern.substr(0, i);
                string right = pattern.substr(i + 1);
                return match_recursive(left, text) || match_recursive(right, text);
            }
        }
    }

    // Handle groups () with alternation and quantifiers
    if (pattern[0] == '(') {
        int level = 0;
        size_t end_paren_pos = string::npos;
        for (size_t i = 1; i < pattern.length(); ++i) {
            if (pattern[i] == '(') level++;
            else if (pattern[i] == ')') {
                if (level == 0) {
                    end_paren_pos = i;
                    break;
                } else {
                    level--;
                }
            }
        }

        if (end_paren_pos != string::npos) {
            string group_content = pattern.substr(1, end_paren_pos - 1);
            string after_group = pattern.substr(end_paren_pos + 1);

            // Handle alternation inside group
            int depth = 0;
            for (size_t i = 0; i < group_content.size(); ++i) {
                if (group_content[i] == '(') depth++;
                else if (group_content[i] == ')') depth--;
                else if (group_content[i] == '|' && depth == 0) {
                    string left = group_content.substr(0, i);
                    string right = group_content.substr(i + 1);
                    return match_recursive(left + after_group, text) ||
                           match_recursive(right + after_group, text);
                }
            }

            // Handle group followed by +
if (!after_group.empty() && after_group[0] == '+') {
    string rest = after_group.substr(1);
    string remaining = text;

    // Must match the group at least once
    if (!match_recursive(group_content, remaining)) return false;

    // Consume group repeatedly
    while (!remaining.empty() && match_recursive(group_content, remaining)) {
        // Try moving forward after consuming one group
        for (size_t cut = 1; cut <= remaining.size(); ++cut) {
            if (match_recursive(group_content, remaining.substr(0, cut))) {
                if (match_recursive("(" + group_content + ")*" + rest, remaining.substr(cut))) {
                    return true;
                }
            }
        }
        break;
    }

    // Or just continue with the rest of the pattern
    return match_recursive(rest, remaining);
}


            // Handle group followed by ?
            if (!after_group.empty() && after_group[0] == '?') {
                return match_recursive(after_group.substr(1), text) ||
                       (match_recursive(group_content, text) &&
                        match_recursive(after_group.substr(1), text.substr(text.size() - text.size()))); 
            }

            // No special quantifier: just expand the group
            return match_recursive(group_content + after_group, text);
        }
    }

    // Handle character-level quantifiers ? and +
    if (pattern.length() > 1) {
        if (pattern[1] == '?') {
            return match_recursive(pattern.substr(2), text) ||
                   (!text.empty() &&
                    (pattern[0] == '.' || pattern[0] == text[0]) &&
                    match_recursive(pattern.substr(2), text.substr(1)));
        }
        if (pattern[1] == '+') {
            return !text.empty() &&
                   (pattern[0] == '.' || pattern[0] == text[0]) &&
                   (match_recursive(pattern, text.substr(1)) ||
                    match_recursive(pattern.substr(2), text.substr(1)));
        }
    }

    // If text is empty here, fail
    if (text.empty()) return false;

    // Handle \d (digit class)
    if (pattern[0] == '\\' && pattern.length() > 1) {
        if (pattern[1] == 'd') {
            if (!text.empty() && isdigit(text[0])) {
                return match_recursive(pattern.substr(2), text.substr(1));
            } else {
                return false;
            }
        }
    }

    // Handle literal match or dot
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
