#include "dummy2.hpp"
#include <print>

int main(int argc, char* argv[]) {
    try {
        throws(argc);
    } catch(std::exception const& e) {
        std::print("{}", e.what());
    }
}
