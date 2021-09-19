#pragma once
#include <optional>
//
#include <lyrahgames/riscv/assembler/instruction.hpp>
#include <lyrahgames/riscv/assembler/int_register.hpp>
#include <lyrahgames/riscv/assembler/memory_address.hpp>
#include <lyrahgames/riscv/assembler/operand.hpp>
#include <lyrahgames/riscv/assembler/program.hpp>
#include <lyrahgames/riscv/assembler/symbol_table.hpp>
#include <lyrahgames/riscv/assembler/token.hpp>

namespace lyrahgames::riscv {

inline auto int_register_match(token_iterator it,
                               token_iterator& last,
                               symbol_table& symbols)
    -> std::optional<int_register> {
  if (!std::holds_alternative<identifier>(*it)) return {};
  auto e = symbols.int_register_map.find(std::string(get<identifier>(*it)));
  if (e == end(symbols.int_register_map)) return {};
  last = ++it;
  return e->second;
}

inline auto memory_address_match(token_iterator it,
                                 token_iterator& last,
                                 symbol_table& symbols)
    -> std::optional<memory_address> {
  memory_address result{};
  last = it;

  if (holds_alternative<int_literal>(*last))
    result.offset = get<int_literal>(*last++);

  if (*last != token{'('}) return {};
  ++last;

  const auto r = int_register_match(last, last, symbols);
  if (!r) return {};
  result.base = r.value();

  if (*last != token{')'}) return {};
  ++last;

  return result;
}

inline auto operand_match(token_iterator it,
                          token_iterator& last,
                          symbol_table& symbols)
    -> std::optional<operand_value> {
  // Memory Address
  if (const auto m = memory_address_match(it, last, symbols)) return m.value();
  // Integer Register
  if (const auto m = int_register_match(it, last, symbols)) return m.value();
  // Integer Literal
  if (holds_alternative<int_literal>(*it)) {
    last = it + 1;
    return immediate(get<int_literal>(*it));
  }
  // Label
  if (holds_alternative<identifier>(*it)) {
    last = it + 1;
    return symbols.label_id(get<identifier>(*it));
  }

  return {};
}

inline auto operand_list_match(token_iterator it,
                               token_iterator& last,
                               symbol_table& symbols)
    -> std::optional<operand_value_list> {
  operand_value_list result{};
  // Try to match first operand.
  auto m = operand_match(it, last, symbols);
  // No success means empty operand list.
  if (!m) {
    last = it;
    return result;
  }
  // Loop for possible successive operands.
  do {
    // Store old operand in operand list.
    result.push_back(m.value());
    // If there is no comma separator, the end is reached.
    if (*last != token{','}) return result;
    ++last;
    // When matching a comma, there has to be another operand.
    m = operand_match(last, last, symbols);
  } while (m);
  return {};
}

auto instruction_match(token_iterator it,
                       token_iterator& last,
                       symbol_table& symbols) -> std::optional<instruction> {
  last = it;
  // First, check for an identifier token.
  if (!holds_alternative<identifier>(*last)) return {};
  // Try to find entries in the instruction map with the same mnemonic.
  const auto [first_overload, last_overload] =
      symbols.instruction_map.equal_range(std::string{get<identifier>(*last)});
  if (distance(first_overload, last_overload) == 0) return {};

  ++last;

  // Parse the operand list.
  auto m = operand_list_match(last, last, symbols);
  if (!m) return {};
  auto& operands = m.value();

  // Find the first overload that matches the operand types.
  for (auto p = first_overload; p != last_overload; ++p) {
    const auto overload_index = p->second;
    const auto& overload_types = symbols.instructions[overload_index].operands;
    // Check the size.
    if (operands.size() != overload_types.size()) continue;
    // Check the types.
    bool check = true;
    for (size_t i = 0; i < operands.size(); ++i) {
      if (operands[i].index() != static_cast<size_t>(overload_types[i])) {
        check = false;
        break;
      }
    }
    // Return instruction constructed from valid overload.
    if (check) return {{overload_index, operands}};
  }
  // No overload could be found.
  return {};
}

inline auto label_definition_match(token_iterator it,
                                   token_iterator& last,
                                   program& prog) -> std::optional<label_id> {
  last = it;
  if (!holds_alternative<identifier>(*last)) return {};
  ++last;
  if (*last != token{':'}) return {};
  ++last;
  const auto id = prog.symbols.label_id(get<identifier>(*it));
  prog.symbols.labels[id].address = prog.instructions.size();
  return id;
}

inline bool directive_match(token_iterator it,
                            token_iterator& last,
                            program& prog) {
  auto optlabel = label_definition_match(it, last, prog);
  if (!optlabel) last = it;
  auto optinstr = instruction_match(last, last, prog.symbols);
  if (optinstr) prog.instructions.push_back(optinstr.value());
  return bool(optlabel) || bool(optinstr);
}

}  // namespace lyrahgames::riscv
