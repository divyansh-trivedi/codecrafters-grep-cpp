#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;

// These helpers are used by the new `match_here` function
bool isDigit(char c){
    return isdigit(c);
}
bool isAlpha(char c){
    return isalpha(c);
}

// Forward declaration for our new recursive helper function
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx);

// The main entry point for matching
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.empty()) {
        return true;
    }
    if (pattern[0] == '^') {
        // If anchored to the start, only try matching from the beginning
        return match_here(pattern, 1, input_line, 0);
    }

    // For unanchored patterns, try to match starting at every position
    for (int i = 0; i <= (int)input_line.length(); ++i) {
        if (match_here(pattern, 0, input_line, i)) {
            return true;
        }
    }
    return false;
}

// The recursive helper function with the new logic for `\d` and `\w`
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx) {
    if (p_idx == (int)pattern.length()) {
        return true;
    }

    if (pattern[p_idx] == '$' && p_idx == (int)pattern.length() - 1) {
        return t_idx == (int)text.length();
    }

    if (p_idx + 1 < (int)pattern.length() && pattern[p_idx + 1] == '+') {
        if (t_idx >= (int)text.length() || (pattern[p_idx] != '.' && pattern[p_idx] != text[t_idx])) {
            return false;
        }
        return match_here(pattern, p_idx, text, t_idx + 1) || match_here(pattern, p_idx + 2, text, t_idx + 1);
    }

    if (p_idx + 1 < (int)pattern.length() && pattern[p_idx + 1] == '?') {
        return match_here(pattern, p_idx + 2, text, t_idx) ||
               (t_idx < (int)text.length() && (pattern[p_idx] == '.' || pattern[p_idx] == text[t_idx]) && match_here(pattern, p_idx + 2, text, t_idx + 1));
    }
    
    // =================================================================
    // START OF NEW CODE for `\` character classes
    // =================================================================
    if (pattern[p_idx] == '\\' && p_idx + 1 < (int)pattern.length()) {
        if (t_idx >= (int)text.length()) { // We need a character to match against
            return false;
        }
        char meta_char = pattern[p_idx + 1];
        bool matches = false;
        if (meta_char == 'd' && isDigit(text[t_idx])) {
            matches = true;
        } else if (meta_char == 'w' && (isalnum(text[t_idx]) || text[t_idx] == '_')) {
            matches = true;
        }

        if (matches) {
            // If the class matches, advance pattern by 2 and text by 1
            return match_here(pattern, p_idx + 2, text, t_idx + 1);
        } else {
            return false;
        }
    }
    // =================================================================
    // END OF NEW CODE
    // =================================================================

    if (t_idx < (int)text.length() && (pattern[p_idx] == '.' || pattern[p_idx] == text[t_idx])) {
        return match_here(pattern, p_idx + 1, text, t_idx + 1);
    }

    return false;
}


// Your main function remains unchanged
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