#include <iostream>
#include <string>

using namespace std;

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool match_sequence(const string& input, const string& pattern) {
    for (size_t i = 0; i + pattern.size() <= input.size(); i++) {
        bool ok = true;
        size_t j = 0;
        for (; j < pattern.size(); j++) {
            if (pattern[j] == '\\' && j + 1 < pattern.size()) {
                if (pattern[j + 1] == 'd') {
                    if (!is_digit(input[i + j])) { ok = false; break; }
                    j++; // skip 'd'
                }
                else if (pattern[j + 1] == 'w') {
                    if (!isalnum(static_cast<unsigned char>(input[i + j])) && input[i + j] != '_') { ok = false; break; }
                    j++; // skip 'w'
                }
                else {
                    ok = false;
                    break;
                }
            }
            else if (input[i + j] != pattern[j]) {
                ok = false;
                break;
            }
        }
        if (ok) return true;
    }
    return false;
}


bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.length() == 1) {
        return input_line.find(pattern) != string::npos;
    }
    else if (pattern == "\\d") {
        // Match any digit character in input_line
        return input_line.find_first_of("0123456789") != string::npos;
    }
    else if(pattern == "\\w"){
        return input_line.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != string::npos;
    }
    else if(pattern.size() >=4 && pattern[0] == '['  && pattern[1] == '^'&& pattern[pattern.size()-1] == ']'){
        string str = pattern.substr(2,pattern.size()-3); // substr(position , count)
        return (input_line.find_first_not_of(str) != string::npos);
    }
    else if(pattern.size() >=3 && pattern[0] == '[' && pattern[pattern.size()-1] == ']'){
        string str = pattern.substr(1,pattern.size()-2);
        return input_line.find_first_of(str) != string::npos;
    }
    else {
        throw runtime_error("Unhandled pattern " + pattern);
    }
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
