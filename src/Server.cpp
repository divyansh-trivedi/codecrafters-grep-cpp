#include <iostream>
#include <string>
#include <cctype>

// Match any single character in a group
bool match_group(const std::string& input, const std::string& group) {
    for (char c : input) {
        if (group.find(c) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Recursive pattern matcher
bool match_pattern(const std::string& input, const std::string& pattern, bool start_of_line = true) {
    if (pattern.empty()) return true;
    if (pattern[0] == '$') return input.empty();
    if (input.empty()) return false;

    // Handle alternation ( )
    if (pattern[0] == '(' && pattern.find('|') != std::string::npos) {
        int close_paren = pattern.find(')');
        std::string inside = pattern.substr(1, close_paren - 1);

        size_t start = 0, pipe;
        while ((pipe = inside.find('|', start)) != std::string::npos) {
            std::string option = inside.substr(start, pipe - start);
            if (match_pattern(input, option + pattern.substr(close_paren + 1), start_of_line)) return true;
            start = pipe + 1;
        }
        std::string last_option = inside.substr(start);
        return match_pattern(input, last_option + pattern.substr(close_paren + 1), start_of_line);
    }

    // Handle +
    if (pattern.size() > 1 && pattern[1] == '+') {
        char target = pattern[0];
        size_t i = 0;
        while (i < input.size() && input[i] == target) i++;
        return i > 0 && match_pattern(input.substr(i), pattern.substr(2), false);
    }

    // Handle ?
    if (pattern.size() > 1 && pattern[1] == '?') {
        if (pattern[0] == input[0]) {
            return match_pattern(input.substr(1), pattern.substr(2), false) ||
                   match_pattern(input, pattern.substr(2), false);
        } else {
            return match_pattern(input, pattern.substr(2), false);
        }
    }

    // Handle .
    if (pattern[0] == '.') {
        return match_pattern(input.substr(1), pattern.substr(1), false);
    }

    // Handle \d
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\d") {
        if (isdigit(input[0])) return match_pattern(input.substr(1), pattern.substr(2), false);
        return match_pattern(input.substr(1), pattern, false);
    }

    // Handle \w
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\w") {
        if (isalnum(input[0])) return match_pattern(input.substr(1), pattern.substr(2), false);
        return match_pattern(input.substr(1), pattern, false);
    }

    // Handle character groups []
    if (pattern[0] == '[') {
        auto close = pattern.find(']');
        bool neg = pattern[1] == '^';
        std::string chars = neg ? pattern.substr(2, close - 2) : pattern.substr(1, close - 1);
        bool match = match_group(input.substr(0,1), chars);
        if ((neg && !match) || (!neg && match)) {
            return match_pattern(input.substr(1), pattern.substr(close + 1), false);
        } else return false;
    }

    // Match exact character
    if (pattern[0] == input[0]) return match_pattern(input.substr(1), pattern.substr(1), false);

    // If start of line, fail; else try next char
    if (start_of_line) return false;
    return match_pattern(input.substr(1), pattern, false);
}

// Wrapper for ^ support
bool match_patterns(const std::string& input, const std::string& pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        return match_pattern(input, pattern.substr(1), true);
    }
    return match_pattern(input, pattern, false);
}

int main(int argc, char* argv[]) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string flag = argv[1];
    std::string pattern = argv[2];

    if (flag != "-E") {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }

    std::string input;
    std::getline(std::cin, input);

    try {
        bool result = match_patterns(input, pattern);
        return result ? 0 : 1;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
