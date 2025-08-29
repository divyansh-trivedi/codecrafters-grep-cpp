#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <unordered_map>

using namespace std;

// --- Global state for captured groups ---
// This map stores the text matched by each capturing group, like (cat).
unordered_map<int, string> captured_groups;

// Forward declaration for our main recursive function
int match_recursive(const string& pattern, const string& text, int& group_count);

/**
 * The main entry point for the regex engine.
 * It handles the start-of-line anchor `^` and the unanchored search loop.
 */
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.empty()) return true;

    // If the pattern is anchored with '^', we only try to match from the start.
    // The match must consume the entire input line.
    if (pattern[0] == '^') {
        int group_count = 0;
        return match_recursive(pattern.substr(1), input_line, group_count) == (int)input_line.length();
    }

    // For a normal, unanchored pattern, we loop through the input.
    // We try to find a match starting at every possible position.
    for (size_t i = 0; i <= input_line.length(); ++i) {
        captured_groups.clear(); // Reset captures for each new attempt
        int group_count = 0;
        if (match_recursive(pattern, input_line.substr(i), group_count) != -1) {
            return true;
        }
    }
    return false;
}

/**
 * The core recursive engine.
 * Tries to match the `pattern` against the `text`.
 * Returns the length of the matched text, or -1 if no match is found.
 */
int match_recursive(const string& pattern, const string& text, int& group_count) {
    // Base Case 1: An empty pattern successfully matches 0 characters.
    if (pattern.empty()) return 0;

    // Base Case 2: '$' anchor only matches at the very end of the text.
    if (pattern[0] == '$' && pattern.length() == 1) {
        return text.empty() ? 0 : -1;
    }

    // --- Operator Precedence: Handle lowest precedence first ---
    // Handle top-level alternation (e.g., "cat|dog"). This is the lowest precedence operation.
    int depth = 0;
    for (int i = pattern.length() - 1; i >= 0; --i) {
        if (pattern[i] == ')') depth++;
        else if (pattern[i] == '(') depth--;
        else if (pattern[i] == '|' && depth == 0) {
            string left_alt = pattern.substr(0, i);
            string right_alt = pattern.substr(i + 1);
            
            // Save state before trying the left path
            int original_group_count = group_count;
            auto original_captures = captured_groups;

            int len = match_recursive(left_alt, text, group_count);
            if (len != -1) return len;

            // If left path failed, restore state and try the right path
            group_count = original_group_count;
            captured_groups = original_captures;
            
            return match_recursive(right_alt, text, group_count);
        }
    }

    // --- Atom Identification ---
    // Determine the "atom" (the fundamental unit to match) and any quantifier that follows it.
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

    // --- Quantifier Logic with Backtracking ---
    if (quantifier == '?') {
        // Save state before trying paths
        int original_group_count = group_count;
        auto original_captures = captured_groups;

        // Path A (skip atom): Try matching the rest of the pattern.
        int len = match_recursive(rest, text, group_count);
        if (len != -1) return len;
        
        // Path B (match atom): Restore state and try matching the atom, then the rest.
        group_count = original_group_count;
        captured_groups = original_captures;

        int atom_match_len = match_recursive(atom, text, group_count);
        if (atom_match_len != -1) {
            int rest_len = match_recursive(rest, text.substr(atom_match_len), group_count);
            if (rest_len != -1) return atom_match_len + rest_len;
        }
        return -1;
    }

    if (quantifier == '+') {
        // A+ is equivalent to A followed by A* (which is like A? recursively).
        // First, match the atom once (mandatory).
        int atom_match_len = match_recursive(atom, text, group_count);
        if (atom_match_len == -1) return -1;

        string remaining_text = text.substr(atom_match_len);
        
        // After one match, it's a choice...
        // Save state before trying the greedy path.
        int original_group_count = group_count;
        auto original_captures = captured_groups;
        
        // Path A (Greedy): Match the quantified atom `A+` again.
        int more_len = match_recursive(pattern.substr(0, atom_len + 1), remaining_text, group_count);
        if (more_len != -1) return atom_match_len + more_len;

        // Path B (Backtrack): If greedy path failed, restore state and match the rest of the pattern.
        group_count = original_group_count;
        captured_groups = original_captures;
        
        int rest_len = match_recursive(rest, remaining_text, group_count);
        if (rest_len != -1) return atom_match_len + rest_len;
        
        return -1;
    }

    // --- Atom Matching Logic ---
    // This part runs if there's no quantifier on the current atom.
    if (pattern[0] == '(') {
        string group_content = pattern.substr(1, atom_len - 2);
        int current_capture_index = ++group_count;
        
        int group_len = match_recursive(group_content, text, group_count);
        if (group_len != -1) {
            captured_groups[current_capture_index] = text.substr(0, group_len);
            int rest_len = match_recursive(rest, text.substr(group_len), group_count);
            if (rest_len != -1) return group_len + rest_len;
        }
        captured_groups.erase(current_capture_index); // Backtrack capture if rest failed
        return -1;
    }
    
    if (text.empty()) return -1; // Cannot match non-group atoms against empty text

    if (pattern[0] == '\\') {
        if (isdigit(pattern[1])) { // Backreference
            int group_num = pattern[1] - '0';
            if (captured_groups.count(group_num)) {
                string captured = captured_groups[group_num];
                if (text.rfind(captured, 0) == 0) { // C++20: text.starts_with(captured)
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
        string group = pattern.substr(is_negated ? 2 : 1, atom_len - (is_negated ? 3 : 2));
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
        cerr << "Usage: " << argv[0] << " -E \"<pattern>\"" << std::endl;
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

