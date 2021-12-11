#include "parser.h"

static Ins::Opcode TokenToOpcode(Token::Type id) {
  assert(id > Token::Type::OPERATION && id < Token::Type::COUNT_TYPES);

  return static_cast<Ins::Opcode>(id - Token::Type::OPERATION - 1);
}

bool Parser::ParseOperation() {
  if (pos == tokens.size()) {
    return false;
  }

  auto t = tokens[pos];

  if (t.type > Token::Type::OPERATION && t.type < Token::Type::COUNT_TYPES) {
    ins.opcode = TokenToOpcode(t.type);
    op = t.type;

    ++pos;

    return true;
  }

  err = Error("Invalid operation", t.bound_l,
              Error::ErrorType::ERR_BAD_OPERATION_NAME,
              Error::ErrorClass::ERROR, t.whole_line);

  return false;
}

bool Parser::ParseOperandComma() {
  if (err.err != Error::ErrorType::NONE) {
    return false;
  }

  if (tokens[pos].type != Token::Type::DEL_COMMA) {
    err = Error("Expected comma", tokens[pos].bound_l,
                Error::ErrorType::ERR_BAD_NUMBER_OPERANDS,
                Error::ErrorClass::ERROR, tokens[pos].whole_line);

    return false;
  }

  pos++;

  return true;
}

bool Parser::ParseOperandINT() {
  if (err.err != Error::ErrorType::NONE) {
    return false;
  }

  if (tokens[pos].type != Token::Type::ID) {
    err = Error("Expected immediate", tokens[pos].bound_l,
                Error::ErrorType::ERR_BAD_OPERAND, Error::ErrorClass::ERROR,
                tokens[pos].whole_line);
    return false;
  }

  const std::string p = tokens[pos].whole_line.substr(
      tokens[pos].bound_l, tokens[pos].bound_r - tokens[pos].bound_l);

  int64_t n = 0;

  for (const auto &i : p) {
    if (!(i <= '9' && i >= '0')) {
      err = Error("Expected integer", tokens[pos].bound_l,
                  Error::ErrorType::ERR_BAD_INTEGER, Error::ErrorClass::ERROR,
                  tokens[pos].whole_line);
      return false;
    }
  }

  if (p.size() == 1) {
    n = p[0] - '0';
  } else {
    n = strtoull(p.data(), nullptr, 10);
  }

  ins.imms.push_back(n);

  ++pos;

  return true;
}

bool Parser::ParseOperandNone() {
  if (err.err != Error::ErrorType::NONE) {
    return false;
  }

  if (pos < tokens.size()) {
    err = Error("Invalid number of operands", tokens[pos].bound_l,
                Error::ErrorType::ERR_BAD_NUMBER_OPERANDS,
                Error::ErrorClass::ERROR, tokens[pos].whole_line);
    --pos;
  }

  return true;
}