#ifndef _APPLYABC_H_INCLUDED
#define _APPLYABC_H_INCLUDED

#include "global.h"

std::string Abc_qbf(std::string filename, long npara);
std::string separate_qbfans(std::string filename, long npara_ctl, long npara_lut);

boost::dynamic_bitset<> read_qbf_ans(std::string filename);
std::string apply_abcopt(std::string filename, int ntime = 10);



#endif