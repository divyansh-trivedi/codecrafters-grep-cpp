#include <iostream>
#include <string>

using namespace std;
bool isDigit(char c) {
    return c >= '0' && c <= '9';
}
bool isAlpha(char c) {
    return c >= 'a' && c <= 'z';
}
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.length() == 1) {
        return input_line.find(pattern) != string::npos;
    }
    else if (pattern == "\\d") {
        return input_line.find_first_of("0123456789") != string::npos;
    }
    else if (pattern == "\\w") {
        return input_line.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != string::npos;
    }
    else if (pattern.size() >= 4 && pattern[0] == '[' && pattern[1] == '^' && pattern[pattern.size() - 1] == ']') {
        string str = pattern.substr(2, pattern.size() - 3);
        return (input_line.find_first_not_of(str) != string::npos);
    }
    else if (pattern.size() >= 3 && pattern[0] == '[' && pattern[pattern.size() - 1] == ']') {
        string str = pattern.substr(1, pattern.size() - 2);
        return input_line.find_first_of(str) != string::npos;
    }
    else if (pattern[pattern.size() - 1] == '$') {
        string temp = pattern.substr(0, pattern.size() - 1);
        if (pattern[0] == '^')
            temp = pattern.substr(1, pattern.size() - 2);
        if (input_line.size() < temp.size()) return false;
        int start = input_line.size() - temp.size();
        for (int i = 0; i < (int)temp.size(); i++) {
            if (input_line[start + i] != temp[i]) return false;
        }
        return true;
    }
    else if (pattern[0] == '^') {
        for (int i = 1; i < (int)pattern.size(); i++) {
            if (pattern[i] != input_line[i - 1]) return false;
        }
        return true;
    }
    else if (true) {
        int len = input_line.size();
        for (int j = 0; j < len; j++) {
            string sub = input_line.substr(j);
            int ptr = 0;
            bool flag = true;

            for (int i = 0; i < (int)pattern.size(); i++) {
                char ch = pattern[i];
                if (ptr >= (int)sub.size()) {
                    flag = false;
                    break;
                }
                if (ch == '\\') continue;
                if (ch == 'd' && i - 1 >= 0 && pattern[i - 1] == '\\') {
                    if (!isDigit(sub[ptr])) {
                        flag = false;
                        break;
                    }
                }
                else if (ch == 'w' && i - 1 >= 0 && pattern[i - 1] == '\\') {
                    char c = sub[ptr];
                    if ((isalpha(c) || isDigit(c) || c == '_') == false) {
                        flag = false;
                        break;
                    }
                }

                // ----------- CHANGED BLOCK START --------------
                else if (ch == '+' && i > 0) {
                    char prev = pattern[i - 1];
                    int cnt = 0;
                    while (ptr < (int)sub.size() && sub[ptr] == prev) {
                        ptr++;
                        cnt++;
                    }
                    if (cnt == 0) {
                        flag = false;
                        break;
                    }
                    continue;
                }
                // ----------- CHANGED BLOCK END ----------------

                else if (ch != ' ' && ch != sub[ptr]) {
                    flag = false;
                    break;
                }
                ptr++;
            }
            cout << sub << " " << flag << endl;
            if (flag) return true;
        }
        return false;
    }
    else {
        throw runtime_error("Unhandled pattern " + pattern);
    }
}

int main(int argc, char* argv[]) {
    cout << unitbuf;
    cerr << unitbuf;
    cerr << "Logs from your program will appear here" << endl;
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
        }
        else {
            return 1;
        }
    }
    catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
