[![C/C++ CI](https://github.com/max0x7ba/optparse/workflows/C/C++%20CI/badge.svg)](https://github.com/max0x7ba/optparse/actions?query=workflow%3A%22C%2FC%2B%2B+CI%22)

# optparse
C++17 command line parsing inspired by Python optparse library. It uses GNU `getopt_long` for actuall command line parsing.

# Usage example

## Code
```
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
```

## Build
```
$ git clone https://github.com/max0x7ba/optparse.git
$ make -rC optparse -j8
```

## Run

```
$ ./optparse/build/release/gcc/example -b --int 1 --double 3.14 --string abc --vector 3,2,1 -h hello optparse
  -h, --help          : Display this help.
  -b, --bool=BOOL     : bool option, value is 1.
  -i, --int=INT       : int option, value is 1.
  -d, --double=DOUBLE : double option, value is 3.14.
  -s, --string=STRING : string option, value is abc.
  -v, --vector=LIST   : a vector option, value is 1.
hello optparse
```

---

Copyright (c) 2020 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.
