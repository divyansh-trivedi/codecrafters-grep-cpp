#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;
bool  isDigit(char c){
    return c>='0' && c<='9';
}
bool  isAlpha(char c){
    return isalpha(c);
}
bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.length() == 1) {
        return input_line.find(pattern) != string::npos;
    }
    else if (pattern == "\\d") {
        return input_line.find_first_of("0123456789") != string::npos;
    }
    else if(pattern == "\\w"){
        return input_line.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != string::npos;
    }
    else if(pattern.size() >=4 && pattern[0] == '['  && pattern[1] == '^'&& pattern[pattern.size()-1] == ']'){
        string str = pattern.substr(2,pattern.size()-3);
        return (input_line.find_first_not_of(str) != string::npos);
    }
    else if(pattern.size() >= 3 && pattern[0] == '[' && pattern[pattern.size()-1]== ']'){
        string str = pattern.substr(1,pattern.size()-2);
        return input_line.find_first_of(str) != string::npos;
    }
    else if(pattern[pattern.size()-1] == '$' ){
        string temp = pattern.substr(0, pattern.size()-1);
        if(pattern[0] == '^')
            temp = pattern.substr(1, pattern.size()-2);
         if(input_line.size() < temp.size()) return false;
        size_t start = input_line.size() - temp.size();
        for(size_t i = 0; i < temp.size(); i++){
            if(input_line[start + i] != temp[i]) return false;
        }
        return true;
    }
    else if(pattern[0] == '^'){
        if (input_line.length() < pattern.length() - 1) return false;
        for(size_t i=1; i<pattern.size(); i++){
            if(pattern[i] != input_line[i-1])return false;
        }
        return true;
    }
    else if(true){
        int len = input_line.size();
        for(int j=0;j<len;j++){
            string sub = input_line.substr(j);
            int ptr = 0;
            bool flag = true;

            for(int i=0; i < (int)pattern.size(); i++){
                char ch =  pattern[i];

                if(ptr >= (int)sub.size()){
                    flag = false;
                    break;
                }

                if (i + 1 < (int)pattern.size() && pattern[i + 1] == '+') {
                    int match_count = 0;
                    while (ptr < (int)sub.size() && sub[ptr] == ch) {
                        ptr++;
                        match_count++;
                    }

                    // =================================================================
                    // START OF MINIMAL CHANGE
                    // This handles cases like "a+a" where the greedy "+" would
                    // consume the character needed by the next part of the pattern.
                    // We "give back" one character in that specific scenario.
                    // =================================================================
                    if (i + 2 < (int)pattern.size() && pattern[i + 2] == ch) {
                        if (match_count > 0) {
                            ptr--;
                        }
                    }
                    // =================================================================
                    // END OF MINIMAL CHANGE
                    // =================================================================

                    if (match_count == 0) {
                        flag = false;
                        break;
                    }
                    i++; 
                } else if (ch == '\\' && i + 1 < (int)pattern.size()) {
                    char meta_char = pattern[i + 1];
                    bool matches = false;
                    if (meta_char == 'd' && isDigit(sub[ptr])) {
                        matches = true;
                    } else if (meta_char == 'w' && (isAlpha(sub[ptr]) || isDigit(sub[ptr]) || sub[ptr] == '_')) {
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
                    if (ch != sub[ptr]) {
                        flag = false;
                        break;
                    }
                    ptr++;
                }
            }
            if(flag)return true;
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