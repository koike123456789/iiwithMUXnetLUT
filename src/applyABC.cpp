#include "applyABC.h"


string Abc_qbf(string filename){
    string cmd_abc = "abc -c \"read " + filename + ";";
    
    cmd_abc += "\"";
    PrintVar(cmd_abc);
    return filename;
}

