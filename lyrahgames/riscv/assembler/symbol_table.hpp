#pragma once
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>
//
#include <lyrahgames/riscv/assembler/token.hpp>

namespace lyrahgames::riscv {

struct symbol_table {
  struct label_data {
    static constexpr size_t invalid = -1;
    size_t address;
  };

  struct instruction_data {
    operand_type_list operands;
  };

  auto label_id(identifier id) -> size_t {
    auto label = std::string(id);
    auto search = label_map.find(label);

    // Label has not been inserted, yet.
    if (search == end(label_map)) {
      auto index = labels.size();
      label_map.emplace(std::move(label), index);
      // Address of label is currently not known.
      labels.push_back({label_data::invalid});
      return index;
    }

    return search->second;
  }

  std::vector<label_data> labels{};
  std::map<std::string, size_t> label_map{};

  const std::map<std::string, int_register> int_register_map{
      {"x0", x0},     {"x1", x1},   {"x2", x2},   {"x3", x3},   {"x4", x4},
      {"x5", x5},     {"x6", x6},   {"x7", x7},   {"x8", x8},   {"x9", x9},
      {"x10", x10},   {"x11", x11}, {"x12", x12}, {"x13", x13}, {"x14", x14},
      {"x15", x15},   {"x16", x16}, {"x17", x17}, {"x18", x18}, {"x19", x19},
      {"x20", x20},   {"x21", x21}, {"x22", x22}, {"x23", x23}, {"x24", x24},
      {"x25", x25},   {"x26", x26}, {"x27", x27}, {"x28", x28}, {"x29", x29},
      {"x30", x30},   {"x31", x31},

      {"zero", zero},

      {"ra", ra},     {"sp", sp},   {"gp", gp},   {"tp", tp},

      {"t0", t0},     {"t1", t1},   {"t2", t2},

      {"fp", fp},

      {"s0", s0},     {"s1", s1},

      {"a0", a0},     {"a1", a1},   {"a2", a2},   {"a3", a3},   {"a4", a4},
      {"a5", a5},     {"a6", a6},   {"a7", a7},

      {"s2", s2},     {"s3", s3},   {"s4", s4},   {"s5", s5},   {"s6", s6},
      {"s7", s7},     {"s8", s8},   {"s9", s9},   {"s10", s10}, {"s11", s11},

      {"t3", t3},     {"t4", t4},   {"t5", t5},   {"t6", t6},
  };

  std::vector<instruction_data> instructions{
      /* add */ {int_r_operand_types},
      /* add */ {int_i_operand_types},
      /* addi */ {int_i_operand_types},
      /* bne */ {int_b_operand_types},
      /* ld */ {int_memory_operand_types},
      /* call */ {{operand_type::label}},
      /* nop */ {{}},
      /* ret */ {{}},
  };
  std::multimap<std::string, size_t> instruction_map{
      {"add", 0}, {"add", 1},  {"addi", 2}, {"bne", 3},
      {"ld", 4},  {"call", 5}, {"nop", 6},  {"ret", 7},
  };
};

inline std::ostream& operator<<(std::ostream& os, const symbol_table& symbols) {
  using namespace std;
  for (auto [label, index] : symbols.label_map)
    os << setw(15) << label << ": " << setw(5) << index << '\n';
  for (size_t i = 0; i < symbols.labels.size(); ++i)
    os << setw(5) << i << ": " << setw(10) << symbols.labels[i].address << '\n';
  return os;
}

}  // namespace lyrahgames::riscv
