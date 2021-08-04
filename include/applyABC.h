#ifndef _APPLYABC_H_INCLUDED
#define _APPLYABC_H_INCLUDED

#include "global.h"

// #include <base/abc/abc.h>
// #include <base/main/main.h>
// #include <base/main/mainInt.h>

// using namespace ABC;

std::string Abc_qbf(std::string filename, long npara_ctl, long npara_lut, bool fverbose, bool fusetwostep);
std::string apply_abcopt(std::string filename, int ntime = 10);



#endif