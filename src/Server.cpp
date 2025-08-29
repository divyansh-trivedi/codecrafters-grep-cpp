#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <algorithm> // For std::copy
#include <cstring>   // For strlen and strcpy
#include <unordered_map> // For backreferences

using namespace std;

// --- Start of RegParser.h content ---

typedef enum
{
    NONE,
    PLUS,
    STAR,
    MARK
} Quantifier;

typedef enum
{
    SINGLE_CHAR,
    DIGIT,
    ALPHANUM,
    START,
    END,
    LIST,
    ALT,
    BACKREFERENCE, // New type for \1, \2, etc.
    ETK, // Error Token
} RegType;

struct Re; // Forward declaration for recursive struct
struct Re
{
    RegType type;
    char* ccl; // Character class list or single char
    bool isNegative;
    Quantifier quantifier;
    std::vector<std::vector<Re>> alternatives;
    int capture_group_id = 0; // New: To identify capture groups and backreferences

    // Destructor to free allocated memory
    ~Re() {
        delete[] ccl;
    }

    // Default constructor
    Re() : type(ETK), ccl(nullptr), isNegative(false), quantifier(NONE) {}

    // Copy constructor
    Re(const Re& other) {
        type = other.type;
        isNegative = other.isNegative;
        quantifier = other.quantifier;
        alternatives = other.alternatives;
        capture_group_id = other.capture_group_id; // New
        if (other.ccl) {
            ccl = new char[strlen(other.ccl) + 1];
            strcpy(ccl, other.ccl);
        } else {
            ccl = nullptr;
        }
    }

    // Copy assignment operator
    Re& operator=(const Re& other) {
        if (this == &other) {
            return *this;
        }
        delete[] ccl;
        type = other.type;
        isNegative = other.isNegative;
        quantifier = other.quantifier;
        alternatives = other.alternatives;
        capture_group_id = other.capture_group_id; // New
        if (other.ccl) {
            ccl = new char[strlen(other.ccl) + 1];
            strcpy(ccl, other.ccl);
        } else {
            ccl = nullptr;
        }
        return *this;
    }
};

class RegParser
{
public:
    RegParser(const std::string& pattern)
    {
        _pattern_str = pattern;
        _pattern = _pattern_str.c_str();
        _begin = _pattern;
        _end = _begin + _pattern_str.size();
    };

    bool parse();
    Re makeRe(RegType type, char* ccl = nullptr, bool isNegative = false);

    static bool match_from_position(const char** start_pos, const std::vector<Re>& regex, int idx);
    static bool match_current(const char* c, const std::vector<Re>& regex, int idx);
    static bool match_one_or_more(const char** c, const std::vector<Re>& regex, int idx);
    static bool match_alt_one_or_more(const char** c, const std::vector<Re>& regex, int idx);
    static bool match_zero_or_one(const char** c, const std::vector<Re>& regex, int idx);
    static bool match_alt_zero_or_one(const char** c, const std::vector<Re>& regex, int idx);
    
    std::vector<Re> regex;

    static unordered_map<int, string> captured_groups;

private:
    std::string _pattern_str;
    const char* _pattern{nullptr};
    const char* _begin{nullptr};
    const char* _end{nullptr};

    int _capture_group_count = 0;

    bool isEof();
    bool consume();
    bool check(char c);
    bool match(char c);

    Re parseElement();
    Re parseCharacterClass();
    Re parseGroup();
    void applyQuantifiers(Re& element);
};

// --- End of RegParser.h content ---
// --- Start of RegParser.cpp content ---

unordered_map<int, string> RegParser::captured_groups;


bool RegParser::parse()
{
    while (!isEof())
    {
        Re element = parseElement();
        if (element.type == ETK)
            return false;
        regex.push_back(element);
    }
    return true;
}

Re RegParser::makeRe(RegType type, char* ccl, bool isNegative)
{
    Re current;
    current.type = type;
    current.ccl = ccl;
    current.isNegative = isNegative;
    current.quantifier = NONE;
    return current;
}

