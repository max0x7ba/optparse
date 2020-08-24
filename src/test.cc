/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */

// Copyright (c) 2020 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#define BOOST_TEST_MODULE optparse
#include "boost/test/unit_test.hpp"

#include "optparse/optparse.h"

#include <iostream>
#include <string_view>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

using optparse::string_view;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(optional_bool) {
    bool a1 = false;
    bool a2 = false;
    optparse::Parser parser;
    parser
        .option('a', "bool1", "", &a1, "bool option")
        .option('b', "bool2", "", &a2, "bool option")
        ;
    {
        char const* av[] = {"test", "-a",  "-b1", nullptr};
        parser.parse(sizeof av / sizeof *av - 1, av);
        BOOST_CHECK_EQUAL(a1, true);
        BOOST_CHECK_EQUAL(a2, true);
    }

    a1 = false;
    a2 = false;
    {
        char const* av[] = {"test", "--bool1",  "--bool2", "1", nullptr};
        parser.parse(sizeof av / sizeof *av - 1, av);
        BOOST_CHECK_EQUAL(a1, true);
        BOOST_CHECK_EQUAL(a2, true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(container) {
    std::vector<int> v1;
    std::vector<string_view> v2;
    optparse::Parser parser;
    parser
        .option('a', "container1", "", optparse::split(&v1, ','), "no help.")
        .option('b', "container2", "", optparse::split_comma(&v2), "no help.")
        ;
    {
        char const* av[] = {"test", "-a", "1",  "-b", "abc", nullptr};
        parser.parse(sizeof av / sizeof *av - 1, av);
        BOOST_CHECK((v1 == std::vector<int>{1}));
        BOOST_CHECK((v2 == std::vector<string_view>{"abc"}));
    }

    {
        v1.clear();
        v2.clear();
        char const* av[] = {"test", "-a", "1,2,3",  "-b", "abc,d,,ef", nullptr};
        parser.parse(sizeof av / sizeof *av - 1, av);
        BOOST_CHECK((v1 == std::vector<int>{1,2,3}));
        BOOST_CHECK((v2 == std::vector<string_view>{"abc","d","","ef"}));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(basic) {
    bool a1 = false;
    int a2 = 0;
    double a3 = 0;
    string_view a4 = "empty";
    std::vector<int> a5{1,2,3};

    optparse::Parser parser;
    parser
        .option('b', "bool", "BOOL",  &a1, "bool option, default is %value.")
        .option('i', "int", "INT", &a2, "int option, default is %value.")
        .option('d', "double", "DOUBLE", &a3, "double option, default is %value.")
        .option('s', "string", "STRING", &a4, "string option, default is %value.")
        .option('v', "vector", "LIST", optparse::split_comma(&a5), "a vector option, default is %value.")
        ;
    std::cout << parser;

    {
        char const* av[] = {"test", "-b1", "-i2", "-d", "3", "-s", "abc", "pos1", nullptr};
        auto pos_args = parser.parse(sizeof av / sizeof *av - 1, av);
        BOOST_CHECK_EQUAL(a1, true);
        BOOST_CHECK_EQUAL(a2, 2);
        BOOST_CHECK_EQUAL(a3, 3);
        BOOST_CHECK_EQUAL(a4, "abc");
        BOOST_CHECK_EQUAL(string_view("pos1"), pos_args.begin()[0]);
    }

    {
        a1 = false;
        a2 = 0;
        a3 = 0;
        a4 = {};
        char const* av[] = {"test", "--bool=1", "--int", "2", "--double", "3", "--string", "abc", nullptr};
        parser.parse(sizeof av / sizeof *av - 1, av);
        BOOST_CHECK_EQUAL(a1, true);
        BOOST_CHECK_EQUAL(a2, 2);
        BOOST_CHECK_EQUAL(a3, 3);
        BOOST_CHECK_EQUAL(a4, "abc");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(long_only) {
    int a1 = 0;
    int a2 = 0;
    optparse::Parser parser;
    parser
        .option("first", "",  &a1, "first option.")
        .option("second", "", &a2, "second option.")
        ;
    char const* av[] = {"test", "--first=1", "--second", "2", nullptr};
    parser.parse(sizeof av / sizeof *av - 1, av);
    BOOST_CHECK_EQUAL(a1, 1);
    BOOST_CHECK_EQUAL(a2, 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace

///////////////////////////////////////////////////////////////////////////////////////////////
