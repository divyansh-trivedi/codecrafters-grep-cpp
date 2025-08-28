#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <limits>
#include <cstddef>
#include <vector>


bool digit_check(const std::string& s) {
    bool check_flag = false;
    for (char c : s) {
        if (std::isdigit(c)) {
            check_flag = true;
            break;
        }
    }
    return check_flag;
}

bool word_check(const std::string& s) {
    bool check_flag = false;
    for (char c : s) {
        if (std::isalnum(c) || c == '_') {
            check_flag = true;
            break;
        }
    }
    return check_flag;
}
bool match_meta_positive_character_group(const std::string& s) {
    size_t first_delim = s.find("[");
    size_t second_delim = s.find("]");
    if (first_delim == std::string::npos || second_delim == std::string::npos) {
        return false;
    }
    int sniffer = static_cast<int>(second_delim - first_delim);
    if (sniffer > 2) {
        return true;
    } else {
        return false;
    }
}

std::vector<char> split_string_into_char(const std::string& s) {
    std::vector<char> split;
    for (char c : s) {
        split.push_back(c);
    }
    return split;
}

bool match_precise_positive_character_group(const std::string& s, const std::string& input_line) {
        size_t first_delim = s.find("[");
        size_t second_delim = s.find("]");
        std::string matching_string = s.substr(first_delim+1).substr(0, second_delim-1);
        std::cout << matching_string << std::endl;
        std::vector<char> matching_char_vector = split_string_into_char(matching_string);
        for (char c : matching_char_vector) {
            std::cout << c << std::endl;
            size_t matcher = input_line.find(c);
            if (matcher == std::string::npos) {
                return true;
            }
        }
        return false;
}

bool match_pattern(const std::string& input_line, const std::string& pattern) {
    std::cout << "match_meta: " << static_cast<int>(match_meta_positive_character_group(pattern)) << std::endl;
    if (pattern.length() == 1) {
        return input_line.find(pattern) != std::string::npos;
    } else if (pattern == "\\d") {
        bool digit_flag = digit_check(input_line);
        return digit_flag;
    } else if (pattern == "\\w") {
        bool word_flag = word_check(input_line);
        return word_flag;
    } else if (match_meta_positive_character_group(pattern) == true) {
        std::cout << "checking..." << std::endl;
        bool positive_charater_group_flag = match_precise_positive_character_group(pattern, input_line);
        std::cout << "precise: " << positive_charater_group_flag << std::endl;
        return positive_charater_group_flag;
    }
    else {
        throw std::runtime_error("Unhandled pattern " + pattern);
    }
}

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here" << std::endl;

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

    // Uncomment this block to pass the first stage
    //
    std::string input_line;
    std::getline(std::cin, input_line);
    //
    try {
       if (match_pattern(input_line, pattern)) {
            std::cout << "Result: 0";
            return 0;
        } else {
            std::cout << "Result: 1";
            return 1;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
}