#include <iostream>
#include <string>

using namespace std;
bool  isDigit(char c){
    return c>='0' && c<='9';
}
bool  isAlpha(char c){
    return c>='a' && c<='z';
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

        int start = input_line.size() - temp.size(); // start comparing from end
        for(int i = 0; i < temp.size(); i++){
            if(input_line[start + i] != temp[i]) return false;
        }
        return true;
    }

    else if(pattern[0] == '^'){
        for(int i=1;i<pattern.size();i++){
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

            for(int i=0;i<pattern.size();i++){
                char ch =  pattern[i];

                if(ptr >= sub.size()){// if exceedes
                    flag = false;
                    break;
                }
                if(i+1 < pattern.size()&&pattern[i+1]  == '+'){
                    int cnt =0;
                    while (ptr<sub.size()  && sub[ptr] == ch)
                    {
                        ptr++;
                        cnt++;
                    }
                    if(i+2 < pattern.size() && pattern[i+2] == ch){
                        if(cnt > 0){
                            ptr--;
                        }
                    }

                    if(cnt == 0){
                        flag = false;
                        break;
                    }
                    i++;
                }else if (i + 1 < (int)pattern.size() && pattern[i + 1] == '?') {
                    // It's optional, so we only advance the input pointer if it matches.
                    if (ptr < (int)sub.size() && sub[ptr] == ch) {
                        ptr++;
                    }
                    i++; // Always advance the pattern pointer past '?'
                }
                else if(ch == '\\' && i+1 < pattern.size()){
                    char meta_char = pattern[i+1];
                    bool matches = false;
                    if(meta_char == 'd' && isDigit(sub[ptr])){
                        matches = true;
                    }else if(meta_char == 'w' && (isAlpha(sub[ptr])  || isDigit(sub[ptr]) || sub[ptr] == '_')){
                        matches = true;
                    }
                    if(matches){
                        ptr++;
                        i++;
                    }else{
                        flag = false;
                        break;
                    }
                }else{
                    if(ch != sub[ptr]){
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

int main(int argc, char* argv[]) { // argc - number of argumnets && argv - array of C-style strings (the actual arguments).

    // Flush after every std::cout / std::cerr
    cout << unitbuf;
    //disable output buffering - Normally, output waits in a buffer until flushed, but with unitbuf, everything gets printed immediately
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
