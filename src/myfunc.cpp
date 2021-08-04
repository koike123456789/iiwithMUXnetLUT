#include "myfunc.hpp"

string QBFinfo::separate_qbfans(string filename, long npara_ctl, long npara_lut, bool fusetwostep){
    boost::dynamic_bitset<> qbfans = read_qbf_ans(filename,fusetwostep);
    if(!qbfans.any()){
        cout << "The problem is UNSAT" << endl;
        return "UNSAT";
    }else{
        cout << "================The problem is SAT=================" <<endl; 
    }
    boost::dynamic_bitset<> para_ctl, para_lut;
    PrintVar(qbfans);
    int lutsize = (int)log2(npara_lut);

    para_ctl = qbfans;
    para_ctl.resize(npara_ctl);
    PrintVar(para_ctl);
    para_lut = qbfans;
    para_lut = (para_lut >> npara_ctl);
    para_lut.resize(npara_lut);
    PrintVar(para_lut);
    cout << "number of 1s = " << para_lut.count() << endl;
    cout << "number of 0s = " << para_lut.flip().count() << endl;

    boost::dynamic_bitset<> tmp_ctl = para_ctl;
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

boost::dynamic_bitset<> QBFinfo::read_qbf_ans(string filename, bool fusetwostep){
    ifstream file(filename);
    string findword;
    if(fusetwostep){ findword = "parameters:";}
    else{ findword = "Parameters:";}
    // cout << "find word = " << findword << endl;
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
            if(str2 == findword){
                flag_para = true;
            }
        }
    }
    file.close();
    return ans;
}

int QBFinfo::add_const_notselectffs(){
    for(auto ff: vwhichlut){
        cout << " " << ff;
    }
    cout << endl;
    return 0;
}