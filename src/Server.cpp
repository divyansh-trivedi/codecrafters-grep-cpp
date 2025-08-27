#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;
bool  isDigit(char c){
    return c>='0' && c<='9';
}
bool  isAlpha(char c){
    return isalpha(c); // Use the standard library function for robustness
}
bool match_pattern(const string& input_line, const string& pattern) {
    // These simple handlers are good for performance but are limited.
    // The main engine below is what handles complex, combined patterns.
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
        int start = input_line.size() - temp.size();
        for(size_t i = 0; i < temp.size(); i++){
            if(input_line[start + i] != temp[i]) return false;
        }
        return true;
    }
    else if(pattern[0] == '^'){
        // This simple logic is flawed for complex patterns, so we rely on the main engine
    }

    // Main matching engine for complex patterns
    for(int j=0; j<=(int)input_line.length(); j++){
        string sub = input_line.substr(j);
        int ptr = 0;
        bool flag = true;

        for(int i=0; i<(int)pattern.size(); i++){
            char ch =  pattern[i];

            if(ptr >= (int)sub.size()){
                flag = false;
                break;
            }
            if(i+1 < (int)pattern.size() && pattern[i+1]  == '+'){
                int cnt =0;
                while (ptr<(int)sub.size()  && (sub[ptr] == ch || ch == '.')) {
                    ptr++;
                    cnt++;
                }
                if(i+2 < (int)pattern.size() && (pattern[i+2] == ch || ch == '.')){
                    if(cnt > 0){
                        ptr--;
                    }
                }
                if(cnt == 0){
                    flag = false;
                    break;
                }
                i++;
            } else if (i+1 < (int)pattern.size() && pattern[i+1] == '?') {
                if (ptr < (int)sub.size() && (ch == '.' || sub[ptr] == ch)) {
                    ptr++;
                }
                i++;
            }
            // =================================================================
            // START OF MINIMAL CHANGE
            // This new block teaches the main engine how to handle character groups.
            // =================================================================
            else if (ch == '[') {
                size_t end_bracket_pos = pattern.find(']', i);
                if (end_bracket_pos == string::npos) { // Malformed pattern
                    flag = false;
                    break;
                }
                
                bool is_negated = (pattern[i + 1] == '^');
                int start_char_pos = is_negated ? i + 2 : i + 1;
                string char_group = pattern.substr(start_char_pos, end_bracket_pos - start_char_pos);
                
                bool found_in_group = (char_group.find(sub[ptr]) != string::npos);

                if (is_negated == found_in_group) { // If negated and found, or not negated and not found, it's a failure.
                    flag = false;
                    break;
                }

                ptr++; // Consume one character from input
                i = end_bracket_pos; // Jump the pattern index past the entire [...] group
            }
            // =================================================================
            // END OF MINIMAL CHANGE
            // =================================================================
            else if(ch == '\\' && i+1 < (int)pattern.size()){
                char meta_char = pattern[i+1];
                bool matches = false;
                if(meta_char == 'd' && isDigit(sub[ptr])){
                    matches = true;
                } else if(meta_char == 'w' && (isAlpha(sub[ptr])  || isDigit(sub[ptr]) || sub[ptr] == '_')){ // Corrected this line
                    matches = true;
                }
                if(matches){
                    ptr++;
                    i++;
                } else {
                    flag = false;
                    break;
                }
            } else {
                if(ch != '.' && ch != sub[ptr]){
                    flag = false;
                    break;
                }
                ptr++;
            }
        }
        if(flag) return true;
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