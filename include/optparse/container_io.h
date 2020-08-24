/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef OPTPARSE_CONTAINER_IO_H
#define OPTPARSE_CONTAINER_IO_H

// Copyright (c) 2020 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include <ostream>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace optparse {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Can't directly overload operator<< for standard containers as that may cause ODR violation if
// other translation units overload these as well. Overload operator<< for a wrapper instead.

template<class Sequence>
struct SequenceWrapper {
    Sequence* c;
    char beg, end, delimiter;
};

template<class Sequence>
inline constexpr SequenceWrapper<Sequence const> as_array(Sequence const& c) noexcept {
    return {&c, '[', ']', ','};
}

template<class Sequence>
inline constexpr SequenceWrapper<Sequence const> as_sequence(Sequence const& c, char beg, char end, char delimiter) noexcept {
    return {&c, beg, end, delimiter};
}

template<class Sequence>
std::ostream& operator<<(std::ostream& s, SequenceWrapper<Sequence const> p) {
    if(p.beg)
        s.put(p.beg);
    bool first = true;
    for(auto const& value : *p.c) {
        if(first)
            first = false;
        else
            s.put(p.delimiter);
        s << value;
    }
    if(p.end)
        s.put(p.end);
    return s;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace optparse

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTPARSE_CONTAINER_IO_H
