#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <iostream>

int main() {
    boost::optional<int> opt;
    std::cout << opt << std::endl;
    return 0;
}
