#include "hash_functions.h"

size_t polynomial_rolling_hash::operator() (std::string const & str) const {

    // define variables
    size_t hash = 0;
    size_t p = 1;

    // for each character, peform the function
    for (size_t i = 0; i < str.size(); ++i) {
        int each = static_cast<int>(str.at(i));
        hash += each * p;
        p = (p * 19) % 3298534883309ul;
    }

    return hash;

}

size_t fnv1a_hash::operator() (std::string const & str) const {

    // define variables
    const size_t prime = 0x00000100000001B3;
    const size_t basis = 0xCBF29CE484222325;
    size_t hash = basis;

    // for each character, perform the function
    for (size_t i = 0; i < str.size(); ++i) {
        int each = static_cast<int>(str.at(i));
        hash = (hash ^ each) * prime;
    }

    return hash;

}
