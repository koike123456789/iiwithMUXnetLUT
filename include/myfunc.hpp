#ifndef _MYFUNC_H_INCLUDED
#define _MYFUNC_H_INCLUDED

#include <map>

#include "global.h"
#include "circuit.h"
#include "node.h"

using namespace nodecircuit;

struct solutioninfo{
    long unsigned int howmany1s;
    set<int> whichffs;
    int score;
};

class QBFinfo{
    private:
        nodecircuit::Circuit qbfcircuit;
        long npara_ctl, npara_lut;
        int lutsize, muxsize;

        // vector<set<int>> set_whichff;
        vector<solutioninfo> vsolutions;
        map<int,int> ff2scores; 

        boost::dynamic_bitset<> read_qbf_ans(std::string filename, bool fusetwostep);
        int print_status();

        map<int,int> get_eachFF_score(vector<solutioninfo> vsolutions);



    public:
        bool separate_qbfans(std::string filename, long npara_ctl, long npara_lut, bool fusetwostep);
        int add_const_notselectffs(nodecircuit::Circuit& circuit,set<int> whichff);
        vector<NodeVector> collect_allnodepat(const Circuit& circuit,set<int> whichff);
        int setsize(int lutsize, int muxsize);
        int main(string filename);

};

bool compare_solution(const solutioninfo& a, const solutioninfo& b);
bool compare_solution_score(const solutioninfo& a, const solutioninfo& b);
#endif