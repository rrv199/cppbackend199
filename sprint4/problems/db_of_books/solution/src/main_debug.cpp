#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cerr << "Debug: program started with " << argc << " arguments" << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cerr << "  argv[" << i << "] = " << argv[i] << std::endl;
    }
    std::cerr << "Debug: exiting" << std::endl;
    return 0;
}
