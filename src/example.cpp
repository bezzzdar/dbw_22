#include <iostream>

#include "lexer.h"
#include "parser.h"

void Logic(const Ins &i) {
  switch (i.opcode) {
  case Ins::Opcode::BAN: {
    std::cout << "banned debil number " << i.imms[0] << "\n";
  } break;
  case Ins::Opcode::PROMOTE: {
    std::cout << "promoted debil number " << i.imms[0] << "\n";
  } break;
  case Ins::Opcode::ADD_POINTS: {
    std::cout << "added " << i.imms[0] << " points to debil number "
              << i.imms[1] << "\n";
  } break;
  case Ins::Opcode::SUB_POINTS: {
    std::cout << "taken " << i.imms[0] << " points from debil number "
              << i.imms[1] << "\n";
  } break;
  case Ins::Opcode::KEK: {
    std::cout << "kek " << i.imms[0] << " " << i.imms[1] << " " << i.imms[2]
              << "\n";
  } break;
  case Ins::Opcode::INVALID:
  default: {
    std::cout << "invalid\n";
  } break;
  }
}

int main() {
  Parser p;
  std::pair<Ins, Error> res = {};

  static std::vector<std::string> cases = {
      "ban 100",        "ban, 1 ",        "ban ",        "ban 1 2 ",
      "add_points 100", "add_points, 1 ", "add_points ", "add_points 1 2 ",
      "add_point 100",  "add_point",      "add_points ", "add_points 1, 2 ",
      "kek 1, 2, 3"};

  for (const auto &s : cases) {
    res = p.Parse(s);

    if (res.second.err != Error::ErrorType::NONE) {
      std::cout << res.second.message << "\n";
      continue;
    }

    Logic(res.first);
  }

  return 0;
}