#ifndef LIN_PARSER_PARSER_H_INCLUDED
#define LIN_PARSER_PARSER_H_INCLUDED

#include "error.h"
#include "instruction.h"
#include "lexer.h"

#include <cassert>
#include <string>
#include <utility>
#include <iostream>

class Parser {
public:
  Parser() : tokens(), op(Token::Type::ID_BAD), pos(0), err(), ins() {}
  ~Parser() = default;

  std::pair<Ins, Error> Parse(const std::vector<Token> &t) {
    std::cout << t[0].whole_line <<"\n";
    for(auto token : t)
    {
      std::cout << token.bound_l << " " << token.bound_r << "\n";
    }
    tokens = {};
    op = Token::Type::ID_BAD;
    pos = 0;
    err = Error();
    ins = Ins();

    if (t.size() == 0) {
      err = Error("No tokens recieved!", 0, Error::ErrorType::ERR_NO_TOKENS,
                  Error::ErrorClass::ERROR, "");
      return std::pair<Ins, Error>(ins, err);
    }

    tokens = t;

    if (ParseOperation()) {
      ParseOperands();
    }

    return std::pair<Ins, Error>(ins, err);
  }

  std::pair<Ins, Error> Parse(const std::string s) {
    Lexer l;
    const auto res = l.TokenizeString(s);

    if (res.second.err != Error::ErrorType::NONE) {
      return std::pair<Ins, Error>(ins, err);
    }

    return Parse(res.first);
  }

  bool ParseOperation();
  bool ParseOperands();

  bool ParseOperandComma();

  bool ParseOperandINT();

  bool ParseOperandNone();

  Error ShowError() const { return err; }

private:
  std::vector<Token> tokens;
  Token::Type op;
  size_t pos;
  Error err;
  Ins ins;
};

#endif