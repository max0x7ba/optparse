/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef OPTPARSE_OPTPARSE_H_INCLUDED
#define OPTPARSE_OPTPARSE_H_INCLUDED

// Copyright (c) 2020 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include "container_io.h"

#include <type_traits>
#include <ostream>
#include <cassert>
#include <iosfwd>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace optparse {

using std::string_view;

template<class T>
struct Type {};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Container>
struct Split {
    Container* container;
    char container_delimiter;
};

template<class Container>
inline Split<Container> split(Container* c, char container_delimiter);

template<class Container>
inline Split<Container> split_comma(Container* c);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PositionalArgs {
    char const **beg_, **end_;

    char const** begin() const noexcept;
    char const** end() const noexcept;
    bool empty() const noexcept;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Option {
private:
    char short_name_;
    char container_delimiter_;
    bool optional_arg_;
    bool cleared_ = false;
    string_view long_name_;
    string_view metavar_;
    string_view help_;

    typedef void(*ToOstream)(std::ostream&, void*, char);
    typedef void(*FromStr)(string_view, void*, bool*);
    FromStr from_str_;
    ToOstream to_ostream_;
    void* value_;

    constexpr Option(char short_name, string_view long_name, string_view metavar, string_view help,
           FromStr, ToOstream, void*,
           bool optional_arg, char container_delimiter) noexcept;

    friend class Parser;

public:
    // The short option name is optional. The long one is required.

    template<class T>
    constexpr Option(char short_name, string_view long_name, string_view metavar, T* value, string_view help) noexcept;

    template<class Container>
    constexpr Option(char short_name, string_view long_name, string_view metavar, Split<Container> value, string_view help) noexcept;

    template<class T>
    constexpr Option(string_view long_name, string_view metavar, T* value, string_view help) noexcept;

    template<class Container>
    constexpr Option(string_view long_name, string_view metavar, Split<Container> value, string_view help) noexcept;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Parser {
private:
    std::vector<Option> options_;
    bool help_;

public:
    Parser();

    template<class... Args>
    Parser& option(Args&&... args);

    Parser& options(std::initializer_list<Option>);

    PositionalArgs parse(int argc, char** argv);
    PositionalArgs parse(int argc, char const** argv);

    // Usage: if(parser.help()) std::cout << parser;
    bool help() const noexcept;
    std::ostream& help(std::ostream&) const;
};

std::ostream& operator<<(std::ostream&, Parser const&);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool optparse_from_str(string_view s, Type<bool>);
float optparse_from_str(string_view s, Type<float>);
double optparse_from_str(string_view s, Type<double>);
long double optparse_from_str(string_view s, Type<long double>);
long optparse_from_str(string_view s, Type<long>);
long long optparse_from_str(string_view s, Type<unsigned long>);
long long optparse_from_str(string_view s, Type<long long>);
unsigned long long optparse_from_str(string_view s, Type<unsigned long long>);

inline char optparse_from_str(string_view s, Type<char>) { return optparse_from_str(s, Type<long>{}); }
inline char optparse_from_str(string_view s, Type<signed char>) { return optparse_from_str(s, Type<long>{}); }
inline char optparse_from_str(string_view s, Type<unsigned char>) { return optparse_from_str(s, Type<unsigned long>{}); }
inline short optparse_from_str(string_view s, Type<short>) { return optparse_from_str(s, Type<long>{}); }
inline unsigned short optparse_from_str(string_view s, Type<unsigned short>) { return optparse_from_str(s, Type<unsigned long>{}); }
inline int optparse_from_str(string_view s, Type<int>) { return optparse_from_str(s, Type<long>{}); }
inline unsigned int optparse_from_str(string_view s, Type<unsigned int>) { return optparse_from_str(s, Type<unsigned long>{}); }
inline char const* optparse_from_str(string_view s, Type<char const*>) { return s.data(); }
inline string_view optparse_from_str(string_view s, Type<string_view>) { return s; }

template<class T>
inline T optparse_from_str(string_view s) {
    return optparse_from_str(s, Type<T>{});
}

// Overload this conversion if overloading std::ostream& operator<<(std::ostream&, T&) is not
// desirable.
template<class T>
inline std::ostream& optparse_to_ostream(std::ostream& s, T const& t, char /*container_delimiter*/) {
    return s << t;
}

template<class T>
inline std::ostream& optparse_to_ostream(std::ostream& s, std::vector<T> const& v, char container_delimiter) {
    return s << as_sequence(v, 0, 0, container_delimiter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline constexpr Option::Option(
      char short_name
    , string_view long_name
    , string_view metavar
    , string_view help
    , FromStr from_str
    , ToOstream to_ostream
    , void* value
    , bool optional_arg
    , char container_delimiter
    ) noexcept
    : short_name_(short_name)
    , container_delimiter_(container_delimiter)
    , optional_arg_(optional_arg)
    , long_name_(long_name)
    , metavar_(metavar)
    , help_(help)
    , from_str_(from_str)
    , to_ostream_(to_ostream)
    , value_(value)
{
    assert(!long_name_.empty()); // The short option name is optional. The long one is required.
}

template<class T>
inline constexpr Option::Option(char short_name, string_view long_name, string_view metavar, T* value, string_view help) noexcept
    : Option(
          short_name
        , long_name
        , metavar
        , help
        , [](string_view from, void* to, bool*) {
              *static_cast<T*>(to) = optparse_from_str<T>(from);
          }
        , [](std::ostream& to, void* from, char d) {
              optparse_to_ostream(to, *static_cast<T*>(from), d);
          }
        , value
        , std::is_same<T, bool>::value // The argument is optional for bool only.
        , 0
        )
{}

template<class T>
inline constexpr Option::Option(char short_name, string_view long_name, string_view metavar, Split<T> value, string_view help) noexcept
    : Option(
          short_name
        , long_name
        , metavar
        , help
        , [](string_view from, void* to, bool* cleared) {
              auto c = static_cast<T*>(to);
              if(!*cleared) {
                  *cleared = true;
                  c->clear();
              }
              c->push_back(optparse_from_str<typename T::value_type>(from));
          }
        , [](std::ostream& to, void* from, char d) {
              optparse_to_ostream(to, *static_cast<T*>(from), d);
          }
        , value.container
        , false
        , value.container_delimiter
        )
{}

template<class T>
inline constexpr Option::Option(string_view long_name, string_view metavar, T* value, string_view help) noexcept
    : Option('\0', long_name, metavar, value, help)
{}

template<class Container>
inline constexpr Option::Option(string_view long_name, string_view metavar, Split<Container> value, string_view help) noexcept
    : Option('\0', long_name, metavar, value, help)
{}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class... Args>
inline Parser& Parser::option(Args&&... args) {
    options_.emplace_back(std::forward<Args>(args)...);
    return *this;
}

inline Parser& Parser::options(std::initializer_list<Option> args) {
    options_.insert(options_.end(), args.begin(), args.end());
    return *this;
}

inline std::ostream& operator<<(std::ostream& s, Parser const& p) {
    return p.help(s);
}

inline PositionalArgs Parser::parse(int argc, char const** argv) {
    return this->parse(argc, const_cast<char**>(argv));
}

inline bool Parser::help() const noexcept {
    return help_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Container>
inline Split<Container> split(Container* c, char container_delimiter) {
    return {c, container_delimiter};
}

template<class Container>
inline Split<Container> split_comma(Container* c) {
    return {c, ','};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline char const** PositionalArgs::begin() const noexcept {
    return beg_;
}

inline char const** PositionalArgs::end() const noexcept {
    return end_;
}

inline bool PositionalArgs::empty() const noexcept {
    return beg_ == end_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // optparse

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTPARSE_OPTPARSE_H_INCLUDED
