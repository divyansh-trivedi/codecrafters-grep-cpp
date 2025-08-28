// Pattern.h
#ifndef PATTERN_H
#define PATTERN_H

#include <string>

class Pattern {
public:
    Pattern(const std::string& pattern);
    bool matches(const std::string& text) const;

private:
    std::string compiled_pattern;
    bool match_here(const std::string& text, const std::string& regexp) const;
    bool match_plus(const std::string& text, char match_char, const std::string& regexp) const;
    bool match_question_mark(const std::string& text, char match_char, const std::string& regexp) const;
};

class PatternCompiler {
public:
    PatternCompiler(const std::string& pattern);
    Pattern compile();

private:
    std::string source_pattern;
};

#endif // PATTERN_H
```cpp
// Pattern.cpp
#include "Pattern.h"
#include <cctype>
#include <vector>

Pattern::Pattern(const std::string& pattern) : compiled_pattern(pattern) {}

bool Pattern::matches(const std::string& text) const {
    if (compiled_pattern.empty()) return true;
    if (compiled_pattern[0] == '^') {
        return match_here(text, compiled_pattern.substr(1));
    }
    std::string remaining_text = text;
    while (!remaining_text.empty()) {
        if (match_here(remaining_text, compiled_pattern)) {
            return true;
        }
        remaining_text = remaining_text.substr(1);
    }
    return match_here("", compiled_pattern);
}

bool Pattern::match_here(const std::string& text, const std::string& regexp) const {
    if (regexp.empty()) return true;
    if (regexp == "$") return text.empty();

    if (regexp.length() > 1) {
        if (regexp[1] == '?') {
            return match_question_mark(text, regexp[0], regexp.substr(2));
        }
        if (regexp[1] == '+') {
            return !text.empty() && (regexp[0] == '.' || text[0] == regexp[0]) &&
                   match_plus(text.substr(1), regexp[0], regexp.substr(2));
        }
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

    if (regexp.length() >= 2 && regexp.substr(0, 2) == "\\d") {
        return !text.empty() && isdigit(text[0]) && match_here(text.substr(1), regexp.substr(2));
    }

    if (regexp.length() >= 2 && regexp.substr(0, 2) == "\\w") {
        return !text.empty() && (isalnum(text[0]) || text[0] == '_') && match_here(text.substr(1), regexp.substr(2));
    }

    if (regexp[0] == '[') {
        size_t close_bracket = regexp.find(']');
        if (close_bracket != std::string::npos) {
            std::string content = regexp.substr(1, close_bracket - 1);
            bool negated = !content.empty() && content[0] == '^';
            if (negated) {
                content = content.substr(1);
            }
            bool found = !text.empty() && content.find(text[0]) != std::string::npos;
            if (negated ? !found : found) {
                return match_here(text.substr(1), regexp.substr(close_bracket + 1));
            }
        }
        return false;
    }

    if (!text.empty() && (regexp[0] == '.' || regexp[0] == text[0])) {
        return match_here(text.substr(1), regexp.substr(1));
    }

    return false;
}

bool Pattern::match_plus(const std::string& text, char match_char, const std::string& regexp) const {
    std::string remaining_text = text;
    while (!remaining_text.empty() && (remaining_text[0] == match_char || match_char == '.')) {
        if (match_here(remaining_text.substr(1), regexp)) {
            return true;
        }
        remaining_text = remaining_text.substr(1);
    }
    return false;
}

bool Pattern::match_question_mark(const std::string& text, char match_char, const std::string& regexp) const {
    if (!text.empty() && (text[0] == match_char || match_char == '.')) {
        if (match_here(text.substr(1), regexp)) {
            return true;
        }
    }
    return match_here(text, regexp);
}

PatternCompiler::PatternCompiler(const std::string& pattern) : source_pattern(pattern) {}

Pattern PatternCompiler::compile() {
    return Pattern(source_pattern);
}
```cpp
// Main.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include "Pattern.h"

namespace fs = std::filesystem;

void searchTextFiles(const Pattern& pattern, const std::vector<std::string>& filepaths);
void handleRecursive(const Pattern& pattern, const std::string& filepath);

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " -E \"pattern\" [file...]" << std::endl;
        return 1;
    }

    std::vector<std::string> args(argv, argv + argc);

    if (args.size() == 3 && args[1] == "-E") {
        Pattern pattern = PatternCompiler(args[2]).compile();
        std::string inputLine;
        if (std::getline(std::cin, inputLine)) {
            if (pattern.matches(inputLine)) {
                return 0;
            }
        }
        return 1;
    } else {
        std::string firstFlag = args[1];
        if (firstFlag == "-E") {
            Pattern pattern = PatternCompiler(args[2]).compile();
            std::vector<std::string> filepaths(args.begin() + 3, args.end());
            searchTextFiles(pattern, filepaths);
        } else if (firstFlag == "-r") {
            if (args.size() < 4 || args[2] != "-E") {
                 std::cerr << "Usage: " << argv[0] << " -r -E \"pattern\" <directory>" << std::endl;
                 return 1;
            }
            Pattern pattern = PatternCompiler(args[3]).compile();
            handleRecursive(pattern, args[4]);
        }
    }

    return 0;
}

void searchTextFiles(const Pattern& pattern, const std::vector<std::string>& filepaths) {
    bool foundOne = false;
    for (const auto& filepath : filepaths) {
        std::ifstream file(filepath);
        if (file.is_open()) {
            std::string inputLine;
            while (std::getline(file, inputLine)) {
                if (pattern.matches(inputLine)) {
                    foundOne = true;
                    if (filepaths.size() > 1) {
                        std::cout << filepath << ":" << inputLine << std::endl;
                    } else {
                        std::cout << inputLine << std::endl;
                    }
                }
            }
        } else {
            std::cerr << "Error: could not open file: " << filepath << std::endl;
        }
    }
    exit(foundOne ? 0 : 1);
}

void handleRecursive(const Pattern& pattern, const std::string& filepath) {
    fs::path startPath(filepath);
    if (!fs::exists(startPath)) {
        std::cerr << "Error: path does not exist: " << filepath << std::endl;
        exit(2);
    }

    bool foundOne = false;
    if (fs::is_directory(startPath)) {
        for (const auto& entry : fs::recursive_directory_iterator(startPath)) {
            if (fs::is_regular_file(entry)) {
                searchTextFiles(pattern, {entry.path().string()});
            }
        }
    } else {
        searchTextFiles(pattern, {filepath});
    }
}
