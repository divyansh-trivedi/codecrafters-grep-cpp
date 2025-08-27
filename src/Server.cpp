#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;

bool match_here(const string& pattern , int pattern_idx,const string& text, int text_idx){
    if (pattern_idx == pattern.size()) return true; // Matched

    if (pattern[pattern_idx] == '$' && pattern_idx == pattern.size()-1)
        return text_idx == text.size();

    // '+' quantifier
    if (pattern_idx+1 < pattern.size() && pattern[pattern_idx+1] == '+') {
        if (text_idx >= text.size() || 
            (pattern[pattern_idx] != '.' && pattern[pattern_idx] != text[text_idx]))
            return false;

        bool notpick = match_here(pattern , pattern_idx+2, text , text_idx+1);
        bool pick    = match_here(pattern , pattern_idx, text , text_idx+1);
        return pick || notpick;
    }

      // '?' quantifier
    if (pattern_idx+1 < pattern.size() && pattern[pattern_idx+1] == '?') {
        bool skip = match_here(pattern, pattern_idx+2, text, text_idx);
        bool take = (text_idx < text.size() &&
                    (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx])) &&
                    match_here(pattern, pattern_idx+2, text, text_idx+1);
        return skip || take;
    }

    // Handle alternation: (cat|dog)
    if(pattern[pattern_idx] == '('){
        int close = pattern.find(')' , pattern_idx);
        if(close == -1)return false;

        string inside = pattern.substr(pattern_idx+1, close-pattern_idx-1);

        int start=0;
        while(true){
            int bar = inside.find('|',start);
            string option = inside.substr(start,(bar == string::npos ? string::npos : bar-start));

            if(match_here(option,0,text, text_idx) && match_here(pattern, close+1, text, text_idx+option.size()))
            return true;

            if(bar == string::npos)break;
            start = bar+1;
        }
        return false;
    }
    
    // Character class [...]
    if (pattern[pattern_idx] == '[') {
        int end = pattern.find(']', pattern_idx);
        if (end == -1 || text_idx >= text.size()) return false;

        bool neg = (pattern[pattern_idx+1] == '^');
        string group = pattern.substr(pattern_idx + (neg ? 2 : 1), 
                                      end - pattern_idx - (neg ? 2 : 1));

        bool in_group = (group.find(text[text_idx]) != string::npos);
        if (neg == in_group) return false;

        return match_here(pattern, end+1, text, text_idx+1);
    }

    // Escaped meta chars \d, \w
    if (pattern[pattern_idx] == '\\' && pattern_idx+1 < pattern.size()) {
        if (text_idx >= text.size()) return false;

        char m = pattern[pattern_idx+1];
        bool flag = (m == 'd' && isdigit(text[text_idx])) ||
                    (m == 'w' && (isalnum(text[text_idx]) || 
                                  text[text_idx] == '_'));

        if (!flag) return false;
        return match_here(pattern, pattern_idx+2, text, text_idx+1);
    }

    // Normal char or '.'
    if (text_idx < text.size() && 
        (pattern[pattern_idx] == '.' || pattern[pattern_idx] == text[text_idx]))
        return match_here(pattern, pattern_idx+1, text, text_idx+1);

    return false; 
}

bool match_pattern(const string& input_line, const string& pattern) {
    if (pattern.empty()) return true;

    if (pattern[0] == '^')
        return match_here(pattern, 1, input_line, 0);

    for (int i=0; i <= (int)input_line.size(); i++) {
        if (match_here(pattern, 0, input_line, i)) return true;
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
        if (match_pattern(input_line, pattern)) return 0;
        else return 1;
    } catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
