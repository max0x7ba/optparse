/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */

// Copyright (c) 2020 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include <optparse/optparse.h>
#include <iostream>

using optparse::string_view;

int main(int ac, char** av) {
    bool a1 = false;
    int a2 = 0;
    double a3 = 0;
    string_view a4 = "empty";
    std::vector<int> a5{1, 2, 3};

    optparse::Parser parser;
    parser
        .option('b', "bool", "BOOL",  &a1, "bool option, value is %value.")
        .option('i', "int", "INT", &a2, "int option, value is %value.")
        .option('d', "double", "DOUBLE", &a3, "double option, value is %value.")
        .option('s', "string", "STRING", &a4, "string option, value is %value.")
        .option('v', "vector", "LIST", optparse::split_comma(&a5), "a vector option, value is %value.")
        ;

    auto pos_args = parser.parse(ac, av);

    if(parser.help()) {
        // Output help with the current values.
        std::cout << parser;

        // Output positional arguments..
        for(auto arg : pos_args)
            std::cout << arg << ' ';

        std::cout << '\n';
    }
}
