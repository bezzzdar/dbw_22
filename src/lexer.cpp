#include <cctype>
#include <string>
#include <unordered_map>

#include "isa.h"
#include "lexer.h"

Token::Type IsDelim(char c) {
  static const std::unordered_map<char, Token::Type> DELIMS = {
      {',', Token::Type::DEL_COMMA},
      {'[', Token::Type::DEL_SQUARE_BRACKET_L},
      {']', Token::Type::DEL_SQUARE_BRACKET_R},
  };

  return (DELIMS.find(c) == DELIMS.end()) ? (Token::Type::ID_BAD)
                                          : (DELIMS.at(c));
}

Token::Type IsOperation(std::string op) {
  static const std::unordered_map<std::string, Token::Type> OPERATIONS = {
#define OPLIST(code, mnemonic, format) {mnemonic, Token::Type::ID_OP_##code},
      INSTRUCTION_LIST(OPLIST)
#undef OPLIST
  };

  return (OPERATIONS.find(op) == OPERATIONS.end()) ? (Token::Type::ID_BAD)
                                                   : (OPERATIONS.at(op));
}

static bool IsQuote(char c) { return c == '"'; }

Tokens Lexer::TokenizeString(const std::string &s) {
  buffer = s;
  pos = 0;
  end = s.size();
  err = Error();
  tokens = {};

  AnalyzeLine();
  return std::pair<std::vector<Token>, Error>(tokens, err);
}

bool Lexer::Eol() const { return pos == end; }

bool Lexer::LexString() {
  bool is_esc_seq = false;
  while (!Eol()) {
    ++pos;

    if (is_esc_seq) {
      is_esc_seq = false;
      continue;
    }

    char c = buffer[pos];

    if (c == '\\') {
      is_esc_seq = true;
    }

    if (IsQuote(c)) {
      break;
    }
  }

  if (!IsQuote(buffer[pos])) {
    err = Error("missing terminatiing character in string sequence!", pos,
                Error::ErrorType::ERR_STRING_NO_TERMINATING_CHARACTER,
                Error::ErrorClass::ERROR, buffer);

    return false;
  }

  ++pos;

  return true;
}

void Lexer::LexTokens() {
  if (Eol()) {
    return;
  }

  while (end > pos && isspace(buffer[end - 1] != 0)) {
    --end;
  }

  while (isspace(buffer[pos] != 0 && !Eol())) {
    ++pos;
  }

  size_t bound_l = 0;
  size_t bound_r = 0;

  for (int i = 0; !Eol(); ++i) {
    bound_l = pos;

    if (IsDelim(buffer[pos]) != Token::Type::ID_BAD) {
      ++pos;
    } else if (IsQuote(buffer[pos])) {
      if (!LexString()) {
        return;
      }
    } else {
      while (!Eol() && IsDelim(buffer[pos]) == Token::Type::ID_BAD &&
             isspace(buffer[pos]) == 0) {
        ++pos;
      }
    }

    bound_r = pos;

    tokens.emplace_back(buffer, bound_l, bound_r, LexGetType(bound_l, bound_r));

    while (isspace(buffer[pos]) != 0 && !Eol()) {
      ++pos;
    }
  }

  return;
}

void Lexer::SkipSpace() {
  while (!Eol() && isspace(buffer[pos]) != 0) {
    ++pos;
  }
}

void Lexer::AnalyzeLine() {
  SkipSpace();
  LexTokens();

  return;
}

Token::Type Lexer::LexGetType(size_t beg, size_t end) const {
  Token::Type t = IsDelim(buffer[beg]);
  if (t != Token::Type::ID_BAD) {
    return t;
  }

  t = IsOperation(buffer.substr(beg, end - beg));
  if (t != Token::Type::ID_BAD) {
    return t;
  }

  if (IsQuote(buffer[beg])) {
    return Token::Type::ID_STRING;
  }

  return Token::Type::ID;
}