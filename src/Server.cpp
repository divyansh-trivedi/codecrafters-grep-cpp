#include <iostream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

enum class Quantifier { NONE, PLUS, MARK };
enum class RegType { SINGLE_CHAR, DIGIT, ALPHANUM, START, END, LIST, ALT, ETK };

struct Re {
    RegType type = RegType::ETK;
    string ccl;
    bool isNegative = false;
    Quantifier quantifier = Quantifier::NONE;
    vector<vector<Re>> alternatives;
};

class RegParser {
public:
    RegParser(const string& pattern) : _pattern(pattern), _pos(0) {}

    bool parse() {
        while (!isEof()) {
            Re element = parseElement();
            if (element.type == RegType::ETK) return false;
            regex.push_back(element);
        }
        return true;
    }

    vector<Re> regex;

    // Recursive matching
    static bool match_current(char c, const Re& r) {
        switch (r.type) {
            case RegType::DIGIT: return isdigit(c);
            case RegType::ALPHANUM: return isalnum(c) || c == '_';
            case RegType::SINGLE_CHAR: return r.ccl == "." || c == r.ccl[0];
            case RegType::LIST:
                return r.isNegative ? r.ccl.find(c) == string::npos : r.ccl.find(c) != string::npos;
            case RegType::START: return true;
            case RegType::END: return c == '\0';
            default: return false;
        }
    }

    static bool match_from(const char** c, const vector<Re>& regex, size_t idx) {
        const char* pos = *c;
        size_t rIdx = idx;

        while (rIdx < regex.size()) {
            const Re& r = regex[rIdx];

            // Quantifiers
            if (r.quantifier == Quantifier::PLUS) {
                const char* t = pos;
                if (!match_current(*t, r)) return false;
                while (*t != '\0' && match_current(*t, r)) t++;
                while (t >= pos) {
                    const char* tmp = t;
                    if (match_from(&tmp, regex, rIdx + 1)) { *c = tmp; return true; }
                    if (t == pos) break;
                    t--;
                }
                return false;
            }
            if (r.quantifier == Quantifier::MARK) {
                const char* t = pos;
                if (match_current(*t, r)) {
                    const char* tmp = t + 1;
                    if (match_from(&tmp, regex, rIdx + 1)) { *c = tmp; return true; }
                }
                return match_from(&pos, regex, rIdx + 1);
            }

            // Alternation
            if (r.type == RegType::ALT) {
                for (const auto& alt : r.alternatives) {
                    const char* t = pos;
                    if (match_from(&t, alt, 0) && match_from(&t, regex, rIdx + 1)) {
                        *c = t;
                        return true;
                    }
                }
                return false;
            }

            // Single character
            if (!match_current(*pos, r)) return false;
            if (r.type != RegType::START) pos++;
            rIdx++;
        }

        *c = pos;
        return true;
    }

private:
    string _pattern;
    size_t _pos = 0;

    bool isEof() { return _pos >= _pattern.size(); }
    char current() { return isEof() ? '\0' : _pattern[_pos]; }
    void advance() { if(!isEof()) _pos++; }

    Re parseElement() {
        if (current() == '^') { advance(); return Re{RegType::START}; }
        if (current() == '$') { advance(); return Re{RegType::END}; }

        if (current() == '\\') {
            advance();
            if (current() == 'd') { advance(); return Re{RegType::DIGIT}; }
            if (current() == 'w') { advance(); return Re{RegType::ALPHANUM}; }
            char c = current(); advance(); Re r{RegType::SINGLE_CHAR, string(1, c)}; applyQuant(r); return r;
        }

        if (current() == '[') { return parseCharacterClass(); }
        if (current() == '(') { return parseGroup(); }

        // single char
        char c = current(); advance();
        Re r{RegType::SINGLE_CHAR, string(1,c)};
        applyQuant(r);
        return r;
    }

    Re parseCharacterClass() {
        advance(); // skip '['
        Re r{RegType::LIST};
        if (current() == '^') { r.isNegative = true; advance(); }
        string buffer;
        while (!isEof() && current() != ']') { buffer += current(); advance(); }
        if (current() == ']') advance();
        r.ccl = buffer;
        applyQuant(r);
        return r;
    }

    Re parseGroup() {
        advance(); // skip '('
        Re r{RegType::ALT};
        vector<Re> seq;
        while (!isEof()) {
            if (current() == '|') { advance(); r.alternatives.push_back(seq); seq.clear(); }
            else if (current() == ')') { advance(); r.alternatives.push_back(seq); break; }
            else seq.push_back(parseElement());
        }
        applyQuant(r);
        return r;
    }

    void applyQuant(Re& r) {
        if (!isEof() && (current() == '+' || current() == '?')) {
            r.quantifier = current() == '+' ? Quantifier::PLUS : Quantifier::MARK;
            advance();
        }
    }
};

// Main matching function
bool match_pattern(const string& input_line, const string& pattern) {
    RegParser rp(pattern);
    if (!rp.parse()) return false;

    const char* c = input_line.c_str();
    bool anchored = !rp.regex.empty() && rp.regex[0].type == RegType::START;
    if (anchored) return RegParser::match_from(&c, rp.regex, 1);

    while (*c != '\0') {
        const char* tmp = c;
        if (RegParser::match_from(&tmp, rp.regex, 0)) return true;
        c++;
    }
    const char* tmp = c;
    return RegParser::match_from(&tmp, rp.regex, 0);
}

int main(int argc, char* argv[]) {
    if (argc != 3 || string(argv[1]) != "-E") return 1;

    string input_line;
    getline(cin, input_line);
    string pattern = argv[2];

    return match_pattern(input_line, pattern) ? 0 : 1;
}
