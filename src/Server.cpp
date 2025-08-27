#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;

// These helpers are no longer needed by the new match_pattern function
bool isDigit(char c){
    return isdigit(c);
}
bool isAlpha(char c){
    return isalpha(c);
}

// Forward declaration for our new recursive helper function
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx);

// This is the new, more powerful matching function that replaces the old one
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

// This recursive helper function is the new "brain" of the regex engine
bool match_here(const string& pattern, int p_idx, const string& text, int t_idx) {
    // If we've reached the end of the pattern, we've succeeded
    if (p_idx == (int)pattern.length()) {
        return true;
    }

    // Handle '$' anchor: it must match the end of the text
    if (pattern[p_idx] == '$' && p_idx == (int)pattern.length() - 1) {
        return t_idx == (int)text.length();
    }

    // Handle the '+' quantifier with backtracking
    if (p_idx + 1 < (int)pattern.length() && pattern[p_idx + 1] == '+') {
        // Enforce the "one" part of "one or more". Must match at least once.
        if (t_idx >= (int)text.length() || (pattern[p_idx] != '.' && pattern[p_idx] != text[t_idx])) {
            return false;
        }
        // After matching one, it's a choice: either match more of the '+' pattern
        // OR move on to the rest of the pattern. The recursion handles this choice.
        return match_here(pattern, p_idx, text, t_idx + 1) || match_here(pattern, p_idx + 2, text, t_idx + 1);
    }

    // Handle the '?' quantifier
    if (p_idx + 1 < (int)pattern.length() && pattern[p_idx + 1] == '?') {
        // Try two paths: 1) Skip the optional character. 2) Match one character and continue.
        return match_here(pattern, p_idx + 2, text, t_idx) ||
               (t_idx < (int)text.length() && (pattern[p_idx] == '.' || pattern[p_idx] == text[t_idx]) && match_here(pattern, p_idx + 2, text, t_idx + 1));
    }

    // Handle a standard character match (literal or '.')
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