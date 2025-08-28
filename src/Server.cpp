#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;

// Forward declaration for the recursive helper function
bool match_here(const string& pattern, size_t pattern_idx, const string& text, size_t text_idx);

// The main entry point for the regex engine.
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.empty()) return true;

    // Handle start anchor. If present, we only try to match from the beginning.
    if (pattern[0] == '^') {
        return match_here(pattern, 1, input_line, 0);
    }

    // For unanchored patterns, we loop and try to match at every possible start position.
    for (size_t i = 0; i <= input_line.size(); i++) {
        if (match_here(pattern, 0, input_line, i)) {
            return true;
        }
    }
    return false;
}

// The core recursive engine. This function definition was likely missing from your file.
bool match_here(const string& pattern, size_t pattern_idx, const string& text, size_t text_idx) {
    // Base Case: If we've successfully consumed the entire pattern, we have a match.
    if (pattern_idx == pattern.size()) {
        return true;
    }

    // Handle '$' anchor: it only matches if we are at the end of the text.
    if (pattern[pattern_idx] == '$' && pattern_idx == pattern.size() - 1) {
        return text_idx == text.size();
    }

    // Handle grouping `()` with alternation `|`. This must be checked before single-char quantifiers.
    if (pattern[pattern_idx] == '(') {
        size_t pipe_pos = string::npos;
        size_t end_paren_pos = string::npos;
        int paren_level = 0;
        // Find the matching ')' and the top-level '|' for this group
        for (size_t i = pattern_idx + 1; i < pattern.size(); i++) {
            if (pattern[i] == '(') paren_level++;
            else if (pattern[i] == ')') {
                if (paren_level-- == 0) {
                    end_paren_pos = i;
                    break;
                }
            } else if (pattern[i] == '|' && paren_level == 0) {
                pipe_pos = i;
            }
        }

        if (end_paren_pos != string::npos) {
            string after_group = pattern.substr(end_paren_pos + 1);

            // Handle alternation (A|B)
            if (pipe_pos != string::npos && pipe_pos < end_paren_pos) {
                string alt_A = pattern.substr(pattern_idx + 1, pipe_pos - (pattern_idx + 1));
                string alt_B = pattern.substr(pipe_pos + 1, end_paren_pos - (pipe_pos + 1));
                // Try (A followed by the rest) OR (B followed by the rest)
                return match_here(alt_A + after_group, 0, text, text_idx) ||
                       match_here(alt_B + after_group, 0, text, text_idx);
            } else {
                // It's a simple group (A), so we effectively remove the parentheses
                string group_content = pattern.substr(pattern_idx + 1, end_paren_pos - (pattern_idx + 1));
                return match_here(group_content + after_group, 0, text, text_idx);
            }
        }
    }

    // This logic handles a single character/unit followed by a quantifier
    if (pattern_idx + 1 < pattern.size()) {
        char quantifier = pattern[pattern_idx + 1];
        if (quantifier == '?') {
            return (text_idx < text.size() && (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx]) && match_here(pattern, pattern_idx + 2, text, text_idx + 1)) ||
                   match_here(pattern, pattern_idx + 2, text, text_idx);
        }
        if (quantifier == '+') {
            if (text_idx < text.size() && (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx])) {
                // Match one, then it's a choice: match more of this char OR match the rest of the pattern
                return match_here(pattern, pattern_idx, text, text_idx + 1) ||
                       match_here(pattern, pattern_idx + 2, text, text_idx + 1);
            }
            return false;
        }
    }

    // Handle single character "atoms" like [...], \d, \w, and literals
    if (pattern[pattern_idx] == '[') {
        size_t end_bracket = pattern.find(']', pattern_idx);
        if (end_bracket != string::npos && text_idx < text.size()) {
            bool is_negated = pattern[pattern_idx + 1] == '^';
            size_t start = pattern_idx + (is_negated ? 2 : 1);
            string group = pattern.substr(start, end_bracket - start);
            bool found = group.find(text[text_idx]) != string::npos;
            if (is_negated != found) {
                return match_here(pattern, end_bracket + 1, text, text_idx + 1);
            }
        }
        return false;
    }

    if (pattern[pattern_idx] == '\\' && pattern_idx + 1 < pattern.size()) {
        if (text_idx < text.size()) {
            char meta = pattern[pattern_idx + 1];
            if ((meta == 'd' && isdigit(text[text_idx])) ||
                (meta == 'w' && (isalnum(text[text_idx]) || text[text_idx] == '_'))) {
                return match_here(pattern, pattern_idx + 2, text, text_idx + 1);
            }
        }
        return false;
    }

    // Default: literal character or '.'
    if (text_idx < text.size() && (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx])) {
        return match_here(pattern, pattern_idx + 1, text, text_idx + 1);
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