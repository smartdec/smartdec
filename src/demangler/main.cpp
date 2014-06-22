/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#include <cstdlib>
#include <iostream>
#include <string> /* getline() */

#include <undname/undname.h>

const char *self = "demangler";

extern "C" {
    /* Provided by libiberty. */
    char *__cxa_demangle(const char *mangled_name, char *output_buffer, size_t *length, int *status);
}

void help() {
    std::cout << "Usage: " << self << std::endl;
    std::cout << std::endl;
    std::cout << "The program reads the standard input line by line and tries to demangle" << std::endl;
    std::cout << "each line as if it were a mangled symbol. The result of demangling is" << std::endl;
    std::cout << "printed to the standard output, also line by line. Currently supported" << std::endl;
    std::cout << "mangling schemes are GNU V3 (via __cxa_demangle from GCC's libiberty)" << std::endl;
    std::cout << "and MSVC (via Wine's undname function). If a line cannot be demangled," << std::endl;
    std::cout << "it is printed as is." << std::endl;
}

int main(int argc, char * /*argv*/ []) {
    if (argc > 1) {
        help();
        return 1;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
        int status;
        if (char *output = __cxa_demangle(line.c_str(), NULL, NULL, &status)) {
            std::cout << output << std::endl;
            free(output);
        } else if (char *output = __unDName(NULL, line.c_str(), 0, 0)) {
            std::cout << output << std::endl;
            free(output);
        } else {
            std::cout << line << std::endl;
        }
    }

    return 0;
}

/* vim:set et sts=4 sw=4: */
