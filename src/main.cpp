#include "circuit.h"
#include "global.h"
#include "node.h"
#include "applyABC.h"

#include "myfunc.hpp"


int main(int argc , char* argv[]){
    // make sub directory
    string cmd = "mkdir -p " + SUBDIR;
    system(cmd.c_str());
    int opt;
    bool fUseout = false;
    bool fverbose = false;
    bool fsolveqbf = false;
    bool fusetwostep = false;
    int lutsize = 1;
    int muxsize = -1;
    int nopttime = 1;

    const char *optstring = "M:L:N:sovht";
    while((opt = getopt(argc,argv,optstring)) != -1){
        switch(opt){
            case 's':
                fsolveqbf = true;
                break;
            case 'o':
                fUseout = true;
                break;
            case 'v':
                fverbose = true;
                break;
            case 'h':
                print_usage(optstring);
                return 0;
            case 't':
                fusetwostep = true;
                break;
            case 'M':
                muxsize = stoi(optarg);
                break;
            case 'L':
                lutsize = stoi(optarg);
                break;
            case 'N':
                nopttime = stoi(optarg);
                break;
            default:
                print_usage(optstring);
                return -1;
        }
    }
    assert(argc == optind + 2);
    std::string infile = argv[optind];
    std::string outfile = argv[optind+1];
    nodecircuit::Circuit circuit1;
    circuit1.ReadBlif(infile,false);
    if(muxsize < 0){ muxsize = circuit1.ffs.size();}
    circuit1.outfile = outfile;

    string opt_filename = circuit1.genQBF_withMUX(lutsize,muxsize,fUseout,fverbose);
    if(fsolveqbf){
        // cout << "START solve QBF to find inductive invariant" << endl;
        // long npara_ctl = circuit1.npara_ctl;
        // long npara_lut = circuit1.npara_lut;
        // cout << "FINISH solve QBF to find inductive invariant" << endl;
        QBFinfo qbf_ii;
        qbf_ii.setsize(lutsize,muxsize);
        qbf_ii.main(opt_filename);
    }
    return 0 ; 
}


int print_usage(const char* optstring){
    cout << "USAGE: cad [" << optstring << "] " << "inputfile outputfile" << endl;
    cout << "   generates QBF miter command for computing an inductive invariant with MUX network." << endl;
    printf("    -L  : LUT size (default 1)\n");
    printf("    -M  : MUX size (default -1 : all FFs, maximum = %d)\n",MAX_MUXSIZE);
    printf("    -N  : how many times optimize the circuit by dc2 command in ABC (default 1)\n");
    printf("    -s  : solve QBF miter after genqbf miter\n");
    printf("    -t  : solve QBF miter by two step QBF solving \n");
    printf("    -o  : use last output (for property checking)\n");
    printf("    -v  : print verboose outputs\n");
    printf("    -h  : print USAGE\n");
    return 0;
}