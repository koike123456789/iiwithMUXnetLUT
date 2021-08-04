#include "applyABC.h"

string myskabc = "/cad_linux/myskabc/abc";
string normalabc = "~/abc/abc/abc";

string Abc_qbf(string filename, long npara_ctl,long npara_lut, bool fverbose, bool fusetwostep){
    long npara_total = npara_ctl + npara_lut;
    string abc_log = "abclog";
    string cmd_abc = myskabc + " -c \"read " + filename + ";";
    cmd_abc += " strash; &get; &qbf ";
    if(fverbose){cmd_abc += "-v ";}
    if(fusetwostep){ cmd_abc += "-P " + to_string(npara_ctl) + " -Q " + to_string(npara_lut) + ";";}
    else{ cmd_abc += " -P " + to_string(npara_total) + ";";}

    cmd_abc += "\"";
    cmd_abc += " > " + abc_log;

    system(cmd_abc.c_str());
    return abc_log;
}

string apply_abcopt(string filename, int ntime){
    string total_cmd, prefix_cmd,body_cmd,suffix_cmd;
    prefix_cmd = "abc -c \"read " + filename + ";";
    suffix_cmd = "\"";
    body_cmd = " strash;";
    for(int i = 0 ; i < ntime; i++){
        body_cmd += " dc2;print_stats;";
    }
    string opt_filename = "opt_" + filename;
    body_cmd += " write " + opt_filename + ";";
    total_cmd = prefix_cmd + body_cmd + suffix_cmd;
    // cout << total_cmd << endl;
    system(total_cmd.c_str());

    return opt_filename;
}
