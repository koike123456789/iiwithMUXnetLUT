// ./src/parser/blif_parser.cpp generated by reflex 1.4.4 from ./src/parser/blif_parser.l

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  OPTIONS USED                                                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#define bl_REFLEX_OPTION_fast                true
#define bl_REFLEX_OPTION_header_file         "./include/parser/blif_parser.h"
#define bl_REFLEX_OPTION_lex                 bl_lex
#define bl_REFLEX_OPTION_lexer               BL_LEXER
#define bl_REFLEX_OPTION_namespace           blifparser
#define bl_REFLEX_OPTION_outfile             "./src/parser/blif_parser.cpp"
#define bl_REFLEX_OPTION_prefix              bl_

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  SECTION 1: %top{ user code %}                                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#line 12 "./src/parser/blif_parser.l"

#include "circuit.h"
using namespace nodecircuit;


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  REGEX MATCHER                                                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <reflex/matcher.h>

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  ABSTRACT LEXER CLASS                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <reflex/abslexer.h>

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  LEXER CLASS                                                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

namespace blifparser {

class BL_LEXER : public reflex::AbstractLexer<reflex::Matcher> {
#line 17 "./src/parser/blif_parser.l"

Node* current_node;
NodeVector cur_nodes;
int mode;
virtual int wrap() { return 1; }
public:
Circuit* circuit;

 public:
  typedef reflex::AbstractLexer<reflex::Matcher> AbstractBaseLexer;
  BL_LEXER(
      const reflex::Input& input = reflex::Input(),
      std::ostream&        os    = std::cout)
    :
      AbstractBaseLexer(input, os)
  {
#line 26 "./src/parser/blif_parser.l"

circuit = NULL;
current_node = NULL;
mode = 255; // 128:start 129:output 130:input 131:gate 132:value 133:latchi 134:latcho 254:nothing 255:finish

  }
  static const int INITIAL = 0;
  virtual int bl_lex();
  int bl_lex(
      const reflex::Input& input,
      std::ostream        *os = NULL)
  {
    in(input);
    if (os)
      out(*os);
    return bl_lex();
  }
};

} // namespace blifparser

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  SECTION 1: %{ user code %}                                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#line 1 "./src/parser/blif_parser.l"
// simple blif parser, without error recover, and without comment detection


#include <cstdio>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;



////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  SECTION 2: rules                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

namespace blifparser {
extern void reflex_code_INITIAL(reflex::Matcher&);
} // namespace blifparser

