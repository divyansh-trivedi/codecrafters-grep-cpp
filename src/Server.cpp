#include <iostream>
#include <string>
#include <cctype>

// Forward declaration
bool match_pattern(const std::string& input_line, const std::string& pattern, bool start_of_line = true);

// Helper: match group content recursively
bool match_group(const std::string& input_line, const std::string& group) {
    // Split alternations inside ( ... )
    size_t pos = 0, depth = 0;
    for (size_t i = 0; i < group.size(); i++) {
        if (group[i] == '(') depth++;
        else if (group[i] == ')') depth--;
        else if (group[i] == '|' && depth == 0) {
            std::string left = group.substr(0, i);
            std::string right = group.substr(i + 1);
            return match_pattern(input_line, left, true) || match_pattern(input_line, right, true);
        }
    }
    return match_pattern(input_line, group, true);
}

bool match_pattern(const std::string& input_line, const std::string& pattern, bool start_of_line) {
    if (pattern.empty()) return true;
    if (pattern == "$") return input_line.empty();
    if (input_line.empty() && !pattern.empty()) return false;

    // Handle groups: ( ... )
    if (pattern[0] == '(') {
        size_t close = 1, depth = 1;
        while (close < pattern.size() && depth > 0) {
            if (pattern[close] == '(') depth++;
            else if (pattern[close] == ')') depth--;
            close++;
        }
        std::string group = pattern.substr(1, close - 2); // inside ( ... )
        char quant = (close < pattern.size()) ? pattern[close] : '\0';

        if (quant == '+') {
            size_t i = 0;
            if (!match_group(input_line.substr(i), group)) return false;
            while (i < input_line.size() && match_group(input_line.substr(i), group)) {
                i++;
            }
            return match_pattern(input_line.substr(i), pattern.substr(close + 1), false);
        }
        else if (quant == '?') {
            return match_pattern(input_line, pattern.substr(close + 1), false) ||
                   (match_group(input_line, group) &&
                    match_pattern(input_line.substr(1), pattern.substr(close + 1), false));
        }
        else {
            if (match_group(input_line, group))
                return match_pattern(input_line.substr(1), pattern.substr(close), false);
            return false;
        }
    }

    // Handle .
    if (pattern[0] == '.') {
        return match_pattern(input_line.substr(1), pattern.substr(1), false);
    }

    // Handle \d
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\d") {
        if (isdigit(input_line[0])) {
            return match_pattern(input_line.substr(1), pattern.substr(2), false);
        }
        return false;
    }

    // Handle \w
    if (pattern.size() >= 2 && pattern.substr(0, 2) == "\\w") {
        if (isalnum(input_line[0])) {
            return match_pattern(input_line.substr(1), pattern.substr(2), false);
        }
        return false;
    }

    // Direct char match
    if (pattern[0] == input_line[0]) {
        return match_pattern(input_line.substr(1), pattern.substr(1), false);
    }

    if (start_of_line) return false;
    return match_pattern(input_line.substr(1), pattern, false);
}

bool match_patterns(const std::string& input_line, const std::string& pattern) {
    if (!pattern.empty() && pattern[0] == '^') {
        return match_pattern(input_line, pattern.substr(1), true);
    }
    return match_pattern(input_line, pattern, false);
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

    std::string input_line;
    std::getline(std::cin, input_line);

    try {
        bool result = match_patterns(input_line, pattern);
        return result ? 0 : 1;
    } catch (...) {
        return 1;
    }
}
