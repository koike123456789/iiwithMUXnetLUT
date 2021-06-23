#include "circuit.h"
#include <queue>

using namespace std;

#include "blif_parser.h"

#define MAX_MUXSIZE (500)

namespace nodecircuit {
  string Circuit::genQBF_withMUX(int lutsize, int muxsize, bool fUseout, bool fverbose){
    if(muxsize < 0){muxsize = ffs.size();}

    for(int i = 0 ; i < ffs.size() ; i++){
      cout << ffs[i]->name << " " << ffs[i]->inputs[0]->type << endl;
    }

    if(lutsize > ffs.size()){ 
      ERR("ERR : lutsize exceeds number of ffs");
      lutsize = ffs.size();
      ERR("| lutsize is set to be " + to_string(lutsize) + "(maximum)");
      }
    if(muxsize > ffs.size() || lutsize > muxsize){
      ERR("ERR : muxsize exceeds number of ffs (or MAX_MUXSIZE) or less than lutsize");
      muxsize = (ffs.size() > MAX_MUXSIZE) ? MAX_MUXSIZE : ffs.size();
      ERR("| muxsize is set to be " + to_string(muxsize) + "(maximum)");
    }

    if(fverbose){
      print_circuitinfo();
      cout << "================== Each size  ==================" << endl;
      cout << "| muxsize = " << muxsize << endl;
      cout << "| lutsize = " << lutsize << endl;
      cout << "================================================" << endl;
    }
    cout << "================ QBF parameter ================" << endl;
    npara_ctl = muxsize*lutsize;
    npara_lut = (1 << lutsize);
    cout << "| control para : " << npara_ctl << endl;
    cout << "|     lut para : " << npara_lut << endl;
    cout << "|   total para : " << npara_ctl + npara_lut << endl;
    cout << "===============================================" << endl;

  // make each subckt
    string lut_file = "lut_size" + to_string(lutsize) + ".blif";
    string mux_file = "mux_size" + to_string(muxsize) + ".blif";
    string checkonehot_file = "checkonehot_size" + to_string(muxsize) + ".blif";
    string checkatmost1_file = "checkatmost1_size" + to_string(lutsize) + ".blif";
    vinputs_original_lut = make_LUT(lut_file,lutsize);
    vinputs_original_mux = make_MUX(mux_file,muxsize);
    vinputs_original_checkonehot = make_checkonehot(checkonehot_file,muxsize);
    vinputs_original_checkatmost1 = make_checkonehot(checkatmost1_file,lutsize,true);
  // make optimized circuit with abc
    string opt_lut_file = apply_abcopt(lut_file);
    string opt_mux_file = apply_abcopt(mux_file);
    string opt_checkonehot_file = apply_abcopt(checkonehot_file);
    string opt_checkatmost1_file = apply_abcopt(checkatmost1_file);
  // write genqbf with mux net blif file
    string main_file = name + "_main.blif"; 
    write_genqbfblif(main_file,lutsize,muxsize,fUseout);

  // integrate all file into one file 
    string cmd = "cat " + main_file + " " + opt_lut_file + " " + opt_mux_file  + " " + opt_checkonehot_file;
    cmd += " " + opt_checkatmost1_file + " > " + outfile;
    system(cmd.c_str());
    string opt_outfile = apply_abcopt(outfile,50);

  // arrangement of files
    string dir_arrange = "tmpfile";
    cmd = "mkdir -p " + dir_arrange;
    system(cmd.c_str());
    cmd = "mv " + lut_file + " " + opt_lut_file + " " + mux_file + " " + opt_mux_file;
    cmd += " " + checkonehot_file + " " + opt_checkonehot_file;
    cmd += " " + checkatmost1_file + " " + opt_checkatmost1_file;
    cmd += " " + main_file + " " + outfile;
    cmd += " ./" + dir_arrange; 
    system(cmd.c_str());

    cout << "finish genqbf blif file" << endl;
    return opt_outfile;
  }

  void Circuit::write_genqbfblif(string filename, int lutsize, int muxsize, bool fUseout){
    boost::dynamic_bitset<> bs(1);
  // check error 
    // if(fUseout){assert(outputs.size() == 1);}
    assert(muxsize >= lutsize);

    ofstream outf(filename);
    vector<vector<string>> vvinputs_mux;
    vector<string> vinputs_lut;
    outf << ".model " << name << "_genqbf" << lutsize << "withMUXnet" << endl;
  // make control val 
    for(int idmux = 0; idmux < lutsize; idmux++){
      outf << ".inputs";
      vector<string> vinputs_eachmux;
      for(int idctl = 0; idctl < muxsize; idctl++){
        string input = "ctl" + to_string(idmux) + "_" + to_string(idctl);
        outf << " " << input;
        vinputs_eachmux.push_back(input);
        }
        vvinputs_mux.push_back(vinputs_eachmux);
      outf << endl;
    }
  // make LUT val
    outf << ".inputs";
    for(int i = 0; i < (1 << lutsize); i++){
      string input = "lut" + to_string(i);
      outf << " " << input;
      vinputs_lut.push_back(input);
      }
    outf << endl;
  // orginal PI and LO
    outf  << ".inputs";
    for(auto pi : inputs){outf << " " << pi->name;}
    outf << endl;
    outf << ".inputs";
    for(auto lo : ffs){outf << " " << lo->name;}
    outf << endl;
  // new output
    outf << ".outputs out" << endl;

  // make subcircuit in LI or LO side
    vector<string> voutputs_mux_LOside,voutputs_mux_LIside;
    for(int LIside = 0; LIside < 2; LIside++){
      if(LIside == true){ outf << "#LI side subcircuit(LUT with MUXnet)" << endl;}
      else{outf << "#LO side subcircuit(LUT with MUXnet)" << endl;}
      for(int idmux = 0; idmux < vvinputs_mux.size(); idmux++){
        outf << ".subckt MUXnet";
        for(int idctl = 0; idctl < vvinputs_mux[0].size(); idctl++){
          outf << " " << vinputs_original_mux[idctl];
          outf << "=" << vvinputs_mux[idmux][idctl]; 
        }
        for(int idff = 0; idff < muxsize; idff++){
          outf << " " << vinputs_original_mux[muxsize + idff];
          if(LIside){outf << "=" << ffs[idff]->inputs[0]->name;}
          else{outf << "=" << ffs[idff]->name;}
        }

        if(LIside){
          string output = "outmux_LI" + to_string(idmux);
          outf << " out=" + output;
          voutputs_mux_LIside.push_back(output);
        }else{
          string output = "outmux_LO" + to_string(idmux);
          outf << " out=" + output;
          voutputs_mux_LOside.push_back(output);
        }
        outf << endl;
      }
    // make subcircuit of LUT
      outf << ".subckt LUT" << lutsize;
      for(int i = 0; i < lutsize; i++){
        if(LIside){
          outf << " " << vinputs_original_lut[i] << "=" << voutputs_mux_LIside[i];
        }else{
          outf << " " << vinputs_original_lut[i] << "=" << voutputs_mux_LOside[i];
        }
      }
      for(int i = 0; i < (1 << lutsize); i++ ){
        outf << " " << vinputs_original_lut[lutsize+i] << "=" << vinputs_lut[i];
      }
      if(LIside){outf << " out=outlut_LIside";}
      else{outf << " out=outlut_LOside";}
      outf << endl;
    }

  // make onehot constraint (ctl is onehot)
    vector<string> voutputs_onehot;
    outf << "#onehot constraint (ctl is onehot)" << endl;
    for(int idmux = 0; idmux < lutsize; idmux++){
      outf << ".subckt check_onehot";
      for(int idctl = 0; idctl < muxsize; idctl++){
        outf << " " << vinputs_original_checkonehot[idctl] << "=" << vvinputs_mux[idmux][idctl];
      }
      string output = "outonehot" + to_string(idmux);
      outf << " out=" << output << endl;
      voutputs_onehot.push_back(output); 
    }
  // make onehot constraint (not select the same ctl signal)
    vector<string> voutputs_atmost1;
    outf <<  "#at most 1 constraint (not select the same ffs)" << endl;
    for(int idctl = 0; idctl < muxsize; idctl++){
      outf << ".subckt check_atmost1";
      for(int idmux = 0; idmux < lutsize; idmux++){
        if(idmux < muxsize) {outf << " " << vinputs_original_checkonehot[idmux] << "=" << vvinputs_mux[idmux][idctl];}
        else{outf << " " << vinputs_original_checkonehot[idmux] << "=0";}
      }
      string output = "outatmost1_" + to_string(idctl);
      outf << " out=" << output << endl;
      voutputs_atmost1.push_back(output);
    }

  // make all constraint
    outf << endl << "#collect all constraint" << endl;
    outf << ".names";
    for(auto outonehot : voutputs_onehot){ outf << " " << outonehot;}
    outf << " cst_onehot" << endl;
    bs.resize(voutputs_onehot.size());
    bs.set();
    outf << bs << " 1" << endl;

    outf << ".names";
    for(auto vallut : vinputs_lut){ outf << " " << vallut;}
    outf << " cst_vallut" << endl;
    bs.resize(vinputs_lut.size());
    bs.set();
    outf << bs << " 0" << endl;
    outf << "0";
    for(int i = 0; i < vinputs_lut.size() - 1; i++){outf << "-";}
    outf << " 0" << endl;

    outf << ".names";
    for(auto outatmost1 : voutputs_atmost1){ outf << " " << outatmost1;}
    outf << " cst_atmost1" << endl;
    bs.resize(voutputs_atmost1.size());
    bs.set();
    outf << bs << " 1" << endl;

    if(!fUseout){
      outf << ".names outlut_LIside outlut_LOside cst_ii" << endl;
      outf << "01 0" << endl;
    }else{
      string out_property = outputs[0]->name;
      outf << ".names outlut_LIside outlut_LOside " << out_property; 
      outf << " cst_ii" << endl;
      outf << "01- 0" << endl;
      outf << "111 0" << endl;
    }

    outf << "#final constraint" << endl;
    outf << ".names cst_onehot cst_vallut cst_atmost1 cst_ii out" << endl;
    outf << "1111 1" << endl;

  // from here original circuit 
    outf << endl << "# from here original circuit" << endl;
    WriteBlifBody(outf);
    outf << ".end" << endl;
    outf.close();
  }

