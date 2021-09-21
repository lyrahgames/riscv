#include <iomanip>
#include <iostream>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/riscv/assembler/deprecated.hpp>
#include <lyrahgames/riscv/assembler/lexer.hpp>
#include <lyrahgames/riscv/assembler/parser.hpp>

using namespace std;
using namespace lyrahgames::riscv;

SCENARIO("Parsing Integer Registers") {
  {
    auto str = "t0 t1 t2 a0 x31 zero sp gp s0";
    auto r = {t0, t1, t2, a0, x31, zero, sp, gp, s0};
    auto end = str;
    auto tokens = scan(str, end);
    CHECK(!*end);

    symbol_table symbols;
    auto it = cbegin(tokens);
    for (auto r : {t0, t1, t2, a0, x31, zero, sp, gp, s0})
      CHECK(r == int_register_match(it, it, symbols).value());
    CHECK(!int_register_match(it, it, symbols));
  }
  {
    auto str = "add addi test call label loop";
    auto end = str;
    auto tokens = scan(str, end);
    CHECK(!*end);

    symbol_table symbols;
    auto it = cbegin(tokens);
    for (const auto& t : tokens)
      CHECK(!int_register_match(token_iterator(&t), it, symbols));
  }
}

SCENARIO("Parsing Memory Addressing Schemes") {
  for (auto [str, mem] : {
           pair{"0x12 ( a0 )", memory_address(a0, 0x12)},
           {"(ra)", memory_address(ra, 0)},
           {"-123( t1 )", memory_address(t1, -123)},
       }) {
    auto end = str;
    auto tokens = scan(str, end);
    CHECK(!*end);

    symbol_table symbols;
    auto it = tokens.cbegin();
    auto m = memory_address_match(it, it, symbols);
    CHECK(holds_alternative<monostate>(*it));
    CHECK(m.value() == mem);
  }
}

SCENARIO("Parsing Instruction Operands") {
  auto str = "bne t0 t1 loop 0o77( sp) ra 12 0xff02 test loop bne";
  auto end = str;
  auto tokens = scan(str, end);
  CHECK(!*end);

  symbol_table symbols{};
  auto it = tokens.cbegin();
  for (auto op : {
           operand_value  //
           {label_id{0}},
           {t0},
           {t1},
           {label_id{1}},
           {memory_address{sp, 077}},
           {ra},
           {immediate{12}},
           {immediate{0xff02}},
           {label_id{2}},
           {label_id{1}},
           {label_id{0}},
       }) {
    CHECK(operand_match(it, it, symbols).value() == op);
  }
  CHECK(holds_alternative<monostate>(*it));
  // cout << symbols;
}

SCENARIO("Parsing Operand Value Lists") {
  for (auto [str, oplist] : {
           pair<czstring, operand_value_list>  //
           {"", {}},
           {"x0", {x0}},
           {"x0, t0", {x0, t0}},
           {"x0, t0, s0", {x0, t0, s0}},
           {"x0, t0, 123", {x0, t0, 123}},
           {"ra, 0x20(tp)", {ra, memory_address{tp, 0x20}}},
           {"a0, a5, loop_start", {a0, a5, label_id{0}}},
       }) {
    CAPTURE(str);

    auto end = str;
    auto tokens = scan(str, end);

    // Check that lexical analysis no errors occured.
    CHECK(!*end);

    symbol_table symbols{};
    auto it = tokens.cbegin();
    auto result = operand_list_match(it, it, symbols);

    CHECK(result.value() == oplist);
  }
}

SCENARIO("Parsing Instructions") {
  for (auto [str, instr] : {
           pair<czstring, instruction>  //
           {"add x0, x1, x2", {0, {x0, x1, x2}}},
           {"add x0, x1, 546", {1, {x0, x1, 546}}},
           {"addi x0, x1, 546", {2, {x0, x1, 546}}},
       }) {
    CAPTURE(str);
    auto end = str;
    auto tokens = scan(str, end);
    CHECK(!*end);
    symbol_table symbols{};
    auto it = tokens.cbegin();
    auto result = instruction_match(it, it, symbols);
    CHECK(result.value() == instr);
  }
}

SCENARIO("Parsing Directives") {
  program prog;

  for (auto str : {
           "main:",
           "addi t0, t1, 10",
           "loop: call test",
           "ld ra, 50(sp)",
           "test:",
           "ret",
           "bne a0,a3,loop",
       }) {
    CAPTURE(str);

    // Scanning
    auto end = str;
    auto tokens = scan(str, end);

    CHECK(!*end);

    // Parsing
    auto it = tokens.cbegin();
    auto success = directive_match(it, it, prog);

    CHECK(success);
    CHECK(holds_alternative<monostate>(*it));
  }

  cout << prog << '\n';
}

// SCENARIO("Parsing") {
//   program prog;
//   auto str =  //
//       "main:\n"
//       "      addi t0, t1, 10\n"
//       "loop: call test\n"
//       "      ld ra, 50(sp)\n"
//       "test:\n"
//       "  ret\n"
//       "  bne a0,a3,loop";

//   auto success = parse(str, prog);
//   CHECK(success);

//   cout << prog << '\n';
// }
