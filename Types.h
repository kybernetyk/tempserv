#pragma once
#include <string>
#include "optional.hpp"

    template <class T> using Result = jsz::optional<T>;
    typedef Result<bool> status;

    struct Path {
    public:
        Path() : value("") {
            
        }
        Path(std::string s) : value(s) {
            
        }
        Path(const char *s) : value(s) {
            
        }
        
        std::string to_string() const {
            return value;
        }
        
        Path& operator+=(const Path& rhs) // compound assignment (does not need to be a member,
        {                           // but often is, to modify the private members)
            value += rhs.value;
            /* addition of rhs to *this takes place here */
            return *this; // return the result by reference
        }
        
        // friends defined inside class body are inline and are hidden from non-ADL lookup
        friend Path operator+(Path lhs,       // passing first arg by value helps optimize chained a+b+c
                           const Path& rhs) // alternatively, both parameters may be const references.
        {
            return lhs += rhs; // reuse compound assignment and return the result by value
        }
        
    protected:
        std::string value;
    };

