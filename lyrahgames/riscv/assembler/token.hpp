#pragma once
#include <iomanip>
#include <iostream>
#include <string_view>
#include <variant>
#include <vector>
//
#include <lyrahgames/riscv/assembler/utility.hpp>

namespace lyrahgames::riscv {

using identifier = std::string_view;
using int_literal = immediate;
using separator = char;

using token = std::variant<std::monostate, identifier, int_literal, separator>;
using token_list = std::vector<token>;
using token_iterator = typename std::vector<token>::const_iterator;

inline std::ostream& operator<<(std::ostream& os, const token& t) {
  using namespace std;
  struct {
    void operator()(std::monostate s) const { os << "end"; }
    void operator()(identifier id) const { os << "id: " << id; }
    void operator()(int_literal n) const { os << "int: " << n; }
    void operator()(separator c) const { os << c; }
    std::ostream& os;
  } printer{os};
  os << "<";
  visit(printer, t);
  return os << ">";
}

inline std::ostream& operator<<(std::ostream& os, const token_list& tokens) {
  using namespace std;
  if (tokens.empty()) return os;
  auto it = begin(tokens);
  os << *it++;
  for (; it != end(tokens); ++it) os << " " << *it;
  return os;
}

}  // namespace lyrahgames::riscv