int blifparser::BL_LEXER::bl_lex()
{
  static const reflex::Pattern PATTERN_INITIAL(reflex_code_INITIAL);
  if (!has_matcher())
  {
    matcher(new Matcher(PATTERN_INITIAL, stdinit(), this));
  }
  while (true)
  {
        switch (matcher().scan())
        {
          case 0:
            if (matcher().at_end())
            {
              return int();
            }
            else
            {
              out().put(matcher().input());
            }
            break;
          case 1: // rule at line 47: (?:\Q.end\E)
#line 47 "./src/parser/blif_parser.l"
mode = 255;
            break;
          case 2: // rule at line 48: (?:\Q.model\E)
#line 48 "./src/parser/blif_parser.l"
mode = 128;
            break;
          case 3: // rule at line 49: (?:\Q.outputs\E)
#line 49 "./src/parser/blif_parser.l"
mode = 129;
            break;
          case 4: // rule at line 50: (?:\Q.inputs\E)
#line 50 "./src/parser/blif_parser.l"
mode = 130;
            break;
          case 5: // rule at line 51: (?:\Q.latch\E)
#line 51 "./src/parser/blif_parser.l"
mode = 133;
            break;
          case 6: // rule at line 52: (?:\Q.names\E)
#line 52 "./src/parser/blif_parser.l"
{
              mode = 131;
              cur_nodes.clear();
            }


            break;
          case 7: // rule at line 58: (?:[A-Za-z][0-9A-Z_a-z]*(?:(?:\Q[\E)[0-9]+(?:\Q]\E))*)
#line 58 "./src/parser/blif_parser.l"
switch (mode) {
          case 128:
            circuit->name = text();
            mode = 254;
            break;
          case 129:
            current_node = new BlifNode;
            current_node->name = text();
            current_node->is_output = true;
            circuit->outputs.push_back(current_node);
            circuit->all_nodes.push_back(current_node);
            circuit->all_nodes_map[current_node->name] = current_node;
            break;
          case 130:
            current_node = new BlifNode;
            current_node->name = text();
            current_node->is_input = true;
            circuit->inputs.push_back(current_node);
            circuit->all_nodes.push_back(current_node);
            circuit->all_nodes_map[current_node->name] = current_node;
            break;
          case 133:
            current_node = circuit->GetNode(string(text()));
            if (!current_node) {
              current_node = new BlifNode;
              current_node->name = text();
              circuit->all_nodes.push_back(current_node);
              circuit->all_nodes_map[current_node->name] = current_node;
            }
            cur_nodes.push_back(current_node);
            mode = 134;
            break;
          case 134:
            current_node = circuit->GetNode(string(text()));
            if (!current_node) {
              current_node = new BlifNode;
              current_node->name = text();
              circuit->all_nodes.push_back(current_node);
              circuit->all_nodes_map[current_node->name] = current_node;
            }
            current_node->is_ff =true;
            current_node->type = NODE_DFF;
            circuit->ffs.push_back(current_node);
            current_node->inputs.push_back(cur_nodes[0]);
            cur_nodes[0]->outputs.push_back(current_node);
            cur_nodes.clear();
            mode = 254;
            break;
          case 131:
            current_node = circuit->GetNode(string(text()));
            if (!current_node) {
              current_node = new BlifNode;
              current_node->name = text();
              circuit->all_nodes.push_back(current_node);
              circuit->all_nodes_map[current_node->name] = current_node;
            }
            cur_nodes.push_back(current_node);
            break;
        }

            break;
          case 8: // rule at line 117: (?:(?:[01])(?:\r?\n)|(?:[\x2d01][\x2d01]+)(?:[\x20])(?:[01])(?:\r?\n)|(?:[01])(?:[\x20])(?:[01])(?:\r?\n))
#line 117 "./src/parser/blif_parser.l"
if (cur_nodes.size() == 0)
          mode = 254;
        else {
          if (mode == 131) {
            current_node = cur_nodes[cur_nodes.size()-1];
            for (int i = 0; i < cur_nodes.size()-1; i++) {
              Node* node = cur_nodes[i];
              current_node->inputs.push_back(node);
              node->outputs.push_back(current_node);
            }
            mode = 132;
          }
          if (mode = 132) {
            string str(text());
            while (str[str.size()-1] == '\n' || str[str.size()-1] == '\r')
              str.resize(str.size()-1);
            ((BlifNode*)current_node)->str_values.push_back(str);
          }
        }

            break;
          case 9: // rule at line 137: .
#line 137 "./src/parser/blif_parser.l"
mode=mode;

            break;
        }
  }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  TABLES                                                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <reflex/matcher.h>

#if defined(OS_WIN)
#pragma warning(disable:4102)
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-label"
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-label"
#endif

namespace blifparser {

void reflex_code_INITIAL(reflex::Matcher& m)
{
  int c0 = 0, c1 = 0;
  m.FSM_INIT(c1);

S0:
  m.FSM_FIND();
  c0 = c1, c1 = m.FSM_CHAR();
  if ('a' <= c1 && c1 <= 'z') goto S16;
  if ('A' <= c1 && c1 <= 'Z') goto S16;
  if ('0' <= c1 && c1 <= '1') goto S23;
  if (c1 == '.') goto S8;
  if (c1 == '-') goto S30;
  if ('\v' <= c1) goto S34;
  if ('\n' <= c1) return m.FSM_HALT(c1);
  if (0 <= c1 && c1 <= '\t') goto S34;
  return m.FSM_HALT(c1);

S8:
  m.FSM_TAKE(9);
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'o') goto S40;
  if (c1 == 'n') goto S46;
  if (c1 == 'm') goto S38;
  if (c1 == 'l') goto S44;
  if (c1 == 'i') goto S42;
  if (c1 == 'e') goto S36;
  return m.FSM_HALT(c1);

S16:
  m.FSM_TAKE(7);
  c0 = c1, c1 = m.FSM_CHAR();
  if ('a' <= c1 && c1 <= 'z') goto S16;
  if (c1 == '_') goto S16;
  if (c1 == '[') goto S48;
  if ('A' <= c1 && c1 <= 'Z') goto S16;
  if ('0' <= c1 && c1 <= '9') goto S16;
  return m.FSM_HALT(c1);

S23:
  m.FSM_TAKE(9);
  c0 = c1, c1 = m.FSM_CHAR();
  if ('0' <= c1 && c1 <= '1') goto S56;
  if (c1 == '-') goto S56;
  if (c1 == ' ') goto S52;
  if (c1 == '\r') goto S54;
  if (c1 == '\n') goto S50;
  return m.FSM_HALT(c1);

S30:
  m.FSM_TAKE(9);
  c0 = c1, c1 = m.FSM_CHAR();
  if ('0' <= c1 && c1 <= '1') goto S56;
  if (c1 == '-') goto S56;
  return m.FSM_HALT(c1);

S34:
  m.FSM_TAKE(9);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);

S36:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'n') goto S60;
  return m.FSM_HALT(c1);

