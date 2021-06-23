#include "circuit.h"
#include "global.h"
#include "node.h"
#include "applyABC.h"

extern char* optarg;
extern int optind,opterr,optopt;

int main(int argc , char* argv[]){
    // make sub directory
    string cmd = "mkdir -p " + SUBDIR;
    system(cmd.c_str());

    int opt;
    bool fUseout = false;
    bool fverbose = false;
    bool fsolveqbf = false;
    int lutsize = 1;
    int muxsize = -1;
    int nopttime = 1;

    const char *optstring = "M:L:N:sovh";
    while((opt = getopt(argc,argv,optstring)) != -1){
        switch(opt){
            case 'h':
                print_usage(optstring);
                return 0;
            case 'o':
                fUseout = true;
                break;
            case 'v':
                fverbose = true;
                break;
            case 's':
                fsolveqbf = true;
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
    circuit1.ReadBlif(infile);
    circuit1.outfile = outfile;

    string opt_filename = circuit1.genQBF_withMUX(lutsize,muxsize,fUseout,fverbose);
    if(fsolveqbf){
        cout << "START solve QBF to find inductive invariant" << endl;
        long npara_ctl = circuit1.npara_ctl;
        long npara_lut = circuit1.npara_lut;
        long npara_total = npara_ctl + npara_lut;
        string qbflogfile = Abc_qbf(opt_filename,npara_total);
        separate_qbfans(qbflogfile,npara_ctl,npara_lut);
    }
    return 0 ; 
}


int print_usage(const char* optstring){
    cout << "USAGE: cad [" << optstring << "] " << "inputfile outputfile" << endl;
    cout << "   generates QBF miter command for computing an inductive invariant with MUX network." << endl;
    printf("    -L  : LUT size (default 1)\n");
    printf("    -M  : MUX size (default -1) (-1 = all FFs)\n");
    printf("    -N  : how many times optimize the circuit by dc2 command in ABC (default 1)\n");
    printf("    -s  : solve QBF miter after genqbf miter\n");
    printf("    -v  : print verboose outputs\n");
    printf("    -h  : print USAGE\n");
    return 0;
}