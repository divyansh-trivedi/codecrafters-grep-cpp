#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <filesystem>
#include <fstream>

// Forward declaration for the main recursive matching function
bool match_here(const std::string& text, const std::string& regexp);

// Helper function to handle the '+' quantifier
bool match_plus(const std::string& text, char match_char, const std::string& regexp) {
    std::string remaining_text = text;
    while (!remaining_text.empty() && (remaining_text[0] == match_char || match_char == '.')) {
        if (match_here(remaining_text.substr(1), regexp)) {
            return true;
        }
        remaining_text = remaining_text.substr(1);
    }
    return false;
}

// Helper function to handle the '?' quantifier
bool match_question_mark(const std::string& text, char match_char, const std::string& regexp) {
    if (!text.empty() && (text[0] == match_char || match_char == '.')) {
        if (match_here(text.substr(1), regexp)) {
            return true;
        }
    }
    return match_here(text, regexp);
}

// The core recursive matching engine
bool match_here(const std::string& text, const std::string& regexp) {
    if (regexp.empty()) {
        return true;
    }
    if (regexp == "$") {
        return text.empty();
    }
    if (regexp.length() > 1 && regexp[1] == '?') {
        return match_question_mark(text, regexp[0], regexp.substr(2));
    }
    if (regexp.length() > 1 && regexp[1] == '+') {
        return !text.empty() && (text[0] == regexp[0] || regexp[0] == '.') && match_plus(text.substr(1), regexp[0], regexp.substr(2));
    }
    if (text.empty()) {
        return false;
    }
    if (regexp[0] == '.') {
        return match_here(text.substr(1), regexp.substr(1));
    }
    if (regexp.length() >= 2 && regexp.substr(0, 2) == "\\d") {
        if (isdigit(text[0])) {
            return match_here(text.substr(1), regexp.substr(2));
        }
        return false;
    }
    if (regexp.length() >= 2 && regexp.substr(0, 2) == "\\w") {
        if (isalnum(text[0])) {
            return match_here(text.substr(1), regexp.substr(2));
        }
        return false;
    }
    if (regexp[0] == '(' && regexp.back() == ')') {
        std::string content = regexp.substr(1, regexp.length() - 2);
        size_t pipe_pos = content.find('|');
        if (pipe_pos != std::string::npos) {
            std::string left = content.substr(0, pipe_pos);
            std::string right = content.substr(pipe_pos + 1);
            return match_here(text, left) || match_here(text, right);
        }
    }
    if (regexp[0] == text[0]) {
        return match_here(text.substr(1), regexp.substr(1));
    }
    return false;
}

// The main entry point for matching logic
bool match_pattern(const std::string& input_line, const std::string& pattern) {
    if (pattern.length() >= 2 && pattern.front() == '[' && pattern.back() == ']') {
        std::string content = pattern.substr(1, pattern.length() - 2);
        bool negated = false;
        if (!content.empty() && content[0] == '^') {
            negated = true;
            content = content.substr(1);
        }
        for (char c : input_line) {
            bool found = (content.find(c) != std::string::npos);
            if (negated && !found) return true;
            if (!negated && found) return true;
        }
        return false;
    }
    if (!pattern.empty() && pattern[0] == '^') {
        return match_here(input_line, pattern.substr(1));
    }
    std::string text = input_line;
    while (!text.empty()) {
        if (match_here(text, pattern)) {
            return true;
        }
        text = text.substr(1);
    }
    // Handle empty input matching empty pattern or specific patterns like '$'
    if (input_line.empty()) {
        return match_here("", pattern);
    }

    return false;
}

int main(int argc, char* argv[]) {
    // Ensure immediate output
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " -E <pattern> [file...]" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) != "-E") {
        std::cerr << "Expected first argument to be '-E'" << std::endl;
        return 1;
    }

    std::string pattern = argv[2];
    
    // If no files are provided, read from stdin
    if (argc == 3) {
        std::string input_line;
        if (std::getline(std::cin, input_line)) {
            if (match_pattern(input_line, pattern)) {
                return 0; // Match found
            }
        }
        return 1; // No match
    }

    // Process files
    bool any_match = false;
    for (int i = 3; i < argc; ++i) {
        std::ifstream file(argv[i]);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << argv[i] << std::endl;
            continue;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (match_pattern(line, pattern)) {
                any_match = true;
                if (argc > 4) { // Print filename if more than one file
                    std::cout << argv[i] << ":" << line << std::endl;
                } else {
                    std::cout << line << std::endl;
                }
            }
        }
    }

    return any_match ? 0 : 1;
}