bool RegParser::match_current(const char* c, const std::vector<Re>& regex, int idx)
{
    if (*c == '\0') {
        return regex[idx].type == END;
    }

    const Re* current = &regex[idx];
    switch (current->type)
    {
    case DIGIT:
        return isdigit(*c);
    case ALPHANUM:
        return isalnum(*c) || *c == '_';
    case SINGLE_CHAR:
        return *c == *current->ccl || *current->ccl == '.';
    case LIST:
    {
        const char* p = current->ccl;
        bool found = false;
        while (*p != '\0')
        {
            if (*c == *p)
            {
                found = true;
                break;
            }
            ++p;
        }
        return current->isNegative ? !found : found;
    }
    case START:
        return true;
    case END:
        return *c == '\0';
    default:
        return false;
    }
}

bool RegParser::match_from_position(const char** start_pos, const std::vector<Re>& regex, int idx)
{
    const char* c = *start_pos;
    int rIdx = idx;
    int pattern_length = regex.size();

    while (rIdx < pattern_length)
    {
        const Re& current = regex[rIdx];
        
        if (current.type == BACKREFERENCE) {
            if (captured_groups.count(current.capture_group_id)) {
                const string& captured = captured_groups.at(current.capture_group_id);
                if (strncmp(c, captured.c_str(), captured.length()) == 0) {
                    c += captured.length();
                    rIdx++;
                    continue;
                }
            }
            return false;
        }

        if (current.quantifier == PLUS) {
            const char* temp_c = c;
            if (current.type == ALT) {
                if(match_alt_one_or_more(&temp_c, regex, rIdx)) {
                    *start_pos = temp_c;
                    return true;
                }
            } else {
                if(match_one_or_more(&temp_c, regex, rIdx)) {
                    *start_pos = temp_c;
                    return true;
                }
            }
            return false;
        }
        
        if (current.quantifier == MARK) {
            const char* temp_c = c;
            if (current.type == ALT) {
                if(match_alt_zero_or_one(&temp_c, regex, rIdx)) {
                    *start_pos = temp_c;
                    return true;
                }
            } else {
                if(match_zero_or_one(&temp_c, regex, rIdx)) {
                    *start_pos = temp_c;
                    return true;
                }
            }
            return false;
        }

        if (current.type == ALT) {
            const char* group_start = c;
            for (const auto& alt : current.alternatives) {
                const char* temp_c = c;
                auto captures_backup = captured_groups;

                if (match_from_position(&temp_c, alt, 0)) {
                    captured_groups[current.capture_group_id] = string(group_start, temp_c - group_start);
                    
                    if (match_from_position(&temp_c, regex, rIdx + 1)) {
                        *start_pos = temp_c;
                        return true;
                    }
                }
                
                captured_groups = captures_backup;
            }
            return false;
        }

        if (!match_current(c, regex, rIdx)) {
            return false;
        }

        if (current.type != START) {
            c++;
        }
        rIdx++;
    }

    *start_pos = c;
    return true;
}

bool RegParser::match_one_or_more(const char** c, const std::vector<Re>& regex, int idx)
{
    const char* t = *c;
    if (!match_current(t, regex, idx))
        return false;
    t++;

    while (*t != '\0' && match_current(t, regex, idx))
    {
        t++;
    }

    while (t >= *c)
    {
        const char* test_pos = t;
        if (match_from_position(&test_pos, regex, idx + 1))
        {
            *c = test_pos;
            return true;
        }
        if (t == *c) break;
        t--;
    }
    return false;
}

