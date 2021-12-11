#include "isa.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

int main() {
  std::unordered_map<std::string, std::vector<std::string>> m = {};

  std::string s = "";
  const char delimiter = '_';
  std::vector<std::string> ss = {};
  size_t pos = 0;

#define OPLIST(code, mnemonic, format)                                         \
  s = #format;                                                                 \
  ss = {};                                                                     \
  pos = 0;                                                                     \
  while ((pos = s.find(delimiter)) != std::string::npos) {                     \
    ss.push_back(s.substr(0, pos));                                            \
    s.erase(0, pos + 1);                                                       \
  }                                                                            \
  ss.push_back(s);                                                             \
  m.emplace(#code, ss);
  INSTRUCTION_LIST(OPLIST)
#undef OPLIST

  FILE *f = fopen("parse_operands.cpp", "w");

  fprintf(f, "#include \"parser.h\"\n"
             "bool Parser::ParseOperands() {\n"
             "switch(op) {\n");

  for (const auto &p : m) {
    fprintf(f, "case Token::Type::ID_OP_%s: {\n", p.first.c_str());

    if (!p.second.empty()) {
      for (size_t i = 0; i < p.second.size(); ++i) {
        if (i > 0) {
          fprintf(f, "if (pos < tokens.size()) {\n"
                     "ParseOperandComma();\n"
                     "} else {\n"
                     "err = Error(\"Expected comma\", tokens[pos - 1].bound_l, "
                     "Error::ErrorType::ERR_BAD_NUMBER_OPERANDS, "
                     "Error::ErrorClass::ERROR, tokens[pos - 1].whole_line);\n"
                     "}\n");
        }

        fprintf(
            f,
            "if (pos < tokens.size()) {\n"
            "ParseOperand%s();\n"
            "} else {\n"
            "err = Error(\"Expected more arguments\", tokens[pos - 1].bound_l, "
            "Error::ErrorType::ERR_BAD_NUMBER_OPERANDS, "
            "Error::ErrorClass::ERROR, tokens[pos - 1].whole_line);\n"
            "}\n",
            p.second[i].c_str());
      }
    }

    fprintf(f, "ParseOperandNone();\n"
               "}\n"
               "break;\n");
  }

  fprintf(f, "default:\n"
             "assert(false);\n"
             "}\n"
             "return pos == tokens.size();");

  fprintf(f, "}\n");

  fclose(f);
}