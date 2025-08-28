#include <iostream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

// Check if text matches regexp starting here
bool match_here(const string& text, const string& regexp);

// Handle '+' quantifier: one or more matches of 'match' char or string then regexp
bool match_plus(const string& text, const string& match, const string& regexp) {
    size_t pos = 0;
    while (pos < text.size() && text.compare(pos, match.size(), match) == 0) {
        pos += match.size();
        if (match_here(text.substr(pos), regexp)) return true;
    }
    return false;
}

// Handle '?' quantifier: zero or one match of 'match' char or string then regexp
bool match_question_mark(const string& text, const string& match, const string& regexp) {
    if (text.compare(0, match.size(), match) == 0) {
        return match_here(text.substr(match.size()), regexp);
    } else {
        return match_here(text, regexp);
    }
}


bool match_here(const string& text, const string& regexp) {
    if (regexp.empty()) return true;
    if (!text.empty() && regexp[0] == '$') return false;
    if (regexp[0] == '$') return text.empty();

    // Handle group alternation like (cat|dog)
    if (!regexp.empty() && regexp[0] == '(' && regexp.back() == ')') {
        string group = regexp.substr(1, regexp.size() - 2);
        size_t pipe_pos = group.find('|');
        if (pipe_pos != string::npos) {
            string alt1 = group.substr(0, pipe_pos);
            string alt2 = group.substr(pipe_pos + 1);
            return match_here(text, alt1) || match_here(text, alt2);
        } else {
            // no alternation inside group
            return match_here(text, group);
        }
    }

    // Handle quantifiers '+' and '?'
    if (regexp.size() > 1) {
        if (regexp[1] == '+') {
            string match_str = regexp.substr(0,1);
            return match_plus(text, match_str, regexp.substr(2));
        }
        if (regexp[1] == '?') {
            string match_str = regexp.substr(0,1);
            return match_question_mark(text, match_str, regexp.substr(2));
        }
    }

    // Escapes \d and \w
    if (regexp.size() >= 2 && regexp.substr(0,2) == "\\d") {
        if (!text.empty() && isdigit(text[0]))
            return match_here(text.substr(1), regexp.substr(2));
        return false;
    }
    if (regexp.size() >= 2 && regexp.substr(0,2) == "\\w") {
        if (!text.empty() && (isalnum(text[0]) || text[0] == '_'))
            return match_here(text.substr(1), regexp.substr(2));
        return false;
    }

    // '.' matches any single character
    if (!regexp.empty() && regexp[0] == '.') {
        if (!text.empty())
            return match_here(text.substr(1), regexp.substr(1));
        return false;
    }

    // Literal character match
    if (!regexp.empty() && !text.empty() && regexp[0] == text[0]) {
        return match_here(text.substr(1), regexp.substr(1));
    }

    return false;
}

bool match_pattern(const string& input_line, const string& pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        return match_here(input_line, pattern.substr(1));
    }
    for (size_t i = 0; i < input_line.size(); i++) {
        if (match_here(input_line.substr(i), pattern)) return true;
    }
    return false;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Expected two arguments" << endl;
        return 1;
    }
    if (string(argv[1]) != "-E") {
        cerr << "Expected first argument to be '-E'" << endl;
        return 1;
    }

    string input_line;
    getline(cin, input_line);

    string pattern = argv[2];

    if (match_pattern(input_line, pattern)) {
        return 0;
    } else {
        return 1;
    }
}
