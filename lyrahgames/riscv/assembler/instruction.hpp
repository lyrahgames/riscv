#pragma once
#include <iomanip>
#include <iostream>
//
#include <lyrahgames/riscv/assembler/operand.hpp>

namespace lyrahgames::riscv {

struct instruction {
  friend inline auto operator<=>(const instruction&,
                                 const instruction&) noexcept = default;

  size_t id;
  operand_value_list operands{};
};

inline std::ostream& operator<<(std::ostream& os, const instruction& instr) {
  using namespace std;
  return os << setw(5) << instr.id << " " << instr.operands;
}

}  // namespace lyrahgames::riscv
