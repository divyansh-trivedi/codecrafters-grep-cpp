#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include "include/RegParser.h"

// When DEBUG is defined, the program will print the parsed structure of the regex.
// You can comment this out to disable the debug output.
#define DEBUG

#ifdef DEBUG
// Forward declaration for the recursive debug print function
void printDebug(const std::vector<Re>& reList);

// Helper to print the type of quantifier
void printQuantifier(const Re& re) {
    switch (re.quantifier) {
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

// Main function to print the parsed regex structure for debugging
void printDebug(const std::vector<Re>& reList) {
    for (const auto& tmp : reList) {
        switch (tmp.type) {
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
                for (size_t i = 0; i < tmp.alternatives.size(); ++i) {
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

// The main matching function that uses your RegParser class
bool match_pattern(const std::string& input_line, const std::string& pattern) {
    RegParser rp(pattern);

    // 1. Parse the pattern into an internal representation (AST)
    if (rp.parse()) {
#ifdef DEBUG
        std::cerr << "--- Parsed Regex Structure ---" << std::endl;
        printDebug(rp.regex);
        std::cerr << "----------------------------" << std::endl;
#endif
        const char* c = input_line.c_str();
        bool hasStartAnchor = !rp.regex.empty() && rp.regex[0].type == START;

        // 2. Perform the match based on the parsed structure
        if (hasStartAnchor) {
            // If anchored, attempt to match only from the beginning of the string.
            // We start matching from the token *after* the START anchor.
            return RegParser::match_from_position(&c, rp.regex, 1);
        } else {
            // If not anchored, loop through the input string, trying to match from each position.
            while (*c != '\0') {
                const char* temp_c = c; // Use a temporary pointer for each attempt
                if (RegParser::match_from_position(&temp_c, rp.regex, 0)) {
                    return true; // Match found
                }
                c++; // Move to the next character to try again
            }
             // Also check if the pattern can match an empty string at the end
            const char* end_of_string = c;
            if (RegParser::match_from_position(&end_of_string, rp.regex, 0)) {
                return true;
            }

            return false; // No match found after trying all positions
        }
    } else {
        // If parsing fails, the pattern is invalid.
        throw std::runtime_error("Invalid pattern: " + pattern);
    }
}

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr to ensure output is not buffered
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cerr << "Logs from your program will appear here" << std::endl;

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " -E \"<pattern>\"" << std::endl;
        return 1;
    }

    std::string flag = argv[1];
    std::string pattern = argv[2];

    if (flag != "-E") {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }

    std::string input_line;
    std::getline(std::cin, input_line);

    try {
        if (match_pattern(input_line, pattern)) {
            return 0; // Success (match found)
        } else {
            return 1; // Failure (no match)
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1; // Failure (error)
    }
}