  vector<string> make_LUT(string filename,long ninput){
    ofstream outfile(filename);
    outfile << ".model LUT" << ninput << endl;
    vector<string> vinputs;
    // make inputs
    outfile << ".inputs";
    for(long i = 0; i < ninput; i++){
      string input = "in" + to_string(i);
      outfile << " " << input;
      vinputs.push_back(input);
    }
    outfile << endl << ".inputs";
    long npars = 1 << ninput;
    for(long i = 0; i < npars; i++){
      string input = "LUT" + to_string(i);
      outfile << " LUT" << i ;
      vinputs.push_back(input);
    }
    outfile << endl;

    // make outputs
    outfile << ".outputs out" << endl;

    // make circuit body
    outfile << ".names" ;
    // for(auto input : vinputs){outfile << " " << input;}
    for(int i = 0; i < vinputs.size(); i++){
      if(i < ninput){ outfile << " " << vinputs[ninput - 1 - i];}
      else{ outfile << " " << vinputs[i];}
    }
    outfile << " out" << endl;
    for(int i = 0; i < npars; i++){
      boost::dynamic_bitset<> bs(ninput,i);
      outfile << bs;
      for(int j = 0 ; j < npars; j++){
        if(i == j) outfile << "1";
        else outfile << "-";
      }
      outfile << " 1" <<endl;
    }
    outfile << ".end";
    cout << "finish make LUT" << endl;

    return vinputs;
  }

// MUX selecting (ninput val -> 1 val)
  vector<string> make_MUX(string filename, long ninput){
    ofstream outfile(filename);
    vector<string> vinputs;
    outfile << ".model MUXnet" << endl;
    outfile << ".inputs";
    for(int i = 0; i < ninput; i++){
      string input = "ctl" + to_string(i);
      outfile << " " << input;
      vinputs.push_back(input);
    }
    outfile << endl << ".inputs";
    for(int i = 0; i < ninput; i++){
      string input = "in" + to_string(i);
      outfile << " " << input;
      vinputs.push_back(input);
    }

    outfile << endl << ".outputs out" << endl;

    outfile << ".names";
    for(auto input : vinputs){
      outfile << " " << input;
    }
    outfile << " out" << endl;
    boost::dynamic_bitset<> bs(ninput,1);
    for(int i = 0; i < ninput; i++){
      outfile << (bs << i) ;
      for(int j = 0 ; j < ninput ; j++){
        if(i == ninput - j - 1){ outfile << "1";}
        else{outfile << "-";}
      }
      outfile << " 1" << endl;
    }
    outfile << ".end";

    return vinputs;
  }

  vector<string> make_checkonehot(std::string filename, long ninput, bool atmost1){
    ofstream outfile(filename);
    vector<string> vinputs;
    if(atmost1){outfile << ".model check_atmost1" << endl;}
    else{outfile << ".model check_onehot" << endl;}
    outfile << ".inputs";
    for(int i = 0 ; i < ninput ; i++){
      string input = "in"  + to_string(i);
      outfile << " " << input;
      vinputs.push_back(input);
    }
    outfile << endl << ".outputs out" << endl;
    outfile << ".names";
    for(auto input : vinputs){
      outfile << " " << input;
    }
    outfile << " out" << endl;
    boost::dynamic_bitset<> bs(ninput,1);
    for(int i = 0; i < ninput; i++){
      outfile << (bs << i) << " 1" << endl;
    }
    if(atmost1){outfile << bs.reset() << " 1" << endl;}
    outfile << ".end";
    return vinputs;
  }

