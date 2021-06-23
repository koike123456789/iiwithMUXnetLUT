#ifndef _CIRCUIT_H_INCLUDED
#define _CIRCUIT_H_INCLUDED

#include "global.h"
#include "node.h"
#include "applyABC.h"

namespace nodecircuit {

  typedef unsigned long long bit64;

  typedef std::vector<bool> ValVector;
  typedef std::vector<bit64> Val64Vector;


  const int NUM_SIG_PAR_CHANGE = 6; // 2^6 = 64

  const bit64 ZERO64 = 0x0000000000000000;
  const bit64 ONE64  = 0xFFFFFFFFFFFFFFFF;

  const bit64 PAT64[] = {
    0xAAAAAAAAAAAAAAAA,
    0xCCCCCCCCCCCCCCCC,
    0xF0F0F0F0F0F0F0F0,
    0xFF00FF00FF00FF00,
    0xFFFF0000FFFF0000,
    0xFFFFFFFF00000000
  };


  class Circuit {
  public:
    Circuit() {};

    virtual ~Circuit() { Clear(); };

    void Clear() {
      inputs.clear();
      outputs.clear();
      ffs.clear();
      all_nodes_map.clear();
      for (unsigned long i = 0; i < all_nodes.size(); i++) {
        delete all_nodes[i];
      }
      all_nodes.clear();
    }

    Circuit *GetDuplicate(std::string input_prefix, std::string output_prefix, std::string internal_prefix);

  public:
    int CreateCodedBlifValues();
    int ConvertEquivalentTypeBlif();
    int RemoveBufNot();
    int ReadBlif(std::string filename, bool simplify = true, bool levelize_sort = true, bool ignore_1input = false);
    int WriteVerilog(std::string filename);
    int WriteBlif(std::string filename);
    int WriteBlifBody(std::ostream &body);
    int SetIndexes();
    int ResetLevels();
    int Levelize(bool ignore_1input = true);
    int SortCurrentLevel();
    int LevelizeSortTopological(bool ignore_1input = true);
    int Simulate(ValVector &input_vals, ValVector &output_vals);
    int Simulate(ValVector &input_vals, ValVector &output_vals, ValVector &ff_vals);
    int Simulate(ValVector &input_vals, ValVector &output_vals, ValVector &ff_vals, ValVector &all_vals);
    int Simulate(Val64Vector &input_vals, Val64Vector &output_vals);
    int Simulate(Val64Vector &input_vals, Val64Vector &output_vals, Val64Vector &all_vals);
    int Simplify();
    int Simplify(NodeVector& target_nodes);
    int ApplyInOutSimplify(ValVector &input_vals, ValVector &output_vals);

    std::string genQBF_withMUX(int lutsize, int muxsize, bool fUseout = false, bool fverbose = false);
    void write_genqbfblif(std::string filename, int lutsize, int muxsize, bool fUseout = false);
    void print_circuitinfo();

  public:
    std::string name;
    NodeVector inputs;    // primary inputs
    NodeVector outputs;   // primary outputs
    NodeVector ffs; // FFs
    NodeVector all_nodes; // all nodes including inputs/outputs/zero/one
    // mapping node names to nodes
    std::map<std::string, Node *> all_nodes_map;

    // find a node by name, returns NULL if not found
    Node *GetNode(std::string name) {
      std::map<std::string, Node *>::iterator it = all_nodes_map.find(name);
      if (it != all_nodes_map.end())
        return it->second;
      return NULL;
    }

    // ==================== from here implemented koike ================================
    long npara_ctl, npara_lut;
    std::string outfile;
    std::vector<std::string> vinputs_original_lut, vinputs_original_mux;
    std::vector<std::string> vinputs_original_checkonehot, vinputs_original_checkatmost1;


  };

  std::vector<std::string> make_LUT(std::string filename, long ninput);
  std::vector<std::string> make_MUX(std::string filenmae, long ninput);
  std::vector<std::string> make_checkonehot(std::string filename, long ninput, bool atmost1 = false);

}

#endif // _CIRCUIT_H_INCLUDED
