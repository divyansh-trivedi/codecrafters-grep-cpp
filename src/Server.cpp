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

    // --- START OF MINIMAL CHANGE ---
    // New: Static map to store captured strings. It's global to the matching process.
    static unordered_map<int, string> captured_groups;
    // --- END OF MINIMAL CHANGE ---

private:
    std::string _pattern_str;
    const char* _pattern{nullptr};
    const char* _begin{nullptr};
    const char* _end{nullptr};

    // --- START OF MINIMAL CHANGE ---
    // New: Counter for capture groups during parsing
    int _capture_group_count = 0;
    // --- END OF MINIMAL CHANGE ---

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

// --- START OF MINIMAL CHANGE ---
// Initialize the static map for captured groups
unordered_map<int, string> RegParser::captured_groups;
// --- END OF MINIMAL CHANGE ---


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
        
        // --- START OF MINIMAL CHANGE ---
        // Handle backreferences first
        if (current.type == BACKREFERENCE) {
            if (captured_groups.count(current.capture_group_id)) {
                const string& captured = captured_groups.at(current.capture_group_id);
                if (strncmp(c, captured.c_str(), captured.length()) == 0) {
                    c += captured.length();
                    rIdx++;
                    continue; // Move to the next token
                }
            }
            return false; // Backreference not found or did not match
        }
        // --- END OF MINIMAL CHANGE ---

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

        // --- START OF MINIMAL CHANGE ---
        // Rewritten ALT logic to handle backtracking and capturing
        if (current.type == ALT) {
            const char* group_start = c;
            for (const auto& alt : current.alternatives) {
                const char* temp_c = c; // Reset text pointer for each alternative
                auto captures_backup = captured_groups; // Save state for backtracking

                if (match_from_position(&temp_c, alt, 0)) {
                    // Capture the matched text
                    captured_groups[current.capture_group_id] = string(group_start, temp_c - group_start);
                    
                    // Try to match the rest of the main pattern
                    if (match_from_position(&temp_c, regex, rIdx + 1)) {
                        *start_pos = temp_c;
                        return true;
                    }
                }
                
                // If we reach here, this alternative path failed. Restore state.
                captured_groups = captures_backup;
            }
            // If no alternative led to a full match, the whole group fails.
            return false;
        }
        // --- END OF MINIMAL CHANGE ---

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

bool RegParser::match_alt_one_or_more(const char** c, const std::vector<Re>& regex, int idx)
{
    const Re& altGp = regex[idx];
    const char* original_pos = *c;

    // Must match at least once
    bool matched_once = false;
    const char* first_match_end = original_pos;
    for(const auto& alt : altGp.alternatives) {
        const char* temp_pos = original_pos;
        if(match_from_position(&temp_pos, alt, 0) && temp_pos > original_pos) {
            first_match_end = temp_pos;
            matched_once = true;
            break;
        }
    }
    if (!matched_once) return false;

    const char* pos = first_match_end;
    while(true) {
        const char* current_match_end = pos;
        bool found_match = false;
        for (const auto& alt : altGp.alternatives) {
            const char* temp_pos = pos;
            if (match_from_position(&temp_pos, alt, 0) && temp_pos > pos) {
                current_match_end = temp_pos;
                found_match = true;
                break;
            }
        }

        if (!found_match) break;
        pos = current_match_end;
    }

    while (pos >= first_match_end) {
        const char* temp_pos = pos;
        if (match_from_position(&temp_pos, regex, idx + 1)) {
            *c = temp_pos;
            return true;
        }
        if (pos == first_match_end) break;
        pos--;
    }
     // Check if matching just once is enough
    const char* temp_pos_once = first_match_end;
    if (match_from_position(&temp_pos_once, regex, idx + 1)) {
        *c = temp_pos_once;
        return true;
    }
    
    return false;
}

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
    // Path 1: Match the group
    const Re& altGp = regex[idx];
    for (const auto& alt : altGp.alternatives)
    {
        const char* t = *c;
        if (match_from_position(&t, alt, 0))
        {
            if (match_from_position(&t, regex, idx + 1))
            {
                *c = t;
                return true;
            }
        }
    }
    // Path 2: Skip the group
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
        // --- START OF MINIMAL CHANGE ---
        } else if (isdigit(*_pattern)) {
            current = makeRe(BACKREFERENCE);
            current.capture_group_id = *_pattern - '0';
        // --- END OF MINIMAL CHANGE ---
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
    // --- START OF MINIMAL CHANGE ---
    altGp.capture_group_id = ++_capture_group_count;
    // --- END OF MINIMAL CHANGE ---
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
        
        // --- START OF MINIMAL CHANGE ---
        // Clear global captures before starting a new match
        RegParser::captured_groups.clear();
        // --- END OF MINIMAL CHANGE ---

        if (hasStartAnchor)
        {
            return RegParser::match_from_position(&c, rp.regex, 1);
        }
        else
        {
            while (*c != '\0')
            {
                const char* temp_c = c;
                if (RegParser::match_from_position(&temp_c, rp.regex, 0))
                {
                    return true;
                }
                c++;
            }
            const char* end_of_string = c;
            if (RegParser::match_from_position(&end_of_string, rp.regex, 0)) {
                return true;
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