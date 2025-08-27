#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>

using namespace std;

// The main recursive matcher. No longer needs the 'start_of_line' flag.
bool match_pattern(const string& input, const string& pattern);

// Your main recursive function, now simplified
bool match_pattern(const string& input, const string& pattern) {
    if (pattern.empty()) return true;
    if (pattern[0] == '$' && pattern.length() == 1) return input.empty();
    if (input.empty() && pattern != "$") return false;

    // Handle groups ( ... ) with alternation
    if (pattern[0] == '(') {
        size_t close = pattern.find(')');
        if (close == string::npos) return false; // Malformed group

        string group = pattern.substr(1, close - 1);
        string rest = pattern.substr(close + 1);
        
        // =================================================================
        // START OF CHANGE: Added Alternation `|` Logic
        // =================================================================
        size_t pipe_pos = group.find('|');
        if (pipe_pos != string::npos) {
            string alt_A = group.substr(0, pipe_pos);
            string alt_B = group.substr(pipe_pos + 1);
            
            // Try to match (alternative A followed by the rest) OR (alternative B followed by the rest)
            return match_pattern(input, alt_A + rest) || match_pattern(input, alt_B + rest);
        } else {
            // It's a simple group without alternation, just concatenate
            return match_pattern(input, group + rest);
        }
        // =================================================================
        // END OF CHANGE
        // =================================================================
    }

    // \d for digits
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\d") {
        if (!input.empty() && isdigit(input[0])) 
            return match_pattern(input.substr(1), pattern.substr(2));
        return false;
    }

    // \w for alphanumeric
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\w") {
        if (!input.empty() && (isalnum(input[0]) || input[0] == '_'))
            return match_pattern(input.substr(1), pattern.substr(2));
        return false;
    }

    // Wildcard .
    if (pattern[0] == '.') {
        if (!input.empty())
            return match_pattern(input.substr(1), pattern.substr(1));
        return false;
    }
    
    // Character classes [...]
    if (pattern[0] == '[') {
        size_t close = pattern.find(']');
        if (close == string::npos) return false; // Malformed
        
        bool neg = pattern[1] == '^';
        string chars = neg ? pattern.substr(2, close - 2) : pattern.substr(1, close - 1);
        bool match = !input.empty() && (chars.find(input[0]) != string::npos);
        if (neg) match = !match;
        
        if (match) 
            return match_pattern(input.substr(1), pattern.substr(close + 1));
        return false;
    }
    
    // Literal match with quantifiers
    if (pattern.size() > 1 && pattern[1] == '+') {
        // Must match at least one
        if (input.empty() || input[0] != pattern[0]) return false;
        // After one match, it's a choice: match more OR match the rest of the pattern
        return match_pattern(input.substr(1), pattern) || match_pattern(input.substr(1), pattern.substr(2));
    }
    if (pattern.size() > 1 && pattern[1] == '?') {
        // Choice: match zero OR match one
        return match_pattern(input, pattern.substr(2)) ||
               (!input.empty() && input[0] == pattern[0] && match_pattern(input.substr(1), pattern.substr(2)));
    }

    // Standard Literal match
    if (!input.empty() && input[0] == pattern[0]) 
        return match_pattern(input.substr(1), pattern.substr(1));

    return false;
}

int main(int argc, char* argv[]) {
    cout << std::unitbuf;
    cerr << std::unitbuf;

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

    // =================================================================
    // START OF CHANGE: Main logic to handle unanchored searches
    // =================================================================
    if (pattern[0] == '^') {
        if (match_pattern(input_line, pattern.substr(1))) {
            return 0;
        }
    } else {
        // For unanchored patterns, we loop and try to match at every position
        for (size_t i = 0; i < input_line.length(); ++i) {
            if (match_pattern(input_line.substr(i), pattern)) {
                return 0;
            }
        }
        // Special case for empty pattern matching empty string
        if (pattern.empty() && input_line.empty()) {
            return 0;
        }
    }
    // =================================================================
    // END OF CHANGE
    // =================================================================

    return 1; // Return 1 if no match was found
}