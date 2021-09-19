#pragma once
#include <vector>
//
#include <iomanip>
#include <iostream>
//
#include <lyrahgames/riscv/assembler/instruction.hpp>
#include <lyrahgames/riscv/assembler/symbol_table.hpp>

namespace lyrahgames::riscv {

struct program {
  friend inline auto operator<=>(const program&,
                                 const program&) noexcept = default;

  symbol_table symbols{};
  std::vector<instruction> instructions{};
};

inline std::ostream& operator<<(std::ostream& os, const program& p) {
  using namespace std;
  os << p.symbols << '\n';
  for (size_t pc = 0; auto& instr : p.instructions) {
    os << setw(5) << pc << ": " << instr << '\n';
    ++pc;
  }
  return os;
}

}  // namespace lyrahgames::riscv
