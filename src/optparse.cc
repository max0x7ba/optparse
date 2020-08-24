/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#include "optparse/optparse.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <limits>

#include <getopt.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace optparse;

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class F>
inline auto parse(string_view s, F f) {
    char* end = 0;
    auto r = f(s.data(), &end);
    if(end == s.end())
        return r;
    throw std::bad_cast{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool optparse::optparse_from_str(string_view s, Type<bool>) {
    if(s.size() == 1) {
        if(std::strchr("0Nn", s[0]))
            return false;
        if(std::strchr("1Yy", s[0]))
            return true;
    }
    throw std::bad_cast{};
}

float optparse::optparse_from_str(string_view s, Type<float>) {
    return parse(s, [](char const* s, char** e) { return std::strtof(s, e); });
}

double optparse::optparse_from_str(string_view s, Type<double>) {
    return parse(s, [](char const* s, char** e) { return std::strtod(s, e); });
}

long double optparse::optparse_from_str(string_view s, Type<long double>) {
    return parse(s, [](char const* s, char** e) { return std::strtold(s, e); });
}

long optparse::optparse_from_str(string_view s, Type<long>) {
    return parse(s, [](char const* s, char** e) { return std::strtol(s, e, 0); });
}

long long optparse::optparse_from_str(string_view s, Type<unsigned long>) {
    return parse(s, [](char const* s, char** e) { return std::strtoul(s, e, 0); });
}

long long optparse::optparse_from_str(string_view s, Type<long long>) {
    return parse(s, [](char const* s, char** e) { return std::strtoll(s, e, 0); });
}

unsigned long long optparse::optparse_from_str(string_view s, Type<unsigned long long>) {
    return parse(s, [](char const* s, char** e) { return std::strtoull(s, e, 0); });
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Parser::Parser()
    : help_()
{
    this->option('h', "help", string_view{}, &help_, "Display this help.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PositionalArgs Parser::parse(int ac, char** av)
{
    // Prepare the arguments for getopt_long.
    auto option_count = options_.size();
    ::option options[option_count + 1];
    char optstring[1 + option_count * 3 + 1], *optstringp = optstring;
    *optstringp++ = ':';
    constexpr int SHORT_VALUES = std::numeric_limits<unsigned char>::max() + 1;
    int short_values[SHORT_VALUES] = {};
    for(size_t i = 0; i < option_count; ++i) {
        auto& from = options_[i];
        auto& to = options[i];
        to.name = from.long_name_.data();
        to.has_arg = 1 + from.optional_arg_;
        to.flag = nullptr;
        to.val = i + SHORT_VALUES;

        if(from.short_name_) {
            short_values[static_cast<unsigned>(from.short_name_)] = i + SHORT_VALUES;
            *optstringp++ = from.short_name_;
            *optstringp++ = ':';
            if(from.optional_arg_)
                *optstringp++ = ':';
        }
    }
    options[option_count] = {};
    *optstringp = 0;

    // Now loop over options with getopt_long.
    struct ResetOptind { ~ResetOptind() { ::optind = 0; } } reset_optind_on_return;
    ::opterr = 0;
    for(int c; -1 != (c = ::getopt_long(ac, av, optstring, options, nullptr));) {
        if(c < SHORT_VALUES) {
            switch(c) {
            case ':':
                throw std::runtime_error(std::string("--") + options[::optopt].name + ": an argument is required.");
            case '?':
                throw std::runtime_error(std::string(av[::optind - 1]) + ": unknown option.");
            }

            assert(static_cast<unsigned>(c) < SHORT_VALUES);
            c = short_values[c];
            assert(c >= SHORT_VALUES);
        }

        auto& o = options_[c - SHORT_VALUES];
        try {
            if(o.container_delimiter_) {
                string_view from(::optarg);
                for(auto cur = from.begin(); cur != from.end();) {
                    auto cur_end = std::find(cur, from.end(), o.container_delimiter_);
                    string_view element(cur, cur_end - cur);
                    o.from_str_(element, o.value_, &o.cleared_);
                    cur = cur_end + (cur_end != from.end());
                }
            }
            else {
                o.from_str_(o.optional_arg_ && !::optarg ? "1" : ::optarg, o.value_, &o.cleared_);
            }
        }
        catch(std::bad_cast&) {
            throw std::runtime_error(std::string("Option --") + o.long_name_.data() + ": invalid value " + ::optarg);
        }
    };

    return {const_cast<char const**>(av) + ::optind, const_cast<char const**>(av) + ac};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& Parser::help(std::ostream& out) const {
    auto text_len = [](Option const& o) {
        return o.long_name_.size() + o.metavar_.size() + !o.metavar_.empty();
    };

    auto longest = text_len(*std::max_element(
          options_.begin()
        , options_.end()
        , [&text_len](Option const& a, Option const& b) {
              return text_len(a) < text_len(b);
          }
        ));

    string_view const value_placeholder = "%value";
    for(auto const& option : options_) {
        out << "  ";
        if(option.short_name_)
            out << '-' << option.short_name_ << ',';
        else
            out << "   ";

        if(!option.long_name_.empty()) {
            out << " --" << option.long_name_;
            if(!option.metavar_.empty())
                out << '=' << option.metavar_;
        }
        else {
            out << "   ";
        }

        for(auto pad = longest - text_len(option); pad--;)
            out.put(' ');

        out << " : ";

        auto default_pos = option.help_.find(value_placeholder);
        if(default_pos != string_view::npos) {
            out.write(option.help_.data(), default_pos);
            option.to_ostream_(out, option.value_, option.container_delimiter_);
            auto default_end_pos = default_pos + value_placeholder.size();
            out.write(option.help_.data() + default_end_pos, option.help_.size() - default_end_pos);
        }
        else {
            out << option.help_;
        }

        out.put('\n');
    }

    return out;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