// =================================================================
// START OF THE CRITICAL FIX
// Replaced the complex iterative version with a clean recursive one
// that correctly handles backtracking and state restoration.
// =================================================================
bool RegParser::match_alt_one_or_more(const char** c, const std::vector<Re>& regex, int idx)
{
    const Re& altGp = regex[idx];
    const char* original_pos = *c;

    // Must match at least once. Try each alternative.
    for (const auto& alt : altGp.alternatives) {
        const char* temp_pos = original_pos;
        auto captures_backup = captured_groups; // Save state for this path

        if (match_from_position(&temp_pos, alt, 0) && temp_pos > original_pos) {
            // After one successful match, we have two choices for the `+`
            
            // Path A (Greedy): Try to match the whole `(...)+` again on the rest of the text.
            const char* greedy_pos = temp_pos;
            if (match_alt_one_or_more(&greedy_pos, regex, idx)) {
                *c = greedy_pos;
                return true;
            }

            // Path B (Backtrack): If greedy fails, try matching just the rest of the main pattern.
            const char* rest_pos = temp_pos;
            if (match_from_position(&rest_pos, regex, idx + 1)) {
                *c = rest_pos;
                return true;
            }
        }
        
        // If we reach here, this alternative failed for the first match, restore state.
        captured_groups = captures_backup;
    }

    return false;
}
// =================================================================
// END OF THE CRITICAL FIX
// =================================================================


bool RegParser::match_zero_or_one(const char** c, const std::vector<Re>& regex, int idx)
{
    const char* temp_pos = *c;
    if (match_current(temp_pos, regex, idx))
    {
        temp_pos++;
        if (match_from_position(&temp_pos, regex, idx + 1))
        {
            *c = temp_pos;
            return true;
        }
    }
    return match_from_position(c, regex, idx + 1);
}

bool RegParser::match_alt_zero_or_one(const char** c, const std::vector<Re>& regex, int idx)
{
    const char* original_pos = *c;
    const Re& altGp = regex[idx];

    // Path 1 (Match group): Try to match the group, then the rest.
    for (const auto& alt : altGp.alternatives)
    {
        const char* t = original_pos;
        auto captures_backup = captured_groups; // Save state
        if (match_from_position(&t, alt, 0))
        {
            captured_groups[altGp.capture_group_id] = string(original_pos, t - original_pos);
            if (match_from_position(&t, regex, idx + 1))
            {
                *c = t;
                return true;
            }
        }
        captured_groups = captures_backup; // Restore state if path failed
    }
    // Path 2 (Skip group): Try to match the rest of the pattern directly.
    return match_from_position(c, regex, idx + 1);
}


bool RegParser::isEof()
{
    return _pattern >= _end;
}

bool RegParser::consume()
{
    if (!isEof())
    {
        _pattern++;
        return true;
    }
    return false;
}

bool RegParser::check(char c)
{
    return !isEof() && *_pattern == c;
}

bool RegParser::match(char c)
{
    if (check(c))
        return consume();
    return false;
}

Re RegParser::parseElement()
{
    if (check('^')) {
        consume();
        return makeRe(START);
    }
    if (check('$')) {
        consume();
        return makeRe(END);
    }
    if (check('\\')) {
        consume();
        if (isEof()) return makeRe(ETK);
        Re current;
        if (check('d')) {
            current = makeRe(DIGIT);
        } else if (check('w')) {
            current = makeRe(ALPHANUM);
        } else if (isdigit(*_pattern)) {
            current = makeRe(BACKREFERENCE);
            current.capture_group_id = *_pattern - '0';
        } else {
            char* cstr = new char[2];
            cstr[0] = *_pattern;
            cstr[1] = '\0';
            current = makeRe(SINGLE_CHAR, cstr);
        }
        consume();
        applyQuantifiers(current);
        return current;
    }
    if (check('[')) {
        consume();
        return parseCharacterClass();
    }
    if (check('(')) {
        consume();
        return parseGroup();
    }
    if (!isEof()) {
        char* cstr = new char[2];
        cstr[0] = *_pattern;
        cstr[1] = '\0';
        consume();
        Re current = makeRe(SINGLE_CHAR, cstr);
        applyQuantifiers(current);
        return current;
    }
    return makeRe(ETK);
}

Re RegParser::parseCharacterClass()
{
    Re current = makeRe(LIST);
    std::string buffer;

    current.isNegative = match('^');
    while (!isEof() && !check(']'))
    {
        buffer.push_back(*_pattern);
        consume();
    }

    if (!match(']'))
    {
        return makeRe(ETK);
    }

    char* cstr = new char[buffer.size() + 1];
    std::copy(buffer.begin(), buffer.end(), cstr);
    cstr[buffer.size()] = '\0';
    current.ccl = cstr;
    applyQuantifiers(current);
    return current;
}

