#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;

bool isDigit(char c){
    return isdigit(c);
}

bool isAlpha(char c){
    return isalpha(c);
}

bool match_pattern(const string& input_line, const string& pattern) {
    // Simple cases for patterns that are just one token
    if (pattern.length() == 1 && pattern != ".") {
        return input_line.find(pattern) != string::npos;
    }
    if (pattern == "\\d") {
        return input_line.find_first_of("0123456789") != string::npos;
    }
    if (pattern == "\\w") {
        return input_line.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != string::npos;
    }
    if (pattern.size() >= 4 && pattern[0] == '[' && pattern[1] == '^' && pattern[pattern.size() - 1] == ']') {
        string str = pattern.substr(2, pattern.size() - 3);
        return (input_line.find_first_not_of(str) != string::npos);
    }
    if (pattern.size() >= 3 && pattern[0] == '[' && pattern[pattern.size() - 1] == ']') {
        string str = pattern.substr(1, pattern.size() - 2);
        return input_line.find_first_of(str) != string::npos;
    }

    // This is the main matching engine for all complex patterns
    for (int j = 0; j <= (int)input_line.length(); ++j) {
        string sub = input_line.substr(j);
        int ptr = 0;
        bool flag = true;

        for (int i = 0; i < (int)pattern.size(); i++) {
            char ch = pattern[i];

            if (ptr >= sub.size()) {
                if (i + 1 < (int)pattern.size() && pattern[i + 1] == '?') {
                    i++;
                    continue;
                }
                flag = false;
                break;
            }

            if (i + 1 < (int)pattern.size() && pattern[i + 1] == '+') {
                int cnt = 0;
                while (ptr < sub.size() && (ch == '.' || sub[ptr] == ch)) {
                    ptr++;
                    cnt++;
                }
                if (i + 2 < (int)pattern.size() && (ch == '.' || pattern[i + 2] == ch)) {
                    if (cnt > 0) {
                        ptr--;
                    }
                }
                if (cnt == 0) {
                    flag = false;
                    break;
                }
                i++;
            } else if (i + 1 < (int)pattern.size() && pattern[i + 1] == '?') {
                if (ptr < (int)sub.size() && (ch == '.' || sub[ptr] == ch)) {
                    ptr++;
                }
                i++;
            } else if (ch == '\\' && i + 1 < pattern.size()) {
                char meta_char = pattern[i + 1];
                bool matches = false;
                if (meta_char == 'd' && isdigit(sub[ptr])) {
                    matches = true;
                } else if (meta_char == 'w' && (isalnum(sub[ptr]) || sub[ptr] == '_')) {
                    matches = true;
                }
                if (matches) {
                    ptr++;
                    i++;
                } else {
                    flag = false;
                    break;
                }
            } else {
                if (ch != '.' && ch != sub[ptr]) {
                    flag = false;
                    break;
                }
                ptr++;
            }
        }
        if (flag) return true;
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