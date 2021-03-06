#ifndef _GLOBAL_H_INCLUDED
#define _GLOBAL_H_INCLUDED

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <iostream>
#include <fstream>

#include <chrono>
#include <iomanip>

#include <cassert>

#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <cmath>
#include <algorithm>
#include <filesystem>

using namespace std;

#define PrintVar(var) std::cout << #var << "=" << var << std::endl;
#define SUBDIR (string)"./tmpfile/"
#define MAX_MUXSIZE (500)

inline void ERR(std::string err_msg) {
  std::cerr << err_msg << std::endl;
}

inline void MSG(std::string err_msg) {
  std::cout << err_msg << std::endl;
}

int print_usage(const char* optstring);

#endif // _GLOBAL_H_INCLUDED