Re RegParser::parseGroup()
{
    Re altGp = makeRe(ALT);
    altGp.capture_group_id = ++_capture_group_count;
    std::vector<Re> current_sequence;
    bool isClosed = false;

    while (!isEof())
    {
        if (check('|')) {
            consume();
            altGp.alternatives.push_back(current_sequence);
            current_sequence.clear();
        } else if (check(')')) {
            consume();
            altGp.alternatives.push_back(current_sequence);
            isClosed = true;
            break;
        } else {
            Re element = parseElement();
            if (element.type == ETK) return makeRe(ETK);
            current_sequence.push_back(element);
        }
    }

    if (!isClosed) return makeRe(ETK);

    applyQuantifiers(altGp);
    return altGp;
}

void RegParser::applyQuantifiers(Re& element)
{
    if (!isEof()) {
        if (check('+')) {
            element.quantifier = PLUS;
            consume();
        } else if (check('?')) {
            element.quantifier = MARK;
            consume();
        }
    }
}

// --- End of RegParser.cpp content ---
// --- Start of Server.cpp content ---

#ifdef DEBUG
void printDebug(const std::vector<Re>& reList);
#endif

bool match_pattern(const std::string& input_line, const std::string& pattern)
{
    RegParser rp(pattern);

    if (rp.parse())
    {
#ifdef DEBUG
        std::cerr << "--- Parsed Regex Structure ---" << std::endl;
        printDebug(rp.regex);
        std::cerr << "----------------------------" << std::endl;
#endif
        const char* c = input_line.c_str();
        bool hasStartAnchor = !rp.regex.empty() && rp.regex[0].type == START;
        
        RegParser::captured_groups.clear();

        if (hasStartAnchor)
        {
            return RegParser::match_from_position(&c, rp.regex, 1);
        }
        else
        {
            while (true)
            {
                const char* temp_c = c;
                if (RegParser::match_from_position(&temp_c, rp.regex, 0))
                {
                    return true;
                }
                if (*c == '\0') break; // Break after trying the last empty string
                c++;
            }
            return false;
        }
    }
    else
    {
        throw std::runtime_error("Invalid pattern: " + pattern);
    }
}

int main(int argc, char* argv[])
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cerr << "Logs from your program will appear here" << std::endl;

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " -E \"<pattern>\"" << std::endl;
        return 1;
    }

    std::string flag = argv[1];
    std::string pattern = argv[2];

    if (flag != "-E")
    {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }

    std::string input_line;
    std::getline(std::cin, input_line);

    try
    {
        if (match_pattern(input_line, pattern))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

#ifdef DEBUG
void printQuantifier(const Re& re);

void printDebug(const std::vector<Re>& reList) {
    for (const auto& tmp : reList) {
        switch (tmp.type) {
            case DIGIT: std::cout << "DIGIT"; break;
            case ALPHANUM: std::cout << "ALPHANUM"; break;
            case SINGLE_CHAR: std::cout << "SINGLE CHAR" << " >> " << tmp.ccl; break;
            case ALT:
                std::cout << "ALT (Group " << tmp.capture_group_id << ")" << " >> " << std::endl;
                std::cout << "(" << std::endl;
                for (size_t i = 0; i < tmp.alternatives.size(); ++i) {
                    printDebug(tmp.alternatives[i]);
                    if (i < tmp.alternatives.size() - 1)
                        std::cout << "|" << std::endl;
                }
                std::cout << ")";
                break;
            case LIST: std::cout << (tmp.isNegative ? "NEGATIVE " : "") << "LIST" << " >> " << tmp.ccl; break;
            case START: std::cout << "START"; break;
            case END: std::cout << "END"; break;
            case BACKREFERENCE: std::cout << "BACKREFERENCE to group " << tmp.capture_group_id; break;
            case ETK: std::cout << "ETK"; break;
            default: std::cout << "UNKNOWN"; break;
        }
        printQuantifier(tmp);
        std::cout << std::endl;
    }
}
#endif

