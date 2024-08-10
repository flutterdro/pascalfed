#include "dummy2.hpp"
#include <stdexcept>


void throws(int a) {
    if (a) {

    throw std::runtime_error{"hewwo"};
    }
}
