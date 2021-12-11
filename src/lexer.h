#ifndef LIN_PARSER_LEXER_H_INCLUDED
#define LIN_PARSER_LEXER_H_INCLUDED

#include <utility>
#include <vector>

#include "error.h"
#include "isa.h"

struct Token {
  enum Type {
    ID_BAD,
    // delims
    DEL_COMMA,
    DEL_SQUARE_BRACKET_L,
    DEL_SQUARE_BRACKET_R,
    // tokens
    ID,
    ID_STRING,
    OPERATION, // delimiter
#define OPLIST(code, mnemonic, format) ID_OP_##code,
    INSTRUCTION_LIST(OPLIST)
#undef OPLIST
        COUNT_TYPES,
  };

  Token() : whole_line(""), bound_l(0), bound_r(0), type(ID_BAD) {}
  Token(std::string line, size_t b_l, size_t b_r, Type t)
      : whole_line(line), bound_l(b_l), bound_r(b_r), type(t) {}

  std::string whole_line;
  size_t bound_l;
  size_t bound_r;
  Type type;
};

typedef std::pair<std::vector<Token>, Error> Tokens;

class Lexer {
public:
  Lexer() {}
  ~Lexer() = default;

  Tokens TokenizeString(const std::string &s);

private:
  bool Eol() const;
  bool LexString();
  void LexTokens();
  void SkipSpace();
  void AnalyzeLine();
  Token::Type LexGetType(size_t bound_l, size_t bound_r) const;

  std::vector<Token> tokens;
  std::string buffer;
  size_t pos;
  size_t end;
  Error err;
};

#endif