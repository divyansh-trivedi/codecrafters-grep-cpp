#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <utility>
#include <cctype>

using namespace std;

// Forward declaration
bool match_here(const string& pattern, size_t pattern_idx, const string& text, size_t text_idx);

bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.empty()) return true;
    // Correct check for start anchor '^'
    if (!pattern.empty() && pattern[0] == '^') {
        return match_here(pattern, 1, input_line, 0);
    }
    for (size_t i = 0; i <= input_line.size(); i++) {
        if (match_here(pattern, 0, input_line, i)) {
            return true;
        }
    }
    return false;
}

bool match_here(const string& pattern, size_t pattern_idx, const string& text, size_t text_idx) {
    if (pattern_idx == pattern.size()) return true;

    // Handle '$' anchor at end of pattern
    if (pattern[pattern_idx] == '$' && pattern_idx == pattern.size() - 1) {
        return text_idx == text.size();
    }

    // Handle grouping and alternation (e.g. (cat|dog|cow))
    if (pattern[pattern_idx] == '(') {
        size_t end_paren = string::npos;
        vector<size_t> pipes;
        int level = 0;
        for (size_t i = pattern_idx + 1; i < pattern.size(); ++i) {
            if (pattern[i] == '(') level++;
            else if (pattern[i] == ')') {
                if (level-- == 0) {
                    end_paren = i;
                    break;
                }
            } else if (pattern[i] == '|' && level == 0) {
                pipes.push_back(i);
            }
        }
        if (end_paren != string::npos) {
            string after_group = pattern.substr(end_paren + 1);
            size_t start = pattern_idx + 1;
            vector<pair<size_t, size_t>> alternatives;
            for (size_t k = 0; k <= pipes.size(); ++k) {
                size_t pipe_pos = (k < pipes.size()) ? pipes[k] : end_paren;
                alternatives.push_back(make_pair(start, pipe_pos));
                start = pipe_pos + 1;
            }
            for (const auto& alt : alternatives) {
                string alt_pattern = pattern.substr(alt.first, alt.second - alt.first) + after_group;
                if (match_here(alt_pattern, 0, text, text_idx)) {
                    return true;
                }
            }
            return false;
        }
    }

    // Handle quantifiers: '?' and '+'
    if (pattern_idx + 1 < pattern.size()) {
        char quantifier = pattern[pattern_idx + 1];
        if (quantifier == '?') {
            return (text_idx < text.size() &&
                    (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx]) &&
                    match_here(pattern, pattern_idx + 2, text, text_idx + 1)) ||
                   match_here(pattern, pattern_idx + 2, text, text_idx);
        }
        if (quantifier == '+') {
            if (text_idx < text.size() &&
                (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx])) {
                return match_here(pattern, pattern_idx, text, text_idx + 1) ||
                       match_here(pattern, pattern_idx + 2, text, text_idx + 1);
            }
            return false;
        }
    }

    // Handle character class [..] and negated [^..]
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

    // Handle escaped sequences \d and \w
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

    // Default: literal characters or '.'
    if (text_idx < text.size() &&
        (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx])) {
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
