#pragma once
#include <map>
#include <string>
#include <string_view>
#include <vector>
//
#include <bitset>
#include <iomanip>
#include <iostream>
//
#include <lyrahgames/riscv/assembler/utility.hpp>

namespace lyrahgames::riscv {

struct int_register {
  friend constexpr auto operator<=>(const int_register&,
                                    const int_register&) noexcept = default;
  uint8_t code{};
};

constexpr int_register x0{0};
constexpr int_register x1{1};
constexpr int_register x2{2};
constexpr int_register x3{3};
constexpr int_register x4{4};
constexpr int_register x5{5};
constexpr int_register x6{6};
constexpr int_register x7{7};
constexpr int_register x8{8};
constexpr int_register x9{9};
constexpr int_register x10{10};
constexpr int_register x11{11};
constexpr int_register x12{12};
constexpr int_register x13{13};
constexpr int_register x14{14};
constexpr int_register x15{15};
constexpr int_register x16{16};
constexpr int_register x17{17};
constexpr int_register x18{18};
constexpr int_register x19{19};
constexpr int_register x20{20};
constexpr int_register x21{21};
constexpr int_register x22{22};
constexpr int_register x23{23};
constexpr int_register x24{24};
constexpr int_register x25{25};
constexpr int_register x26{26};
constexpr int_register x27{27};
constexpr int_register x28{28};
constexpr int_register x29{29};
constexpr int_register x30{30};
constexpr int_register x31{31};

constexpr auto zero = x0;

constexpr auto ra = x1;
constexpr auto sp = x2;
constexpr auto gp = x3;
constexpr auto tp = x4;

constexpr auto t0 = x5;
constexpr auto t1 = x6;
constexpr auto t2 = x7;

constexpr auto fp = x8;

constexpr auto s0 = x8;
constexpr auto s1 = x9;

constexpr auto a0 = x10;
constexpr auto a1 = x11;
constexpr auto a2 = x12;
constexpr auto a3 = x13;
constexpr auto a4 = x14;
constexpr auto a5 = x15;
constexpr auto a6 = x16;
constexpr auto a7 = x17;

constexpr auto s2 = x18;
constexpr auto s3 = x19;
constexpr auto s4 = x20;
constexpr auto s5 = x21;
constexpr auto s6 = x22;
constexpr auto s7 = x23;
constexpr auto s8 = x24;
constexpr auto s9 = x25;
constexpr auto s10 = x26;
constexpr auto s11 = x27;

constexpr auto t3 = x28;
constexpr auto t4 = x29;
constexpr auto t5 = x30;
constexpr auto t6 = x31;

inline std::ostream& operator<<(std::ostream& os, int_register r) {
  return os << 'x' << int(r.code) << "[" << std::bitset<5>(r.code) << "]";
}

struct memory_address {
  friend constexpr auto operator<=>(const memory_address&,
                                    const memory_address&) noexcept = default;

  int_register base{};
  immediate offset{};
};

inline std::ostream& operator<<(std::ostream& os, const memory_address& m) {
  return os << m.offset << "(" << m.base << ")";
}

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
