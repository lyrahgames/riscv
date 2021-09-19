#pragma once
#include <vector>
//
#include <iomanip>
#include <iostream>
//
#include <lyrahgames/riscv/assembler/int_register.hpp>
#include <lyrahgames/riscv/assembler/memory_address.hpp>
#include <lyrahgames/riscv/assembler/utility.hpp>

namespace lyrahgames::riscv {

enum class operand_type : size_t {
  int_register = 0,
  int_literal,
  memory_address,
  label,
};

using label_id = size_t;

using operand_value =
    std::variant<int_register, immediate, memory_address, size_t>;
using operand_value_list = std::vector<operand_value>;

using operand_type_list = std::vector<operand_type>;
const operand_type_list int_r_operand_types{operand_type::int_register,
                                            operand_type::int_register,
                                            operand_type::int_register};
const operand_type_list int_i_operand_types{operand_type::int_register,
                                            operand_type::int_register,
                                            operand_type::int_literal};
const operand_type_list int_b_operand_types{operand_type::int_register,
                                            operand_type::int_register,
                                            operand_type::label};
const operand_type_list int_memory_operand_types{operand_type::int_register,
                                                 operand_type::memory_address};

inline std::ostream& operator<<(std::ostream& os, const operand_value& op) {
  using namespace std;
  struct {
    void operator()(int_register id) const { os << id; }
    void operator()(immediate n) const { os << n; }
    void operator()(memory_address c) const { os << c; }
    void operator()(size_t c) const { os << '$' << c; }
    std::ostream& os;
  } printer{os};
  visit(printer, op);
  return os;
}

inline std::ostream& operator<<(std::ostream& os,
                                const operand_value_list& oplist) {
  using namespace std;
  if (oplist.empty()) return os;
  auto it = begin(oplist);
  os << *it++;
  for (; it != end(oplist); ++it) os << ", " << *it;
  return os;
}

}  // namespace lyrahgames::riscv
