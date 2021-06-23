#include "applyABC.h"


string Abc_qbf(string filename, long npara){
    string abc_log = "abclog";
    string cmd_abc = " abc -q \"read " + filename + ";";
    cmd_abc += " strash; &get;";
    cmd_abc += " &qbf -P " + to_string(npara) + ";";
    cmd_abc += "\"";

    string cmd_total = cmd_abc + " > " + abc_log;
    system(cmd_total.c_str());
    return abc_log;
}

string separate_qbfans(string filename, long npara_ctl, long npara_lut){
    boost::dynamic_bitset<> qbfans = read_qbf_ans(filename);
    if(!qbfans.any()){
        cout << "The problem is UNSAT" << endl;
        return "UNSAT";
    }else{
        cout << "================The problem is SAT=================" <<endl; 
    }
    boost::dynamic_bitset<> para_ctl, para_lut;
    PrintVar(qbfans);
    int lutsize = (int)log2(npara_lut);
    PrintVar(lutsize);

    para_ctl = qbfans;
    para_ctl.resize(npara_ctl);
    PrintVar(para_ctl);
    para_lut = qbfans;
    para_lut = (para_lut >> npara_ctl);
    para_lut.resize(npara_lut);
    PrintVar(para_lut);

    boost::dynamic_bitset<> tmp_ctl = para_ctl;
    vector<int> vwhichlut;
    for(int i = 0; i < lutsize; i++){
        // cout << para_ctl << " ";
        // cout << para_ctl.find_first() << endl;
        vwhichlut.push_back(para_ctl.find_first());
        para_ctl = (para_ctl >> (npara_ctl/lutsize));
    }
    // sort(vwhichlut.begin(),vwhichlut.end());
    cout << "selected FFs:" << endl;
    for(auto ilut : vwhichlut){
        cout << ilut << " ";
    }
    cout << endl << "===================================================" << endl;

    return filename;
}

boost::dynamic_bitset<> read_qbf_ans(string filename){
    ifstream file(filename);
    assert(file);

    boost::dynamic_bitset<> ans;
    string str;
    while(getline(file, str)){
        char delim = ' ';
        stringstream ss(str);
        string str2;
        bool flag_para = false;
        while(getline(ss,str2,delim)){
            if(flag_para){
                reverse(str2.begin() , str2.end());
                boost::dynamic_bitset<> temp(str2);
                ans = temp;
                break;
            }
            if(str2 == "Parameters:"){
                flag_para = true;
            }
        }
    }
    file.close();
    return ans;
}

string apply_abcopt(string filename, int ntime){
    string total_cmd, prefix_cmd,body_cmd,suffix_cmd;
    prefix_cmd = "abc -q \"read " + filename + ";";
    suffix_cmd = "\"";
    body_cmd = " strash;";
    for(int i = 0 ; i < ntime; i++){
        body_cmd += " dc2;";
    }
    string opt_filename = "opt_" + filename;
    body_cmd += " write " + opt_filename + ";";
    total_cmd = prefix_cmd + body_cmd + suffix_cmd;
    // cout << total_cmd << endl;
    system(total_cmd.c_str());

    return opt_filename;
}
