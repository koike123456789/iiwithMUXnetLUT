#include "circuit.h"
#include "global.h"
#include "node.h"

int main(int argc , char* argv[]){
    if(argc != 3 && argc != 4){
        std::cout << "error ./a.out <input file> <output file> <num parameter>" << std::endl;
        return 0;
    }
    std::cout << "start!!" << std::endl;
    std::string infile = argv[1];
    std::string outfile = argv[2];
    int num_in_LUT = -1;
    if(argc == 4){
        num_in_LUT = std::stoi(argv[3]);
    }
    std::cout << "num para = " << num_in_LUT << std::endl;

    nodecircuit::Circuit circuit1;
    circuit1.ReadBlif(infile);
    circuit1.outfile = outfile;
    circuit1.genQBF_withMUX();
    // circuit1.WriteBlif(outfile);

    return 0 ; 
}