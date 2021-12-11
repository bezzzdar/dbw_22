#ifndef LIN_PARSER_ERROR_H_INCLUDED
#define LIN_PARSER_ERROR_H_INCLUDED

#include <string>

struct Error {
  enum ErrorClass {
    WARNING,
    ERROR,
  };

  enum ErrorType {
    NONE,
    // lexer
    ERR_STRING_NO_TERMINATING_CHARACTER,
    // parser
    ERR_NO_TOKENS,
    ERR_BAD_STRING,
    ERR_BAD_INTEGER,
    ERR_BAD_OPERAND,
    ERR_BAD_OPERATION_NAME,
    ERR_BAD_NUMBER_OPERANDS,
    // waring
  };

  Error() : type(WARNING), err(NONE), whole_line(""), pos(0), message("") {}
  Error(std::string msg, size_t p, ErrorType err_type, ErrorClass err_class,
        std::string buff)
      : type(err_class), err(err_type), whole_line(buff), pos(p), message(msg) {
  }

  ErrorClass type;
  ErrorType err;
  std::string whole_line;
  size_t pos;
  std::string message;
};

#endif