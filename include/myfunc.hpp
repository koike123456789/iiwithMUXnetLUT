#ifndef _MYFUNC_H_INCLUDED
#define _MYFUNC_H_INCLUDED

#include <global.h>

class QBFinfo{
    private:
        boost::dynamic_bitset<> read_qbf_ans(std::string filename, bool fusetwostep);

        vector<int> vwhichlut;

    public:
        string separate_qbfans(std::string filename, long npara_ctl, long npara_lut, bool fusetwostep);
        int add_const_notselectffs();

};
// string separate_qbfans(std::string filename, long npara_ctl, long npara_lut, bool fusetwostep);
// boost::dynamic_bitset<> read_qbf_ans(std::string filename, bool fusetwostep);

#endif