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
        // For an anchored pattern, the match must consume the entire line.
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
            
            // --- START OF THE CRITICAL FIX ---
            // Save the state before trying the left path
            int original_group_count = group_count;
            auto original_captures = captured_groups;

            int len = match_recursive(left, text, group_count);
            if (len != -1) return len;

            // Restore the state before trying the right path
            group_count = original_group_count;
            captured_groups = original_captures;
            // --- END OF THE CRITICAL FIX ---

            return match_recursive(right, text, group_count);
        }
    }
    
    // Determine the "atom" (the unit to be matched) and check for a quantifier after it.
    size_t atom_len = 1;
    if (pattern[0] == '\\' && pattern.length() > 1) {
        atom_len = 2;
    } else if (pattern[0] == '[') {
        size_t end_bracket = pattern.find(']', 1);
        if (end_bracket != string::npos) atom_len = end_bracket + 1;
    } else if (pattern[0] == '(') {
        int level = 0;
        for (size_t i = 1; i < pattern.length(); ++i) {
            if (pattern[i] == '(') level++;
            else if (pattern[i] == ')') {
                if (level-- == 0) {
                    atom_len = i + 1;
                    break;
                }
            }
        }
    }

    char quantifier = (atom_len < pattern.length()) ? pattern[atom_len] : 0;
    string atom = pattern.substr(0, atom_len);
    string rest = pattern.substr(atom_len + (quantifier == '?' || quantifier == '+' ? 1 : 0));

    if (quantifier == '?') {
        // Path A (skip atom): Try matching the rest of the pattern.
        int len = match_recursive(rest, text, group_count);
        if (len != -1) return len;
        
        // Path B (match atom): Match the atom, then the rest.
        int atom_match_len = match_recursive(atom, text, group_count);
        if (atom_match_len != -1) {
            int rest_len = match_recursive(rest, text.substr(atom_match_len), group_count);
            if (rest_len != -1) return atom_match_len + rest_len;
        }
        return -1;
    }

    if (quantifier == '+') {
        // Must match the atom at least once.
        int atom_match_len = match_recursive(atom, text, group_count);
        if (atom_match_len == -1) return -1;

        string remaining_text = text.substr(atom_match_len);
        
        // Path A (Greedy): Try to match the quantified atom again.
        int more_len = match_recursive(pattern.substr(0, atom_len + 1), remaining_text, group_count);
        if (more_len != -1) return atom_match_len + more_len;

        // Path B (Backtrack): Try to match the rest of the pattern.
        int rest_len = match_recursive(rest, remaining_text, group_count);
        if (rest_len != -1) return atom_match_len + rest_len;
        
        return -1;
    }

    // Handle the actual matching of the atom
    if (pattern[0] == '(') {
        string group_content = pattern.substr(1, atom_len - 2);
        int current_capture_index = ++group_count;
        int group_len = match_recursive(group_content, text, group_count);
        if (group_len != -1) {
            captured_groups[current_capture_index] = text.substr(0, group_len);
            int rest_len = match_recursive(rest, text.substr(group_len), group_count);
            if (rest_len != -1) return group_len + rest_len;
        }
        captured_groups.erase(current_capture_index); // Backtrack
        return -1;
    }
    
    if (text.empty()) return -1;

    if (pattern[0] == '\\' && pattern.length() > 1) {
        if (isdigit(pattern[1])) { // Backreference
            int group_num = pattern[1] - '0';
            if (captured_groups.count(group_num)) {
                string captured = captured_groups[group_num];
                if (text.rfind(captured, 0) == 0) {
                    int rest_len = match_recursive(rest, text.substr(captured.length()), group_count);
                    if (rest_len != -1) return captured.length() + rest_len;
                }
            }
            return -1;
        }
        if ((pattern[1] == 'd' && isdigit(text[0])) || (pattern[1] == 'w' && (isalnum(text[0]) || text[0] == '_'))) {
            int len = match_recursive(rest, text.substr(1), group_count);
            if (len != -1) return 1 + len;
        }
        return -1;
    }

    if (pattern[0] == '[') {
        bool is_negated = pattern[1] == '^';
        string group = pattern.substr(is_negated ? 2 : 1, atom_len - (is_negated ? 2 : 1) -1);
        bool found = group.find(text[0]) != string::npos;
        if (is_negated != found) {
            int len = match_recursive(rest, text.substr(1), group_count);
            if (len != -1) return 1 + len;
        }
        return -1;
    }

    if (pattern[0] == '.' || pattern[0] == text[0]) {
        int len = match_recursive(rest, text.substr(1), group_count);
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
