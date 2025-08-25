#include <iostream>
#include <string>

using namespace std;

bool match(const string& text, const string& pattern) {
    for (int i = 0; i <= text.size(); i++) {
        if (match_at_position(text, i, pattern, 0))
            return true;
    }
    return false;
}
bool match_at_position(const string& text, int i, const string& pattern, int j) {
    if (j == pattern.size()) return true; // fully matched
    if (i == text.size()) return false;   // text ended early

    if (pattern[j] == '\\') { // handle special class
        if (j+1 < pattern.size()) {
            char c = text[i];
            if (pattern[j+1] == 'd' && isdigit(c))
                return match_at_position(text, i+1, pattern, j+2);
            if (pattern[j+1] == 'w' && isalnum(c))
                return match_at_position(text, i+1, pattern, j+2);
        }
    } else { // literal char
        if (text[i] == pattern[j])
            return match_at_position(text, i+1, pattern, j+1);
    }

    return false;
}

int main(int argc, char* argv[]) { // argc - number of argumnets && argv - array of C-style strings (the actual arguments).

    // Flush after every std::cout / std::cerr
    cout << unitbuf;//disable output buffering - Normally, output waits in a buffer until flushed, but with unitbuf, everything gets printed immediately
    cerr << unitbuf;

    cerr << "Logs from your program will appear here" << endl;

    if (argc != 3) { // if 3 argumnets return because we need 2
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