S38:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'o') goto S62;
  return m.FSM_HALT(c1);

S40:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'u') goto S64;
  return m.FSM_HALT(c1);

S42:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'n') goto S66;
  return m.FSM_HALT(c1);

S44:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'a') goto S68;
  return m.FSM_HALT(c1);

S46:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'a') goto S70;
  return m.FSM_HALT(c1);

S48:
  c0 = c1, c1 = m.FSM_CHAR();
  if ('0' <= c1 && c1 <= '9') goto S72;
  return m.FSM_HALT(c1);

S50:
  m.FSM_TAKE(8);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);

S52:
  c0 = c1, c1 = m.FSM_CHAR();
  if ('0' <= c1 && c1 <= '1') goto S75;
  return m.FSM_HALT(c1);

S54:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == '\n') goto S50;
  return m.FSM_HALT(c1);

S56:
  c0 = c1, c1 = m.FSM_CHAR();
  if ('0' <= c1 && c1 <= '1') goto S56;
  if (c1 == '-') goto S56;
  if (c1 == ' ') goto S78;
  return m.FSM_HALT(c1);

S60:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'd') goto S80;
  return m.FSM_HALT(c1);

S62:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'd') goto S82;
  return m.FSM_HALT(c1);

S64:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 't') goto S84;
  return m.FSM_HALT(c1);

S66:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'p') goto S86;
  return m.FSM_HALT(c1);

S68:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 't') goto S88;
  return m.FSM_HALT(c1);

S70:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'm') goto S90;
  return m.FSM_HALT(c1);

S72:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == ']') goto S92;
  if ('0' <= c1 && c1 <= '9') goto S72;
  return m.FSM_HALT(c1);

S75:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == '\r') goto S95;
  if (c1 == '\n') goto S50;
  return m.FSM_HALT(c1);

S78:
  c0 = c1, c1 = m.FSM_CHAR();
  if ('0' <= c1 && c1 <= '1') goto S97;
  return m.FSM_HALT(c1);

S80:
  m.FSM_TAKE(1);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);

S82:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'e') goto S100;
  return m.FSM_HALT(c1);

S84:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'p') goto S102;
  return m.FSM_HALT(c1);

S86:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'u') goto S104;
  return m.FSM_HALT(c1);

S88:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'c') goto S106;
  return m.FSM_HALT(c1);

S90:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'e') goto S108;
  return m.FSM_HALT(c1);

S92:
  m.FSM_TAKE(7);
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == '[') goto S48;
  return m.FSM_HALT(c1);

S95:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == '\n') goto S50;
  return m.FSM_HALT(c1);

S97:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == '\r') goto S110;
  if (c1 == '\n') goto S50;
  return m.FSM_HALT(c1);

S100:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'l') goto S112;
  return m.FSM_HALT(c1);

S102:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'u') goto S114;
  return m.FSM_HALT(c1);

S104:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 't') goto S116;
  return m.FSM_HALT(c1);

S106:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 'h') goto S118;
  return m.FSM_HALT(c1);

S108:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 's') goto S120;
  return m.FSM_HALT(c1);

S110:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == '\n') goto S50;
  return m.FSM_HALT(c1);

S112:
  m.FSM_TAKE(2);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);

S114:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 't') goto S122;
  return m.FSM_HALT(c1);

S116:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 's') goto S124;
  return m.FSM_HALT(c1);

S118:
  m.FSM_TAKE(5);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);

S120:
  m.FSM_TAKE(6);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);

S122:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 's') goto S126;
  return m.FSM_HALT(c1);

S124:
  m.FSM_TAKE(4);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);

S126:
  m.FSM_TAKE(3);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);
}

} // namespace blifparser

