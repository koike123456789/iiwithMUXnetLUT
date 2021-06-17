#include "circuit.h"
#include "global.h"
#include "node.h"
#include "applyABC.h"


int main(int argc , char* argv[]){
    if(argc != 3 && argc != 4){
        std::cout << "error ./a.out <input file> <output file> <num parameter>" << std::endl;
        return 0;
    }
    std::cout << "start!!" << std::endl;
    std::string infile = argv[1];
    std::string outfile = argv[2];
    int lutsize = -1;
    if(argc == 4){
        lutsize = std::stoi(argv[3]);
    }
    nodecircuit::Circuit circuit1;
    circuit1.ReadBlif(infile);
    circuit1.outfile = outfile;


    string opt_filename = circuit1.genQBF_withMUX(lutsize);
    Abc_qbf(opt_filename);


    return 0 ; 
}