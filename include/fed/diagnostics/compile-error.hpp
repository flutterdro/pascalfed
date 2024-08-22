#ifndef FED_COMILATION_ERROR_HPP_
#define FED_COMILATION_ERROR_HPP_

#include "fed/representations/raw-source.hpp"

#include <string>

namespace fed {

class compilation_error {
public:
    virtual auto message() const 
        -> std::string = 0;
    virtual auto location() const
        -> source::location;
    virtual ~compilation_error() = default;

};


}


#endif
