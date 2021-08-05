#include "myfunc.hpp"


int QBFinfo::main(string filename){
    this->npara_ctl = muxsize*lutsize;
    this->npara_lut = (1 << lutsize);
    bool fusetwostep = false, fverbose = false;
    qbfcircuit.ReadBlif(filename,false);
    qbfcircuit.print_circuitinfo();
    string tmpfile = "tmp.blif";
    
    string qbflogfile = Abc_qbf(filename,npara_ctl,npara_lut,fverbose,fusetwostep);
    
    while(separate_qbfans(qbflogfile,npara_ctl,npara_lut,fusetwostep)){
        print_status();
        // add_const_notselectffs(qbfcircuit,set_whichff.back());
        add_const_notselectffs(qbfcircuit,vsolutions.back().whichffs);
        qbfcircuit.print_circuitinfo();
        qbfcircuit.WriteBlif(tmpfile);
        qbflogfile = Abc_qbf(tmpfile,npara_ctl,npara_lut,fverbose,fusetwostep);

        if(vsolutions.size() >= 100){break;}
    }

// sort each solutions
    sort(vsolutions.begin(),vsolutions.end(),&compare_solution);
    print_status();

// get each FF's score
    ff2scores = get_eachFF_score(vsolutions);
    for(auto itr = ff2scores.begin(); itr != ff2scores.end(); itr++){
        cout << itr->first << " : " << itr->second << endl;
    }

//  get each solutions' score
    for(auto& solution : vsolutions){
        for(const auto& ff : solution.whichffs){
            solution.score = solution.score + ff2scores[ff];
        }
    }
    sort(vsolutions.begin(),vsolutions.end(),&compare_solution_score);
    print_status();

    return 0;
}

bool QBFinfo::separate_qbfans(string filename, long npara_ctl, long npara_lut, bool fusetwostep){
    boost::dynamic_bitset<> qbfans = read_qbf_ans(filename,fusetwostep);
    set<int> setwhichff;
    vector<int> vwhichff;

    if(!qbfans.any()){
        cout << "The problem is UNSAT" << endl;
        return false;
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
    cout << "number of 0s = " << npara_lut - para_lut.count() << endl;

    boost::dynamic_bitset<> tmp_ctl = para_ctl;
    for(int i = 0; i < lutsize; i++){
        // cout << para_ctl << " ";
        // cout << para_ctl.find_first() << endl;
        vwhichff.push_back(para_ctl.find_first());
        setwhichff.insert(para_ctl.find_first());
        para_ctl = (para_ctl >> (npara_ctl/lutsize));
    }
    cout << "selected FFs:" << endl;
    for(auto ilut : vwhichff){
        cout << ilut << " ";
    }
    cout << endl << "===================================================" << endl;
    solutioninfo sol = {para_lut.count(),setwhichff,0};
    vsolutions.push_back(sol);
    // set_whichff.push_back(setwhichlut);

    return true;
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

int QBFinfo::add_const_notselectffs(nodecircuit::Circuit& circuit,set<int> whichff){
    assert((unsigned int)lutsize == whichff.size());
    assert(circuit.outputs.size() == 1);
    string suffix = "_" + to_string(vsolutions.size());
    cout << "# Add constraint to the circuit not to select founded FFs" << endl;
    vector<NodeVector> allpat_innode = collect_allnodepat(circuit,whichff);

    // print all pattern
    for(auto innode : allpat_innode){
        cout << "Allpat : " ;
        for(auto node : innode){
            cout << " " << node->name;
        }
        cout << endl;
    }

    // create new OR gate in each allpat node
    NodeVector vCTLORnode;
    int n = 0;
    for(auto vinnode : allpat_innode){
        Node* nodeOR = new BlifNode;
        nodeOR->name = "const_OR" + to_string(n) + suffix;
        nodeOR->type = NODE_OR;
        for(auto innode : vinnode){
            nodeOR->inputs.push_back(innode);
        }
        vCTLORnode.push_back(nodeOR);
        circuit.all_nodes.push_back(nodeOR);
        n++;
    }

    //create new NAND gate in all CTLOR gate
    Node* node_notselectFFs = new BlifNode;
    circuit.all_nodes.push_back(node_notselectFFs);
    node_notselectFFs->name = "NotselectFFs" + suffix;
    node_notselectFFs->type = NODE_NAND;
    for(auto node : vCTLORnode){
        node_notselectFFs->inputs.push_back(node);
    }

    // create new OUT
    Node* oldout = qbfcircuit.outputs[0];
    oldout->is_output = false;
    circuit.outputs.erase(qbfcircuit.outputs.begin());
    Node* newout = new BlifNode;
    newout->is_output = true;
    newout->name = "newout" + suffix;
    newout->type = nodecircuit::NODE_AND;
    newout->inputs.push_back(oldout);
    newout->inputs.push_back(node_notselectFFs);

    circuit.outputs.push_back(newout);
    circuit.all_nodes.push_back(newout);
    return 0;
}

vector<NodeVector> QBFinfo::collect_allnodepat(const Circuit& circuit,set<int> whichff){
    vector<NodeVector> allnodepat;
    for(int mux = 0; mux < lutsize; mux++){
        NodeVector innode;
        for(auto ff: whichff){
            const int index = ff + mux*muxsize;
            innode.push_back(circuit.inputs[index]);
        }
        allnodepat.push_back(innode);
    }
    return allnodepat;
}

int QBFinfo::print_status(){
    cout << "============================ current status =============================" << endl;
    cout << "| MUX size = " << muxsize << endl;
    cout << "| LUT size = " << lutsize << endl;
    cout << "| Number of FOUNDED solutions = " << vsolutions.size() << endl;
    for(unsigned int i = 0; i < vsolutions.size(); i++){
        cout << "  |" << i+1 << ":";
        for(auto ff : vsolutions[i].whichffs){
            cout << " " << ff;
        }
        cout << " ( 1s = " << vsolutions[i].howmany1s << ", 0s = " << npara_lut-vsolutions[i].howmany1s << ")";
        cout << " score = " << vsolutions[i].score;
        cout << endl;
    }
    cout << "=========================================================================" << endl;
    return 0;
}

map<int,int> QBFinfo::get_eachFF_score(vector<solutioninfo> vsolutions){
    map<int,int> FFscores;
    for(unsigned int i = 0; i < vsolutions.size(); i++){
        for(const auto& ff : vsolutions[i].whichffs){
            auto itr = FFscores.find(ff);
            if(itr == FFscores.end()){
                FFscores[ff] = npara_lut - vsolutions[i].howmany1s;
            }else{
                itr->second += npara_lut - vsolutions[i].howmany1s;
            }
        }
    }
    return FFscores;
}

int QBFinfo::setsize(int lutsize, int muxsize){
    this->lutsize = lutsize;
    this->muxsize = muxsize;

    return 0;
}

bool compare_solution(const solutioninfo& a, const solutioninfo& b){
    set<int> ffsa = a.whichffs;
    set<int> ffsb = b.whichffs;
    auto itra = ffsa.begin();
    auto itrb = ffsb.begin();
    while(itra != ffsa.end()){
        if(*(itra) != *(itrb)){
            return *(itra) < *(itrb);
        }else{
            itra++;
            itrb++;
        }
    }
    return false;
}

bool compare_solution_score(const solutioninfo& a, const solutioninfo& b){
    return a.score < b.score;
}
