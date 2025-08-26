#include <iostream>
#include <string>
// START OF MODIFICATION: Added for std::runtime_error
#include <stdexcept>
// END OF MODIFICATION

using namespace std;
bool  isDigit(char c){
    return c>='0' && c<='9';
}
bool  isAlpha(char c){
    // START OF MODIFICATION: User's isAlpha was incomplete for \w
    return isalpha(c);
    // END OF MODIFICATION
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
    else if(pattern.size() >= 3 && pattern[0] == '[' && pattern[pattern.size()-1]== ']'){
        string str = pattern.substr(1,pattern.size()-2);
        return input_line.find_first_of(str) != string::npos;
    }
    else if(pattern[pattern.size()-1] == '$' ){ // for eg pinapple123 is not pinapple$ because it should end with ple not 123
        string temp = pattern.substr(0, pattern.size()-1);

        if(pattern[0] == '^')
            temp = pattern.substr(1, pattern.size()-2);

         if(input_line.size() < temp.size()) return false;

        // START OF MODIFICATION: Corrected types for size comparison
        size_t start = input_line.size() - temp.size(); // start comparing from end
        for(size_t i = 0; i < temp.size(); i++){
        // END OF MODIFICATION
            if(input_line[start + i] != temp[i]) return false;
        }
        return true;
    }

    else if(pattern[0] == '^'){
        // START OF MODIFICATION: Corrected types and logic for start anchor
        if (input_line.length() < pattern.length() - 1) return false;
        for(size_t i=1; i<pattern.size(); i++){
        // END OF MODIFICATION
            if(pattern[i] != input_line[i-1])return false;
        }
        return true;
    }
    else if(true){
        int len = input_line.size();

        for(int j=0;j<len;j++){
            string sub = input_line.substr(j);// substring
            int ptr = 0;// to traverse in string 
            bool flag = true;// false when nothing matches

            // =================================================================
            // START OF MINIMAL CHANGES TO THIS BLOCK
            // The logic inside this loop was fundamentally refactored to correctly
            // handle quantifiers and literal characters.
            // =================================================================
            for(int i=0; i < (int)pattern.size(); i++){
                char ch =  pattern[i];

                if(ptr >= (int)sub.size()){ // Ran out of text to match
                    flag = false;
                    break;
                }

                // Lookahead for the '+' quantifier
                if (i + 1 < (int)pattern.size() && pattern[i + 1] == '+') {
                    int match_count = 0;
                    while (ptr < (int)sub.size() && sub[ptr] == ch) {
                        ptr++;
                        match_count++;
                    }
                    if (match_count == 0) { // '+' requires at least one match
                        flag = false;
                        break;
                    }
                    i++; // Manually advance pattern index to skip the '+'
                } else if (ch == '\\' && i + 1 < (int)pattern.size()) {
                    // Correctly handle meta characters like \d and \w
                    char meta_char = pattern[i + 1];
                    bool matches = false;
                    if (meta_char == 'd' && isDigit(sub[ptr])) {
                        matches = true;
                    } else if (meta_char == 'w' && (isAlpha(sub[ptr]) || isDigit(sub[ptr]) || sub[ptr] == '_')) {
                        matches = true;
                    }

                    if (matches) {
                        ptr++;
                        i++; // Also skip the meta_char itself
                    } else {
                        flag = false;
                        break;
                    }
                } else {
                    // This is the crucial missing piece: handle a normal character match
                    if (ch != sub[ptr]) {
                        flag = false;
                        break;
                    }
                    ptr++; // Advance pointer only on a successful match
                }
            }
            // =================================================================
            // END OF MINIMAL CHANGES TO THIS BLOCK
            // =================================================================
            
            if(flag)return true;
        }
        return false;
    }
    else {
        throw runtime_error("Unhandled pattern " + pattern);
    }
}

int main(int argc, char* argv[]) { // argc - number of argumnets && argv - array of C-style strings (the actual arguments).

    // Flush after every std::cout / std::cerr
    cout << unitbuf;
    //disable output buffering - Normally, output waits in a buffer until flushed, but with unitbuf, everything gets printed immediately
    cerr << unitbuf;

    // cerr << "Logs from your program will appear here" << endl; // Removed for cleaner output

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