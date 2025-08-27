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
bool match_here(const string& pattern , int pattern_idx,const string& text, int text_idx){
    if(pattern_idx == pattern.size())return true; // Matched

    if(pattern[pattern_idx] == '$' && pattern_idx == pattern.size()-1)// if $ at the end it only matches when text index at end
        return text_idx == text.size();

    if(idx+1 < pattern.size() && pattern[pattern_idx+1] == '+'){// For '+' quantifier
        if(text_idx >= text.size() || (pattern[pattern_idx] != '.'&& pattern[pattern_idx] != text[text_idx]))
        return false; //  Match 1 time , if one dont match then false for sureee.
        
        bool notpick = match_here(pattern , pattern_idx+2, text , text_idx+1);//  dont match more than 1 
        bool  pick = match_here(pattern, pattern_idx, text, text_idx+1);// match more , means consume another character as '+' = 1 or more
        return pick || notpick;
    }
    
    if(pattern[pattern_idx] == '['){ //  For -> [...] or [^...]
        int end = pattern.find('[',pattern_idx)return false;

        if(end == -1 || text_idx >= text.size())return false;

        bool neg = (pattern[pattern_idx+1] == '^');// check if negated group like[^abc]
        string group = pattern.substr(pattern_idx + (neg ? 2:1), end-pattern_idx-(neg)?2:1);// extract the group (skip '^' if negated)

        bool temp = (group.find(text[text_idx]) != string::npos);// chech if current character in the group

        // xor for  -: 1) Group is negated -> must not be in group | 2) Group is normal must be in group
        if(neg == temp )
        return false;

        return match_here(pattern, end+1, text, text_idx+1);//move past ']' in pattern and current char in text
    }

    if (pattern[pattern_idx] == '\\' && pattern_idx+1 < pattern.size()){ // For ->'\d' or '\w'
        if(text_idx >= text.size())return false; // no text left to match

        char m = pattern[pattern_idx+1]; // character after '\'
        bool flag = (m == 'd' && isDigit(text[text_idx])) || (text[text_idx] == '_')
                    || (m == 'w' && isAlpha(text[text_idx]) || isDigit(text[text_idx]));

        if(!flag)return false;// not matched
        return match_here(pattern, pattern_idx+2, text, text_idx+1);// consume both '\' + meta
    }

    if(text_idx < text.size() && // Handle normal or '.'character
    (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx]))
    return match_here(pattern, pattern_idx+1, text, text_idx+1);
}
bool match_pattern(const string& input_line, const string& pattern) {
   if(pattern.empty())
     return true;

   if(pattern[0] == '^')// Just match cuzz , ^ after this it should match -> anchored!
    return match_here(pattern , 1, input_line, 0);

    for(int i=0;i<=input_line.size();i++){
        // We try to find a match starting at every possible position -> as unanchored!
        if(match_here(pattern, 1,input_line,0))return true;
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