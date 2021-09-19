#pragma once
#include <iostream>
//
#include <lyrahgames/riscv/assembler/int_register.hpp>
#include <lyrahgames/riscv/assembler/utility.hpp>

namespace lyrahgames::riscv {

struct memory_address {
  friend constexpr auto operator<=>(const memory_address&,
                                    const memory_address&) noexcept = default;

  int_register base{};
  immediate offset{};
};

inline std::ostream& operator<<(std::ostream& os, const memory_address& m) {
  return os << m.offset << "(" << m.base << ")";
}

}  // namespace lyrahgames::riscv
