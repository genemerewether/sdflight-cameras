#include <stdio.h>

#ifndef DEBUG_HPP
#define DEBUG_HPP

#define DEBUG_PRINT(x,...) printf(x,##__VA_ARGS__); fflush(stdout)
//#define DEBUG_PRINT(x,...)

#endif // DEBUG_HPP
