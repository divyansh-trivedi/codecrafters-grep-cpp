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
    else if(pattern.size() >=3 && pattern[0] == '[' && pattern[pattern.size()-1] == ']'){
        string str = pattern.substr(1,pattern.size()-2);
        return input_line.find_first_of(str) != string::npos;
    }else if(true){
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
                if(ch == '\\')continue; 
                if(ch == 'd' && i-1>=0 && pattern[i-1] == '\\'){// 0 to 9
                    if(!isDigit(sub[ptr])){
                        flag = false;
                        break;
                    }
                }else if(ch == 'w' && i-1>=0 && sub[i-1] == '\\'){// A-Z, a-z , 0-9,'_'
                    char c = sub[ptr];
                    if((isalpha(c) || isDigit(c) || c=='_') == false){
                        flag = false;
                        break;
                    }
                }else if(ch != ' ' && ch != sub[ptr]){// // If not special (\d or \w), then it must match exactly (except space handling)
                    flag = false;
                    break;
                }
                ptr++;
            }
            cout<<sub<<" "<<flag<<endl;
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
