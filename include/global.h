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

using namespace std;

#define PrintVar(var) std::cout << #var << "=" << var << std::endl;


inline void ERR(std::string err_msg) {
  std::cerr << err_msg << std::endl;
}

inline void MSG(std::string err_msg) {
  std::cout << err_msg << std::endl;
}

#endif // _GLOBAL_H_INCLUDED