  void Circuit::print_circuitinfo(){
    cout << "================ Circuit Info ==================" << endl;
    cout << "|           name = " << name << endl;
    cout << "|           Gate = " << all_nodes.size() << endl;
    cout << "| Inputs/Outputs = " << inputs.size() << "/" << outputs.size() << endl;
    cout << "|            FFs = " << ffs.size() << endl;
    cout << "================================================" << endl;
  }

// ================================= from here ==================================================================
// original cad contents
  int Circuit::ReadBlif(string filename, bool simplify, bool levelize_sort, bool ignore_1input) {
    Clear();

    FILE *infile = fopen(filename.c_str(), "r");
    if (!infile) {
      ERR("cannot open input file: " + filename);
      return 0;
    }
    ofstream parser_log("parser.log");
    blifparser::BL_LEXER ilexer(infile, parser_log);
    ilexer.circuit = this;
    ilexer.bl_lex();
    fclose(infile);
    parser_log.close();

    system("rm parser.log");

    CreateCodedBlifValues();
    ConvertEquivalentTypeBlif();

    if (simplify) {
      LevelizeSortTopological(false);
      RemoveBufNot();
      Simplify();
      LevelizeSortTopological(false);
    }
    else if (levelize_sort) {
      LevelizeSortTopological(ignore_1input);
    }

    SetIndexes();

    return 1;
  }

  int Circuit::WriteBlif(string filename) {
    ofstream out_file_stream(filename);
    if (!out_file_stream.is_open())
      return 0;

    long i, j, k;

    out_file_stream << ".model " << name << endl;
    out_file_stream << ".inputs ";
    for (i = 0; i < inputs.size(); i++){
      out_file_stream << " \\" << endl << " " << inputs[i]->name;
    }
    out_file_stream << endl;
    out_file_stream << ".outputs ";
    for (i = 0; i < outputs.size(); i++){
      out_file_stream << " \\" << endl << " " << outputs[i]->name;
    }
    out_file_stream << endl;
    for (i = 0; i < ffs.size(); i++){
      out_file_stream << ".latch " << ffs[i]->inputs[0]->name << " " << ffs[i]->name << endl;
    }
    WriteBlifBody(out_file_stream);

    out_file_stream << ".end" << endl;
    out_file_stream << endl;

    out_file_stream.close();
    return 0;
  }

  int Circuit::WriteBlifBody(ostream &body) {
    long i, j, k;

    for (i = 0; i < all_nodes.size(); i++) {
      Node *node = all_nodes[i];
      if (node->type == NODE_ONE) {
        body << ".names " << node->name << endl << "1" << endl;
        continue;
      }
      if (node->type == NODE_ZERO) {
        body << ".names " << node->name << endl << "0" << endl;
        continue;
      }
      if (node->is_input)
        continue;
      if (node->type == NODE_DFF)
        continue;
      body << ".names";
      for (j = 0; j < node->inputs.size(); j++)
        body << " " << node->inputs[j]->name;
      body << " " << node->name << endl;
      switch (node->type) {
        case NODE_BUF:
          body << "1 1" << endl;
          break;
        case NODE_NOT:
          body << "1 0" << endl;
          break;
        case NODE_AND:
          for (k = 0; k < node->inputs.size(); k++)
            body << "1";
          body << " 1" << endl;
          break;
        case NODE_NAND:
          for (k = 0; k < node->inputs.size(); k++)
            body << "1";
          body << " 0" << endl;
          break;
        case NODE_OR:
          for (k = 0; k < node->inputs.size(); k++)
            body << "0";
          body << " 0" << endl;
          break;
        case NODE_NOR:
          for (k = 0; k < node->inputs.size(); k++)
            body << "0";
          body << " 1" << endl;
          break;
        case NODE_XOR:
          switch (node->inputs.size()) {
            case 2:
              body << "10 1" << endl;
              body << "01 1" << endl;
              break;
            case 3:
              body << "100 1" << endl;
              body << "010 1" << endl;
              body << "001 1" << endl;
              body << "111 1" << endl;
              break;
              // TODO: for larger XORs
          }
          break;
        case NODE_XNOR:
          switch (node->inputs.size()) {
            case 2:
              body << "10 0" << endl;
              body << "01 0" << endl;
              break;
            case 3:
              body << "100 0" << endl;
              body << "010 0" << endl;
              body << "001 0" << endl;
              body << "111 0" << endl;
              break;
              // TODO: for larger XNORs
          }
          break;
        case NODE_AND2_NP:
          body << "01 1" << endl;
          break;
        case NODE_NAND2_NP:
          body << "01 0" << endl;
          break;
        case NODE_AND2_PN:
          body << "10 1" << endl;
          break;
        case NODE_NAND2_PN:
          body << "10 0" << endl;
          break;
        case NODE_ONE:
          body << "1" << endl;
          break;
        case NODE_ZERO:
          body << "0" << endl;
          break;
        case NODE_BLIF:
          for (k = 0; k < ((BlifNode *) node)->str_values.size(); k++)
            body << ((BlifNode *) node)->str_values[k] << endl;
          //for (set<uint64_t>::iterator iter = ((BlifNode*)node)->coded_values.begin(); iter != ((BlifNode*)node)->coded_values.end() ; ++iter)
          //  out_file_stream << hex << *iter << endl;
          break;
        default:
          for (k = 0; k < node->inputs.size(); k++)
            body << "-";
          body << " 0" << endl;
          body << "# err " << node->type << endl;
          // this case should not happen
          // TODO: check why this case has happened
      }
    }

    return 0;
  }

  //======================================= Not used code =====================================================

  int Circuit::ApplyInOutSimplify(ValVector &input_vals, ValVector &output_vals) {
    if (input_vals.size() > inputs.size() /*|| output_vals.size() != outputs.size()*/)
      return -1;

    NodeVector::iterator in_iter = inputs.begin();
    for (long i = 0; i < input_vals.size(); i++) {
      Node *inode = *in_iter;
      inode->is_input = false;
      inode->inputs.clear();
      inode->type = input_vals[i] ? NODE_ONE : NODE_ZERO;
      ++in_iter;
    }
    inputs.erase(inputs.begin(), in_iter);

    return Simplify();
  }

