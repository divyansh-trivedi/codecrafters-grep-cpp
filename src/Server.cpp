#include <iostream>
#include <regex>
#include <string>

int main(int argc, char* argv[]) {
    std::string pattern;
    bool use_extended = false;

    // Parse arguments (-E <pattern>)
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-E" && i + 1 < argc) {
            use_extended = true;
            pattern = argv[++i];
        }
    }

    // Read full stdin
    std::string input((std::istreambuf_iterator<char>(std::cin)),
                      std::istreambuf_iterator<char>());

    try {
        std::regex re(pattern, std::regex::ECMAScript);

        if (std::regex_match(input, re)) {
            return 0; // success
        } else {
            return 1; // no match
        }
    } catch (std::regex_error& e) {
        std::cerr << "Regex error: " << e.what() << std::endl;
        return 1;
    }
}
