#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <algorithm> // For std::copy

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
};

class RegParser
{
public:
    RegParser(const std::string& pattern) : _pattern(pattern.c_str())
    {
        _begin = _pattern;
        _end = _begin + pattern.size();
    };

    bool parse();
    Re makeRe(RegType type, char* ccl = nullptr, bool isNegative = false);

    static bool match_current(const char* c, const std::vector<Re>& regex, int idx);
    static bool match_from_position(const char** start_pos, const std::vector<Re>& regex, int idx);
    static bool match_one_or_more(const char** c, const std::vector<Re>& regex, int idx);
    static bool match_alt_one_or_more(const char** c, const std::vector<Re>& regex, int idx);
    static bool match_zero_or_one(const char** c, const std::vector<Re>& regex, int idx);
    static bool match_alt_zero_or_one(const char** c, const std::vector<Re>& regex, int idx);
    
    std::vector<Re> regex;

private:
    const char* _pattern{nullptr};
    const char* _begin{nullptr};
    const char* _end{nullptr};

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

bool RegParser::parse()
{
    do
    {
        Re element = parseElement();
        if (element.type == ETK)
            return false;
        regex.push_back(element);
    } while (!isEof());
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
    const Re* current = &regex[idx];
    switch (current->type)
    {
    case DIGIT:
    {
        return isdigit(*c);
    }
    case ALPHANUM:
    {
        return isalnum(*c) || *c == '_';
    }
    case SINGLE_CHAR:
    {
        return *c == *current->ccl || *current->ccl == '.';
    }
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
    {
        return false;
    }
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
        bool matched = false;
        switch (current.quantifier)
        {
        case PLUS:
            if (current.type == ALT)
            {
                if (match_alt_one_or_more(&c, regex, rIdx))
                {
                    *start_pos = c;
                    return true;
                }
                else
                    return false;
            }
            else
            {
                if (match_one_or_more(&c, regex, rIdx))
                {
                    *start_pos = c;
                    return true;
                }
                else
                    return false;
            }
            break;
        case MARK:
            if (current.type == ALT)
            {
                if (match_alt_zero_or_one(&c, regex, rIdx))
                {
                    *start_pos = c;
                    return true;
                }
                else
                    return false;
            }
            else
            {
                if (match_zero_or_one(&c, regex, rIdx))
                {
                    *start_pos = c;
                    return true;
                }
                else
                    return false;
            }
            break;
        case NONE:
        default:
            if (current.type == ALT)
            {
                for (const auto& alt : current.alternatives)
                {
                    const char* temp_c = c;
                    if (match_from_position(&temp_c, alt, 0))
                    {
                        c = temp_c;
                        matched = true;
                        break;
                    }
                }
                if (!matched)
                {
                    c = *start_pos;
                    return false;
                }
            }
            else
            {
                if (RegParser::match_current(c, regex, rIdx))
                {
                    if (!(regex[rIdx].type == START || regex[rIdx].type == END))
                    {
                        ++c;
                    }
                    matched = true;
                }
                else
                {
                    return false;
                }
            }
            break;
        }
        if (matched)
            ++rIdx;
    }

    *start_pos = c;
    return (rIdx >= pattern_length) || (rIdx < pattern_length && regex[rIdx].type == END && *c == '\0');
}

bool RegParser::match_one_or_more(const char** c, const std::vector<Re>& regex, int idx)
{
    const char* t = *c;
    if (!match_current(t, regex, idx))
        return false;
    ++t;

    while (*t != '\0' && match_current(t, regex, idx))
    {
        ++t;
    }

    do
    {
        const char* test_pos = t;
        if (match_from_position(&test_pos, regex, idx + 1))
        {
            *c = test_pos;
            return true;
        }
        --t;
    } while (t >= *c); // Corrected boundary condition
    return false;
}

bool RegParser::match_alt_one_or_more(const char** c, const std::vector<Re>& regex, int idx)
{
    const Re& altGp = regex[idx];
    std::vector<const char*> match_ends;
    const char* pos = *c;

    while (*pos != '\0')
    {
        bool found_match = false;
        for (const auto& alt : altGp.alternatives)
        {
            const char* temp_pos = pos;
            if (match_from_position(&temp_pos, alt, 0))
            {
                if (temp_pos == pos) continue;
                match_ends.push_back(temp_pos);
                pos = temp_pos;
                found_match = true;
                break;
            }
        }
        if (!found_match) break;
    }

    if (match_ends.empty()) return false;

    for (int num_matches = match_ends.size() - 1; num_matches >= 0; num_matches--)
    {
        const char* t = match_ends[num_matches];
        if (match_from_position(&t, regex, idx + 1))
        {
            *c = t;
            return true;
        }
    }
    // Also check the case where the group matches but the rest of the pattern matches from the start
    const char* original_pos = *c;
    if (match_from_position(&original_pos, regex, idx + 1)) {
        *c = original_pos;
        return true;
    }


    return false;
}

bool RegParser::match_zero_or_one(const char** c, const std::vector<Re>& regex, int idx)
{
    if (match_current(*c, regex, idx))
    {
        const char* temp_pos = *c + 1;
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
    return match_from_position(c, regex, idx + 1);
}

bool RegParser::isEof()
{
    return *_pattern == '\0';
}

bool RegParser::consume()
{
    if (!isEof())
    {
        ++_pattern;
        return true;
    }
    return false;
}

bool RegParser::check(char c)
{
    return *_pattern == c;
}

bool RegParser::match(char c)
{
    if (check(c))
        return consume();
    return false;
}

Re RegParser::parseElement()
{
    if (match('^'))
    {
        return makeRe(START);
    }
    else if (match('$'))
        return makeRe(END);
    else if (match('\\'))
    {
        if (match('w')) {
            Re current = makeRe(ALPHANUM);
            applyQuantifiers(current);
            return current;
        }
        else if (match('d')) {
            Re current = makeRe(DIGIT);
            applyQuantifiers(current);
            return current;
        }
        else
        {
            // Handle other escaped characters as literals
            char* cstr = new char[2];
            cstr[0] = *_pattern;
            cstr[1] = '\0';
            consume();
            Re current = makeRe(SINGLE_CHAR, cstr);
            applyQuantifiers(current);
            return current;
        }
    }
    else if (match('['))
    {
        return parseCharacterClass();
    }
    else if (match('('))
    {
        return parseGroup();
    }
    else
    {
        if (isEof()) return makeRe(ETK);
        char* cstr = new char[2];
        cstr[0] = *_pattern;
        cstr[1] = '\0';
        consume();

        Re current = makeRe(SINGLE_CHAR, cstr);
        applyQuantifiers(current);
        return current;
    }
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
    std::vector<Re> current_sequence;
    bool isClosed = false;

    do
    {
        if (match('|'))
        {
            altGp.alternatives.push_back(current_sequence);
            current_sequence.clear();
        }
        else if (match(')'))
        {
            altGp.alternatives.push_back(current_sequence);
            isClosed = true;
            break;
        }
        else
        {
            if (isEof()) break;
            Re element = parseElement();
            if (element.type == ETK)
                return makeRe(ETK);
            current_sequence.push_back(element);
        }
    } while (!isEof());

    if (!isClosed)
        return makeRe(ETK);

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

void printQuantifier(const Re& re)
{
    switch (re.quantifier)
    {
    case PLUS:
        std::cout << "+";
        break;
    case MARK:
        std::cout << "?";
        break;
    case STAR:
        std::cout << "*";
        break;
    default:
        break;
    }
}

void printDebug(const std::vector<Re>& reList)
{
    for (const auto& tmp : reList)
    {
        switch (tmp.type)
        {
        case DIGIT:
            std::cout << "DIGIT";
            break;
        case ALPHANUM:
            std::cout << "ALPHANUM";
            break;
        case SINGLE_CHAR:
            std::cout << "SINGLE CHAR" << " >> " << tmp.ccl;
            break;
        case ALT:
            std::cout << "ALT" << " >> " << std::endl;
            std::cout << "(" << std::endl;
            for (size_t i = 0; i < tmp.alternatives.size(); ++i)
            {
                printDebug(tmp.alternatives[i]);
                if (i < tmp.alternatives.size() - 1)
                    std::cout << "|" << std::endl;
            }
            std::cout << ")";
            break;
        case LIST:
            std::cout << (tmp.isNegative ? "NEGATIVE " : "") << "LIST" << " >> " << tmp.ccl;
            break;
        case START:
            std::cout << "START";
            break;
        case END:
            std::cout << "END";
            break;
        case ETK:
            std::cout << "ETK";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
        }
        printQuantifier(tmp);
        std::cout << std::endl;
    }
}
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
            // Also check if the pattern can match an empty string at the end
            const char* end_of_string = c;
            if (RegParser::match_from_position(&end_of_string, rp.regex, 0)) {
                return true;
            }
            return false;
        }
    }
    else
    {
        throw std::runtime_error("Unhandled pattern " + pattern);
    }
}

int main(int argc, char* argv[])
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cerr << "Logs from your program will appear here" << std::endl;

    if (argc != 3)
    {
        std::cerr << "Expected two arguments" << std::endl;
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
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