  int Circuit::Simplify() {
    NodeVector::iterator it;
    NodeSet orphan_nodes;
    // TODO: add simplification for the new 2-input and/nand gates
    for (long index = 0; index < all_nodes.size(); index++) {
      Node *cur_node = all_nodes[index];
      if (cur_node->type == NODE_ZERO) {
        for (long k = 0; k < cur_node->outputs.size(); k++) {
          Node *out_node = cur_node->outputs[k];
          switch (out_node->type) {
            case NODE_BUF:
            case NODE_AND:
              for (long m = 0; m < out_node->inputs.size(); m++) {
                Node *n = out_node->inputs[m];
                if (n == cur_node)
                  continue;
                it = n->outputs.begin();
                while (it != n->outputs.end()) {
                  if (*it == out_node) {
                    n->outputs.erase(it);
                    break;
                  }
                  ++it;
                }
                if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
                  orphan_nodes.insert(n);
                }
              }
              out_node->inputs.clear();
              out_node->type = NODE_ZERO;
              break;
            case NODE_NOT:
            case NODE_NAND:
              for (long m = 0; m < out_node->inputs.size(); m++) {
                Node *n = out_node->inputs[m];
                if (n == cur_node)
                  continue;
                it = n->outputs.begin();
                while (it != n->outputs.end()) {
                  if (*it == out_node) {
                    n->outputs.erase(it);
                    break;
                  }
                  ++it;
                }
                if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
                  orphan_nodes.insert(n);
                }
              }
              out_node->inputs.clear();
              out_node->type = NODE_ONE;
              break;
            case NODE_OR:
            case NODE_XOR: // TODO: only 2-input XOR
              it = out_node->inputs.begin();
              while (it != out_node->inputs.end()) {
                if (*it == cur_node) {
                  out_node->inputs.erase(it);
                  break;
                }
                ++it;
              }
              if (out_node->inputs.size() == 1)
                out_node->type = NODE_BUF;
              break;
            case NODE_NOR:
            case NODE_XNOR: // TODO: only 2-input XNOR
              it = out_node->inputs.begin();
              while (it != out_node->inputs.end()) {
                if (*it == cur_node) {
                  out_node->inputs.erase(it);
                  break;
                }
                ++it;
              }
              if (out_node->inputs.size() == 1)
                out_node->type = NODE_NOT;
              break;
            default:
              if ((out_node->type == NODE_AND2_NP && out_node->inputs[1] == cur_node) || (out_node->type == NODE_AND2_PN && out_node->inputs[0] == cur_node)) {
                for (long m = 0; m < out_node->inputs.size(); m++) {
                  Node *n = out_node->inputs[m];
                  if (n == cur_node)
                    continue;
                  it = n->outputs.begin();
                  while (it != n->outputs.end()) {
                    if (*it == out_node) {
                      n->outputs.erase(it);
                      break;
                    }
                    ++it;
                  }
                  if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
                    orphan_nodes.insert(n);
                  }
                }
                out_node->inputs.clear();
                out_node->type = NODE_ZERO;
              }
              else if ((out_node->type == NODE_AND2_NP && out_node->inputs[0] == cur_node) || (out_node->type == NODE_AND2_PN && out_node->inputs[1] == cur_node)) {
                it = out_node->inputs.begin();
                while (it != out_node->inputs.end()) {
                  if (*it == cur_node) {
                    out_node->inputs.erase(it);
                    break;
                  }
                  ++it;
                }
                if (out_node->inputs.size() == 1)
                  out_node->type = NODE_BUF;
              }
              else if ((out_node->type == NODE_NAND2_NP && out_node->inputs[1] == cur_node) || (out_node->type == NODE_NAND2_PN && out_node->inputs[0] == cur_node)) {
                for (long m = 0; m < out_node->inputs.size(); m++) {
                  Node *n = out_node->inputs[m];
                  if (n == cur_node)
                    continue;
                  it = n->outputs.begin();
                  while (it != n->outputs.end()) {
                    if (*it == out_node) {
                      n->outputs.erase(it);
                      break;
                    }
                    ++it;
                  }
                  if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
                    orphan_nodes.insert(n);
                  }
                }
                out_node->inputs.clear();
                out_node->type = NODE_ONE;
              }
              else if ((out_node->type == NODE_NAND2_NP && out_node->inputs[0] == cur_node) || (out_node->type == NODE_NAND2_PN && out_node->inputs[1] == cur_node)) {
                it = out_node->inputs.begin();
                while (it != out_node->inputs.end()) {
                  if (*it == cur_node) {
                    out_node->inputs.erase(it);
                    break;
                  }
                  ++it;
                }
                if (out_node->inputs.size() == 1)
                  out_node->type = NODE_NOT;
              }
          }
        }
      }
      else if (cur_node->type == NODE_ONE) {
        for (long k = 0; k < cur_node->outputs.size(); k++) {
          Node *out_node = cur_node->outputs[k];
          switch (out_node->type) {
            case NODE_BUF:
            case NODE_OR:
              for (long m = 0; m < out_node->inputs.size(); m++) {
                Node *n = out_node->inputs[m];
                if (n == cur_node)
                  continue;
                it = n->outputs.begin();
                while (it != n->outputs.end()) {
                  if (*it == out_node) {
                    n->outputs.erase(it);
                    break;
                  }
                  ++it;
                }
                if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
                  orphan_nodes.insert(n);
                }
              }
              out_node->inputs.clear();
              out_node->type = NODE_ONE;
              break;
            case NODE_NOT:
            case NODE_NOR:
              for (long m = 0; m < out_node->inputs.size(); m++) {
                Node *n = out_node->inputs[m];
                if (n == cur_node)
                  continue;
                it = n->outputs.begin();
                while (it != n->outputs.end()) {
                  if (*it == out_node) {
                    n->outputs.erase(it);
                    break;
                  }
                  ++it;
                }
                if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
                  orphan_nodes.insert(n);
                }
              }
              out_node->inputs.clear();
              out_node->type = NODE_ZERO;
              break;
            case NODE_AND:
            case NODE_XNOR: // TODO: only 2-input XNOR
              it = out_node->inputs.begin();
              while (it != out_node->inputs.end()) {
                if (*it == cur_node) {
                  out_node->inputs.erase(it);
                  break;
                }
                ++it;
              }
              if (out_node->inputs.size() == 1)
                out_node->type = NODE_BUF;
              break;
            case NODE_NAND:
            case NODE_XOR: // TODO: only 2-input XOR
              it = out_node->inputs.begin();
              while (it != out_node->inputs.end()) {
                if (*it == cur_node) {
                  out_node->inputs.erase(it);
                  break;
                }
                ++it;
              }
              if (out_node->inputs.size() == 1)
                out_node->type = NODE_NOT;
              break;
            default:
              if ((out_node->type == NODE_AND2_NP && out_node->inputs[0] == cur_node) || (out_node->type == NODE_AND2_PN && out_node->inputs[1] == cur_node)) {
                for (long m = 0; m < out_node->inputs.size(); m++) {
                  Node *n = out_node->inputs[m];
                  if (n == cur_node)
                    continue;
                  it = n->outputs.begin();
                  while (it != n->outputs.end()) {
                    if (*it == out_node) {
                      n->outputs.erase(it);
                      break;
                    }
                    ++it;
                  }
                  if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
                    orphan_nodes.insert(n);
                  }
                }
                out_node->inputs.clear();
                out_node->type = NODE_ZERO;
              }
              else if ((out_node->type == NODE_AND2_NP && out_node->inputs[1] == cur_node) || (out_node->type == NODE_AND2_PN && out_node->inputs[0] == cur_node)) {
                it = out_node->inputs.begin();
                while (it != out_node->inputs.end()) {
                  if (*it == cur_node) {
                    out_node->inputs.erase(it);
                    break;
                  }
                  ++it;
                }
                if (out_node->inputs.size() == 1)
                  out_node->type = NODE_NOT;
              }
              else if ((out_node->type == NODE_NAND2_NP && out_node->inputs[0] == cur_node) || (out_node->type == NODE_NAND2_PN && out_node->inputs[1] == cur_node)) {
                for (long m = 0; m < out_node->inputs.size(); m++) {
                  Node *n = out_node->inputs[m];
                  if (n == cur_node)
                    continue;
                  it = n->outputs.begin();
                  while (it != n->outputs.end()) {
                    if (*it == out_node) {
                      n->outputs.erase(it);
                      break;
                    }
                    ++it;
                  }
                  if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
                    orphan_nodes.insert(n);
                  }
                }
                out_node->inputs.clear();
                out_node->type = NODE_ONE;
              }
              else if ((out_node->type == NODE_NAND2_NP && out_node->inputs[1] == cur_node) || (out_node->type == NODE_NAND2_PN && out_node->inputs[0] == cur_node)) {
                it = out_node->inputs.begin();
                while (it != out_node->inputs.end()) {
                  if (*it == cur_node) {
                    out_node->inputs.erase(it);
                    break;
                  }
                  ++it;
                }
                if (out_node->inputs.size() == 1)
                  out_node->type = NODE_BUF;
              }
          }
        }
      }
      else if (cur_node->outputs.size() == 0 && !cur_node->is_output && !cur_node->is_input && cur_node->type != NODE_DFF) {
        orphan_nodes.insert(cur_node);
      }
    }

    while (orphan_nodes.size() > 0) {
      Node *onode = *orphan_nodes.begin();
      orphan_nodes.erase(orphan_nodes.begin());
      for (long m = 0; m < onode->inputs.size(); m++) {
        Node *n = onode->inputs[m];
        if (n->type == NODE_ZERO || n->type == NODE_ONE)
          continue;
        it = n->outputs.begin();
        while (it != n->outputs.end()) {
          if (*it == onode) {
            n->outputs.erase(it);
            break;
          }
          ++it;
        }
        if (n->outputs.size() == 0 && !n->is_output && !n->is_input && n->type != NODE_DFF) {
          orphan_nodes.insert(n);
        }
      }
    }

    int copy_index = 0, cur_index = 0;
    while (cur_index < all_nodes.size()) {
      Node *cur_node = all_nodes[cur_index];
      if (cur_node->is_input || cur_node->is_output) {
        if (copy_index < cur_index)
          all_nodes[copy_index] = all_nodes[cur_index];
        cur_index++;
        copy_index++;
      }
      else if (cur_node->type == NODE_ZERO || cur_node->type == NODE_ONE || cur_node->outputs.size() == 0) {
        all_nodes_map.erase(cur_node->name);
        cur_node->inputs.clear();
        cur_node->outputs.clear();
        delete cur_node;
        cur_index++;
      }
      else {
        if (copy_index < cur_index)
          all_nodes[copy_index] = all_nodes[cur_index];
        cur_index++;
        copy_index++;
      }
    }
    all_nodes.resize(copy_index);
    all_nodes.shrink_to_fit();

    return 0;
  }

  int Circuit::Simplify(NodeVector& target_nodes) {
    if (target_nodes.size() == 0)
      return -1;

    // find affected outputs
    NodeSet fanout_cone;
    NodeSet fanout_outputs;
    NodeSet process_list;
    for (int i = 0; i < target_nodes.size(); i++)
      process_list.insert(target_nodes[i]);
    while (process_list.size() > 0) {
      Node *cur_node = *process_list.begin();
      process_list.erase(process_list.begin());
      fanout_cone.insert(cur_node);
      if (cur_node->is_output)
        fanout_outputs.insert(cur_node);
      for (int i = 0; i < cur_node->outputs.size(); i++) {
        Node *out_node = cur_node->outputs[i];
        if (fanout_cone.find(out_node) == fanout_cone.end())
          process_list.insert(out_node);
      }
    }

    if (fanout_outputs.size() == outputs.size())
      return 0;

    int org_inp_size = inputs.size();
    int org_outp_size = outputs.size();
    int org_node_size = all_nodes.size();

    // remove not affected outputs
    NodeVector::iterator iter = outputs.begin();
    while (iter != outputs.end()) {
      if (fanout_outputs.find(*iter) == fanout_outputs.end()) {
        (*iter)->is_output = false;
        iter = outputs.erase(iter);
      }
      else
        ++iter;
    }

    Simplify();

    //remove not affecting inputs
    iter = inputs.begin();
    while (iter != inputs.end()) {
      if ((*iter)->outputs.size() == 0) {
        (*iter)->is_input = false;
        (*iter)->type = NODE_ZERO; // TODO: should be removed from the all nodes list!
        iter = inputs.erase(iter);
      }
      else
        ++iter;
    }

    cout << "Nodes/PIs/POs after(before) simplification based on:";
    for (int i = 0; i < target_nodes.size(); i++)
      cout << " " << target_nodes[i]->name;
    cout << " -> " << all_nodes.size() << "(" << org_node_size << ")/" << inputs.size() << "(" << org_inp_size << ")/" << outputs.size() << "(" << org_outp_size << ")" << endl;

    return 0;
  }

  int Circuit::Simulate(ValVector &input_vals, ValVector &output_vals) {
    if (ffs.size() > 0)
      return 1;
    ValVector ff_vals;
    ValVector all_vals;
    all_vals.resize(all_nodes.size());
    return Simulate(input_vals, output_vals, ff_vals, all_vals);
  }

  int Circuit::Simulate(ValVector &input_vals, ValVector &output_vals, ValVector &ff_vals) {
    ValVector all_vals;
    all_vals.resize(all_nodes.size());
    return Simulate(input_vals, output_vals, ff_vals, all_vals);
  }

  int Circuit::Simulate(ValVector &input_vals, ValVector &output_vals, ValVector &ff_vals, ValVector &all_vals) {
    if (input_vals.size() != inputs.size()) {
      ERR("Simulation Error: values are not assigned for all the inputs!");
      return 1;
    }
    if (ff_vals.size() != ffs.size()) {
      ERR("Simulation Error: values are not assigned for all the FFs!");
      return 1;
    }
    output_vals.resize(outputs.size());
    all_vals.resize(all_nodes.size());
    for (long i = 0; i < inputs.size(); i++)
      all_vals[inputs[i]->index] = input_vals[i];
    for (long k = 0; k < ffs.size(); k++)
      all_vals[ffs[k]->index] = ff_vals[k];
    NodeVector::iterator node_it = all_nodes.begin();
    while ((*node_it)->inputs.size() == 0) {
      if ((*node_it)->type == NODE_ONE)
        all_vals[(*node_it)->index] = true;
      else if ((*node_it)->type == NODE_ZERO)
        all_vals[(*node_it)->index] = false;
      // TODO: check other cases should not happen
      ++node_it;
    }
    long i = 0;
    while (node_it != all_nodes.end()) {
      // do simulate!
      bool res;
      switch ((*node_it)->type) {
        case NODE_ZERO:
          all_vals[(*node_it)->index] = false;
          break;
        case NODE_ONE:
          all_vals[(*node_it)->index] = true;
          break;
        case NODE_BUF:
          all_vals[(*node_it)->index] = all_vals[(*node_it)->inputs[0]->index];
          break;
        case NODE_NOT:
          all_vals[(*node_it)->index] = !all_vals[(*node_it)->inputs[0]->index];
          break;
        case NODE_AND:
          res = all_vals[(*node_it)->inputs[0]->index];
          for (i = 1; res && i < (*node_it)->inputs.size(); i++)
            res &= all_vals[(*node_it)->inputs[i]->index];
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_NAND:
          res = all_vals[(*node_it)->inputs[0]->index];
          for (i = 1; res && i < (*node_it)->inputs.size(); i++)
            res &= all_vals[(*node_it)->inputs[i]->index];
          all_vals[(*node_it)->index] = !res;
          break;
        case NODE_OR:
          res = all_vals[(*node_it)->inputs[0]->index];
          for (i = 1; !res && i < (*node_it)->inputs.size(); i++)
            res |= all_vals[(*node_it)->inputs[i]->index];
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_NOR:
          res = all_vals[(*node_it)->inputs[0]->index];
          for (i = 1; !res && i < (*node_it)->inputs.size(); i++)
            res |= all_vals[(*node_it)->inputs[i]->index];
          all_vals[(*node_it)->index] = !res;
          break;
        case NODE_XOR:
          // TODO: currently only for 2 input gate
          res = all_vals[(*node_it)->inputs[0]->index] ^ all_vals[(*node_it)->inputs[1]->index];
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_XNOR:
          // TODO: currently only for 2 input gate
          res = all_vals[(*node_it)->inputs[0]->index] ^ all_vals[(*node_it)->inputs[1]->index];
          all_vals[(*node_it)->index] = !res;
          break;
        case NODE_AND2_NP:
          res = (!all_vals[(*node_it)->inputs[0]->index]) & all_vals[(*node_it)->inputs[1]->index];
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_NAND2_NP:
          res = (!all_vals[(*node_it)->inputs[0]->index]) & all_vals[(*node_it)->inputs[1]->index];
          all_vals[(*node_it)->index] = !res;
          break;
        case NODE_AND2_PN:
          res = all_vals[(*node_it)->inputs[0]->index] & (!all_vals[(*node_it)->inputs[1]->index]);
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_NAND2_PN:
          res = all_vals[(*node_it)->inputs[0]->index] & (!all_vals[(*node_it)->inputs[1]->index]);
          all_vals[(*node_it)->index] = !res;
          break;
        case NODE_BLIF:
          ERR("general blif gate is not yet supported! " + (*node_it)->name);
          return 1;
          break;
        case NODE_DFF:
          // update at the last stage!
          break;
        default:
          ERR("unexpected gate in simulation! " + (*node_it)->name);
          return 1;
      }
      ++node_it;
    }

    for (long i = 0; i < outputs.size(); i++)
      output_vals[i] = all_vals[outputs[i]->index];
    for (long k = 0; k < ffs.size(); k++) {
      Node* ffnode = ffs[k];
      bool val = all_vals[ffnode->inputs[0]->index];
      all_vals[ffnode->index] = val;
      ff_vals[k] = val;
    }
    return 0;
  }

  int Circuit::Simulate(Val64Vector &input_vals, Val64Vector &output_vals) {
    Val64Vector all_vals;
    return Simulate(input_vals,output_vals, all_vals);
  }

  int Circuit::Simulate(Val64Vector &input_vals, Val64Vector &output_vals, Val64Vector &all_vals) {
    if (input_vals.size() != inputs.size()) {
      ERR("Simulation Error: values are not assigned for all the inputs!");
      return 1;
    }
    if (ffs.size() > 0) {
      ERR("Simulation Error: sequential circuit cannot be simulated this way!");
      return 1;
    }
    output_vals.resize(outputs.size());
    all_vals.resize(all_nodes.size());
    for (long i = 0; i < inputs.size(); i++)
      all_vals[inputs[i]->index] = input_vals[i];
    NodeVector::iterator node_it = all_nodes.begin();
    while (node_it != all_nodes.end() && (*node_it)->inputs.size() == 0) {
      if ((*node_it)->type == NODE_ONE)
        all_vals[(*node_it)->index] = ONE64;
      else if ((*node_it)->type == NODE_ZERO)
        all_vals[(*node_it)->index] = ZERO64;
      // TODO: check other cases should not happen
      ++node_it;
    }
    long i = 0;
    while (node_it != all_nodes.end()) {
      // do simulate!
      bit64 res;
      switch ((*node_it)->type) {
        case NODE_ZERO:
          all_vals[(*node_it)->index] = ZERO64;
          break;
        case NODE_ONE:
          all_vals[(*node_it)->index] = ONE64;
          break;
        case NODE_BUF:
          all_vals[(*node_it)->index] = all_vals[(*node_it)->inputs[0]->index];
          break;
        case NODE_NOT:
          all_vals[(*node_it)->index] = ~all_vals[(*node_it)->inputs[0]->index];
          break;
        case NODE_AND:
          res = all_vals[(*node_it)->inputs[0]->index];
          for (i = 1; i < (*node_it)->inputs.size(); i++)
            res &= all_vals[(*node_it)->inputs[i]->index];
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_NAND:
          res = all_vals[(*node_it)->inputs[0]->index];
          for (i = 1; i < (*node_it)->inputs.size(); i++)
            res &= all_vals[(*node_it)->inputs[i]->index];
          all_vals[(*node_it)->index] = ~res;
          break;
        case NODE_OR:
          res = all_vals[(*node_it)->inputs[0]->index];
          for (i = 1; i < (*node_it)->inputs.size(); i++)
            res |= all_vals[(*node_it)->inputs[i]->index];
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_NOR:
          res = all_vals[(*node_it)->inputs[0]->index];
          for (i = 1; i < (*node_it)->inputs.size(); i++)
            res |= all_vals[(*node_it)->inputs[i]->index];
          all_vals[(*node_it)->index] = ~res;
          break;
        case NODE_XOR:
          // TODO: currently only for 2 input gate
          res = all_vals[(*node_it)->inputs[0]->index] ^ all_vals[(*node_it)->inputs[1]->index];
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_XNOR:
          // TODO: currently only for 2 input gate
          res = all_vals[(*node_it)->inputs[0]->index] ^ all_vals[(*node_it)->inputs[1]->index];
          all_vals[(*node_it)->index] = ~res;
          break;
        case NODE_AND2_NP:
          res = (~all_vals[(*node_it)->inputs[0]->index]) & all_vals[(*node_it)->inputs[1]->index];
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_NAND2_NP:
          res = (~all_vals[(*node_it)->inputs[0]->index]) & all_vals[(*node_it)->inputs[1]->index];
          all_vals[(*node_it)->index] = ~res;
          break;
        case NODE_AND2_PN:
          res = all_vals[(*node_it)->inputs[0]->index] & (~all_vals[(*node_it)->inputs[1]->index]);
          all_vals[(*node_it)->index] = res;
          break;
        case NODE_NAND2_PN:
          res = all_vals[(*node_it)->inputs[0]->index] & (~all_vals[(*node_it)->inputs[1]->index]);
          all_vals[(*node_it)->index] = ~res;
          break;
        case NODE_BLIF:
          ERR("general blif gate is not yet supported! " + (*node_it)->name);
          return 1;
          break;
        case NODE_DFF:
          ERR("sequential circuit is not yet supported! " + (*node_it)->name);
          return 1;
          break;
        default:
          ERR("unexpected gate in simulation! " + (*node_it)->name);
          return 1;
      }
      ++node_it;
    }

    for (long i = 0; i < outputs.size(); i++)
      output_vals[i] = all_vals[outputs[i]->index];
    return 0;
  }


  int Circuit::LevelizeSortTopological(bool ignore_1input) {
    // sort from inputs to outputs
    Levelize(false);
    SortCurrentLevel();
    if (ignore_1input) {
      Levelize(true);
      SortCurrentLevel();
    }
    return 0;
  }

  int Circuit::SortCurrentLevel() {
    vector<int> level_count;
    vector<int> level_start_index;
    level_count.reserve(128);
    level_start_index.reserve(128);
    for (int i = 0; i < all_nodes.size(); i++) {
      int level = all_nodes[i]->level;
      if (level < 0)
        continue;
      if (level_count.size() < level+1) {
        level_count.resize(level + 1, 0);
        level_start_index.resize(level+1, 0);
      }
      int cur_count = level_count[level];
      if (cur_count == 0) {
        level_count[level] = 1;
        level_start_index[level] = i;
      }
      else {
        level_count[level] = cur_count+1;
      }
    }

    NodeVector cp_all_nodes(all_nodes);
    int index = 0;
    for (int level = 0; level < level_count.size(); level++) {
      int count = level_count[level];
      int cp_index = level_start_index[level];
      //cout << "level " << level << ": " << count << " starting from: " << cp_index << endl;
      for (int j = 0; j < count; ) {
        Node* node = cp_all_nodes[cp_index];
        if (node->level == level) {
          all_nodes[index] = node;
          index++;
          j++;
        }
        cp_index++;
      }
    }

    if (index < all_nodes.size()) {
      MSG("nodes with level=-1 removed! "+std::to_string(all_nodes.size())+ " < "+std::to_string(index));
      all_nodes.resize(index);
    }

    SetIndexes();

    return 0;

    // OLD sort! very slow
    NodeVector::iterator iter1, iter2, iter3;
    iter3 = all_nodes.begin();
    iter1 = iter3;
    ++iter3;
    iter2 = iter3;
    while (iter3 != all_nodes.end()) {
      while (iter1 != all_nodes.begin() && (*iter2)->level < (*iter1)->level) {
        Node *temp = *iter1;
        *iter1 = *iter2;
        *iter2 = temp;
        //MSG("swapping "+(*iter1)->name+" "+(*iter2)->name);
        iter1--;
        iter2--;
      }
      /*while (iter1 != all_nodes.begin() && (*iter2)->level == (*iter1)->level && (*iter2)->inputs.size() > (*iter1)->inputs.size()) {
        Node* temp = *iter1;
        *iter1 = *iter2;
        *iter2 = temp;
        MSG("swapping "+(*iter1)->name+" "+(*iter2)->name);
        iter1--;
        iter2--;
      }
      while (iter1 != all_nodes.begin() && (*iter2)->level == (*iter1)->level && (*iter2)->inputs.size() == (*iter1)->inputs.size() && (*iter2)->type < (*iter1)->type) {
        Node* temp = *iter1;
        *iter1 = *iter2;
        *iter2 = temp;
        MSG("swapping "+(*iter1)->name+" "+(*iter2)->name);
        iter1--;
        iter2--;
      }*/
      iter1 = iter3;
      ++iter3;
      iter2 = iter3;
    }

    SetIndexes();
    return 0;
  }

  int Circuit::Levelize(bool ignore_1input) {
    long i, j, k;

    queue<Node *> node_process_queue;
    for (i = 0; i < all_nodes.size(); i++) {
      Node *node = all_nodes[i];
      if (node->inputs.size() == 0 || node->type == NODE_DFF) {
        node->level = 0;
        for (j = 0; j < node->outputs.size(); j++)
          node_process_queue.push(node->outputs[j]);
      }
      else {
        node->level = -1; // reset the level
      }
    }
    while (node_process_queue.size() > 0) {
      Node *node = node_process_queue.front();
      node_process_queue.pop();
      if (node->level < 0) {
        int max_level = 0;
        for (k = 0; k < node->inputs.size() && max_level >= 0; k++) {
          int lev = node->inputs[k]->level;
          if (lev < 0)
            max_level = -1;
          else if (lev > max_level)
            max_level = lev;
        }
        if (max_level >= 0) {
          if (node->inputs.size() > 1)
            node->level = max_level + 1;
          else if (ignore_1input)
            node->level = max_level;
          else
            node->level = max_level + 1;
          for (k = 0; k < node->outputs.size(); k++) {
            Node *outnode = node->outputs[k];
            if (outnode->level < 0)
              node_process_queue.push(outnode);
          }
        }
      }
    }

    for (int k = 0; k < ffs.size(); k++) {
      Node* ffnode = ffs[k];
      ffnode->level = ffnode->inputs[0]->level + 1;
    }

    return 0;
  }

  int Circuit::SetIndexes() {
    for (long i = 0; i < all_nodes.size(); i++)
      all_nodes[i]->index = i;
    return 0;
  }

  int Circuit::ResetLevels() {
    for (long i = 0; i < all_nodes.size(); i++)
      all_nodes[i]->level = -1;
    return 0;
  }

  int Circuit::CreateCodedBlifValues() {
    for (long i = 0; i < all_nodes.size(); i++) {
      Node *node = all_nodes[i];
      if (node->type == NODE_BLIF)
        ((BlifNode *) node)->CreateCodedValues();
    }

    return 0;
  }

  int Circuit::ConvertEquivalentTypeBlif() {
    for (long i = 0; i < all_nodes.size(); i++) {
      Node *node = all_nodes[i];
      if (node->type == NODE_BLIF) {
        NodeType newtype = ((BlifNode *) node)->GetEquivalentType();
        if (newtype != node->type) {
          //MSG("node type changed: " + node->name);
          //cout << " -> " << node->type << " :: " << newtype << endl;
          node->type = newtype;
        }
      }
    }

    return 0;
  }

  int Circuit::RemoveBufNot() {
    long last_index = all_nodes.size() - 1;
    long cur_index = last_index;
    while (cur_index >= 0) {
      Node *cur_node = all_nodes[cur_index];
      if (cur_node->outputs.size() > 0 && cur_node->inputs.size() == 1 && !cur_node->is_output && !cur_node->is_input && cur_node->type != NODE_DFF) {
        Node *in_node = cur_node->inputs[0];
        if (in_node->outputs.size() == 1 && in_node->type < NODE_BLIF && in_node->type > NODE_UNKNOWN) {
          if (cur_node->type == NODE_NOT) {
            switch (in_node->type) {
              case NODE_ZERO:
                in_node->type = NODE_ONE;
                break;
              case NODE_ONE:
                in_node->type = NODE_ZERO;
                break;
              case NODE_BUF:
                in_node->type = NODE_NOT;
                break;
              case NODE_NOT:
                in_node->type = NODE_BUF;
                break;
              case NODE_AND:
                in_node->type = NODE_NAND;
                break;
              case NODE_NAND:
                in_node->type = NODE_AND;
                break;
              case NODE_OR:
                in_node->type = NODE_NOR;
                break;
              case NODE_NOR:
                in_node->type = NODE_OR;
                break;
              case NODE_XOR:
                in_node->type = NODE_XNOR;
                break;
              case NODE_XNOR:
                in_node->type = NODE_XOR;
                break;
              case NODE_AND2_NP:
                in_node->type = NODE_NAND2_NP;
                break;
              case NODE_NAND2_NP:
                in_node->type = NODE_AND2_NP;
                break;
              case NODE_AND2_PN:
                in_node->type = NODE_NAND2_PN;
                break;
              case NODE_NAND2_PN:
                in_node->type = NODE_AND2_PN;
                break;
            }
          }
          // else it is a buffer!
          in_node->outputs.clear();
          in_node->outputs = cur_node->outputs;
          for (long k = 0; k < cur_node->outputs.size(); k++) {
            Node *out_node = cur_node->outputs[k];
            for (int j = 0; j < out_node->inputs.size(); j++)
              if (out_node->inputs[j] == cur_node) {
                out_node->inputs[j] = in_node;
                break; // exit the loop
              }
          }
          all_nodes_map.erase(cur_node->name);
          //for (long i = cur_index; i < last_index; i++)
          //  all_nodes[i] = all_nodes[i + 1];
          //last_index--;
          delete cur_node;
          all_nodes[cur_index] = NULL; // delete later!
        }
      }
      cur_index--;
    }
    // delete removed nodes
    last_index = 0;
    while (last_index < all_nodes.size() && all_nodes[last_index] != NULL)
      last_index++;
    cur_index = last_index;
    while (cur_index < all_nodes.size() && all_nodes[cur_index] == NULL)
      cur_index++;
    if (last_index < all_nodes.size()) {
      while (cur_index < all_nodes.size()) {
        if (all_nodes[cur_index] != NULL) {
          all_nodes[last_index] = all_nodes[cur_index];
          last_index++;
        }
        cur_index++;
      }
      all_nodes.resize(last_index);
      all_nodes.shrink_to_fit();
    }

    return 0;
  }

  int Circuit::WriteVerilog(string filename) {
    ofstream out_file_stream(filename);
    if (!out_file_stream.is_open())
      return 0;

    long i, j, k;

    out_file_stream << "module " << name << " (" << endl;
    for (i = 0; i < outputs.size(); i++)
      out_file_stream << "  " << outputs[i]->name << "," << endl;
    for (i = 0; i < inputs.size() - 1; i++)
      out_file_stream << "  " << inputs[i]->name << "," << endl;
    out_file_stream << "  " << inputs[i]->name << ");" << endl;


    out_file_stream << "input" << endl;
    for (i = 0; i < inputs.size() - 1; i++)
      out_file_stream << "  " << inputs[i]->name << "," << endl;
    out_file_stream << "  " << inputs[i]->name << ";" << endl;

    out_file_stream << "output" << endl;
    for (i = 0; i < outputs.size() - 1; i++)
      out_file_stream << "  " << outputs[i]->name << "," << endl;
    out_file_stream << "  " << outputs[i]->name << ";" << endl;

    out_file_stream << "wire" << endl;
    bool first = true;
    for (i = 0; i < all_nodes.size(); i++) {
      Node *node = all_nodes[i];
      if (!node->is_input && !node->is_output) {
        if (!first)
          out_file_stream << "," << endl;
        out_file_stream << "  " << all_nodes[i]->name;
        first = false;
      }
    }
    out_file_stream << ";" << endl;

    for (i = 0; i < all_nodes.size(); i++) {
      Node *node = all_nodes[i];
      if (node->type == NODE_ONE) {
        out_file_stream << "  buf ( " << node->name << " , 1'b1 );" << endl;
        continue;
      }
      if (node->type == NODE_ZERO) {
        out_file_stream << "  buf ( " << node->name << " , 1'b0 );" << endl;
        continue;
      }
      if (node->is_input)
        continue;
      if (node->inputs.size() == 0)
        continue;
      switch (node->type) {
        case NODE_BUF:
          out_file_stream << "  buf (";
          break;
        case NODE_NOT:
          out_file_stream << "  not (";
          break;
        case NODE_AND:
          out_file_stream << "  and (";
          break;
        case NODE_NAND:
          out_file_stream << "  nand (";
          break;
        case NODE_OR:
          out_file_stream << "  or (";
          break;
        case NODE_NOR:
          out_file_stream << "  nor (";
          break;
        case NODE_XOR:
          out_file_stream << "  xor (";
          break;
        case NODE_XNOR:
          out_file_stream << "  xnor (";
          break;
        case NODE_BLIF:
          out_file_stream << "lut (";
          break;
        case NODE_DFF:
          out_file_stream << "dff (";
          break;
        default:
          out_file_stream << "GATE_" << node->type << " (";
          // TODO: check why this case has happened!
      }
      out_file_stream << " " << node->name;
      for (j = 0; j < node->inputs.size(); j++)
        out_file_stream << " , " << node->inputs[j]->name;
      out_file_stream << " );" << endl;
    }
    out_file_stream << "endmodule" << endl;

    out_file_stream.close();

    return 0;
  }

  Circuit *Circuit::GetDuplicate(string input_prefix, string output_prefix, string internal_prefix) {
    // TODO: different prefix for ffs?
    Circuit *new_cir = new Circuit;
    new_cir->name = name;
    new_cir->inputs.reserve(inputs.size());
    new_cir->outputs.reserve(outputs.size());
    new_cir->ffs.reserve(ffs.size());
    new_cir->all_nodes.reserve(all_nodes.size());

    for (long i = 0; i < all_nodes.size(); i++) {
      Node *cur_node = all_nodes[i];
      Node *new_node = NULL;
      if (cur_node->type == NODE_BLIF)
        new_node = new BlifNode;
      else
        new_node = new Node(cur_node->type);
      new_node->is_input = cur_node->is_input;
      new_node->is_output = cur_node->is_output;
      if (cur_node->is_input)
        new_node->name = input_prefix + cur_node->name;
      else if (cur_node->is_output)
        new_node->name = output_prefix + cur_node->name;
      else
        new_node->name = internal_prefix + cur_node->name;
      new_node->inputs.reserve(cur_node->inputs.size());
      new_node->outputs.reserve(cur_node->outputs.size());
      new_node->level = cur_node->level;
      new_node->index = i;
      cur_node->index = i;
      if (cur_node->type == NODE_BLIF) {
        ((BlifNode *) new_node)->result_is_one = ((BlifNode *) cur_node)->result_is_one;
        ((BlifNode *) new_node)->str_values = ((BlifNode *) cur_node)->str_values;
        ((BlifNode *) new_node)->coded_values = ((BlifNode *) cur_node)->coded_values;
      }
      new_cir->all_nodes.push_back(new_node);
      new_cir->all_nodes_map[new_node->name] = new_node;
    }
    for (long i = 0; i < all_nodes.size(); i++) {
      Node *cur_node = all_nodes[i];
      Node *new_node = new_cir->all_nodes[i];
      for (long j = 0; j < cur_node->inputs.size(); j++)
        new_node->inputs.push_back(new_cir->all_nodes[cur_node->inputs[j]->index]);
      for (long k = 0; k < cur_node->outputs.size(); k++)
        new_node->outputs.push_back(new_cir->all_nodes[cur_node->outputs[k]->index]);
    }
    for (long j = 0; j < inputs.size(); j++)
      new_cir->inputs.push_back(new_cir->all_nodes[inputs[j]->index]);
    for (long k = 0; k < outputs.size(); k++)
      new_cir->outputs.push_back(new_cir->all_nodes[outputs[k]->index]);
    for (long m = 0; m < ffs.size(); m++)
      new_cir->ffs.push_back(new_cir->all_nodes[ffs[m]->index]);

    return new_cir;
  }


}
