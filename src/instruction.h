#ifndef LIN_PARSER_INSTRUCTION_H_INCLUDED
#define LIN_PARSER_INSTRUCTION_H_INCLUDED

#include <string>
#include <vector>

#include "isa.h"

struct Ins {
  enum Opcode {
#define OPLIST(code, mnemonic, format) code,
    INSTRUCTION_LIST(OPLIST)
#undef OPLIST
        INVALID,
    NUM_OPCODES = INVALID,
  };

  // move to std::variant if floats are needed
  typedef int64_t int_t;

  Opcode opcode = INVALID;
  std::vector<std::string> ids;
  std::vector<int_t> imms;
};

#endif
