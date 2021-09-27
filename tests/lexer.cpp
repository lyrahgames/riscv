#include <iomanip>
#include <iostream>
#include <sstream>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/riscv/assembler/lexer.hpp>

using namespace std;
using namespace lyrahgames::riscv;

SCENARIO("Testing for Lower- and Uppercase Letters") {
  char c = 0;
  char d = 0;
  for (; c < 'A'; ++c) {
    CHECK(!lexer::is_letter(c));
    CHECK(!lexer::is_uppercase_letter(c));
    CHECK(!lexer::is_lowercase_letter(c));
  }
  for (c = 'A', d = 'a'; c <= 'Z'; ++c, ++d) {
    CHECK(lexer::is_letter(c));
    CHECK(lexer::is_uppercase_letter(c));
    CHECK(!lexer::is_lowercase_letter(c));
    CHECK(c == lexer::uppercase_letter(c));
    CHECK(d == lexer::lowercase_letter(c));
  }
  for (; c < 'a'; ++c) {
    CHECK(!lexer::is_letter(c));
    CHECK(!lexer::is_uppercase_letter(c));
    CHECK(!lexer::is_lowercase_letter(c));
  }
  for (c = 'a', d = 'A'; c <= 'z'; ++c, ++d) {
    CHECK(lexer::is_letter(c));
    CHECK(!lexer::is_uppercase_letter(c));
    CHECK(lexer::is_lowercase_letter(c));
    CHECK(d == lexer::uppercase_letter(c));
    CHECK(c == lexer::lowercase_letter(c));
  }
  for (; c != 0; ++c) {
    CHECK(!lexer::is_letter(c));
    CHECK(!lexer::is_uppercase_letter(c));
    CHECK(!lexer::is_lowercase_letter(c));
  }
}

SCENARIO("Testing for and Parsing Character Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!lexer::is_digit(c));
  for (; c <= '9'; ++c, ++n) {
    CHECK(lexer::is_digit(c));
    CHECK(n == lexer::digit(c));
  }
  for (; c != 0; ++c) CHECK(!lexer::is_digit(c));
}

SCENARIO("Testing for and Parsing Binary Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!lexer::binary_digit(c));
  for (; c <= '1'; ++n, ++c) CHECK(n == lexer::binary_digit(c).value());
  for (; c != 0; ++c) CHECK(!lexer::binary_digit(c));
}

SCENARIO("Testing for and Parsing Octal Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!lexer::octal_digit(c));
  for (; c <= '7'; ++n, ++c) CHECK(n == lexer::octal_digit(c).value());
  for (; c != 0; ++c) CHECK(!lexer::octal_digit(c));
}

SCENARIO("Testing for and Parsing Decimal Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!lexer::decimal_digit(c));
  for (; c <= '9'; ++n, ++c) CHECK(n == lexer::decimal_digit(c).value());
  for (; c != 0; ++c) CHECK(!lexer::decimal_digit(c));
}

SCENARIO("Testing for and Parsing Hexadecimal Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!lexer::hexadecimal_digit(c));
  for (; c <= '9'; ++c, ++n) CHECK(n == lexer::hexadecimal_digit(c).value());
  for (; c < 'A'; ++c) CHECK(!lexer::hexadecimal_digit(c));
  for (c = 'A', n = 10; c <= 'F'; ++c, ++n)
    CHECK(n == lexer::hexadecimal_digit(c).value());
  for (; c < 'a'; ++c) CHECK(!lexer::hexadecimal_digit(c));
  for (c = 'a', n = 10; c <= 'f'; ++c, ++n)
    CHECK(n == lexer::hexadecimal_digit(c).value());
  for (; c != 0; ++c) CHECK(!lexer::hexadecimal_digit(c));
}

// SCENARIO("Ignoring Starting Whitespace") {
//   for (auto str : {"test", " test", "\ttest", "  test", "\t\ttest", "\t
//   test",
//                    "\t\t  \ttest"})
//     CHECK(string("test") == ignore_space(str));
// }

SCENARIO("Parsing Integer Literals") {
  for (auto [str, value] : {
           std::pair  //
           {"123", 123},
           {"-456", -456},
           {"-32'450", -32450},
           {"+987", 987},
           {"0x3fa", 0x3fa},
           {"-0x5aCd", -0x5acd},
           {"0o751", 0751},
           {"-0o154", -0154},
           {"0O'1234567", 01234567},
           {"0b01100101011010", 0b01100101011010},
           {"-0b00101011010", -0b00101011010},
           {"-0b'0010'101'1010", -0b00101011010},
       }) {
    CAPTURE(str);
    stringstream stream{str};
    lexer l{stream};
    const auto match = l.int_literal_match();
    CHECK(value == match.value());
  }

  for (auto str : {
           "",      "+",    "-",    "+'",   "'123",  "123'",   "123a",
           "+ 123", "a123", "12a3", "0oc8", "0o193", "0b0121", "0b",
           "0b'",   "0x",   "0x'",  "0o",   "0o'",   "0bB",
       }) {
    CAPTURE(str);
    stringstream stream{str};
    lexer l{stream};
    const auto match = l.int_literal_match();
    CHECK(!match);
  }
}

SCENARIO("Parsing Identifiers") {
  for (auto str : {".", "_", "a", "X", ".L1", "_1", "next", "next_2", "next1",
                   "test_label"}) {
    CAPTURE(str);
    stringstream stream{str};
    lexer l{stream};
    const auto match = l.identifier_match();
    CHECK(str == match.value());
  }

  for (auto str : {"", "1forward", "+test", "test-label"}) {
    CAPTURE(str);
    stringstream stream{str};
    lexer l{stream};
    const auto match = l.identifier_match();
    CHECK(!match);
  }
}

SCENARIO("Getting New Tokens") {
  for (auto [str, tokens] : {
           pair<czstring, vector<lexer::token>>  //
           {"ld a0, 0x10 ( s0)",
            {
                {"ld"},
                {"a0"},
                {','},
                {0x10},
                {'('},
                {"s0"},
                {')'},
                {'\n'},
                {},
            }},
           {"ld a0, 0x10 ( s0)\n"
            "\n"
            "main: // This is a comment\n"
            "      addi t0, t1, 10\n"
            "loop: call /* Multi-line comment */ test\n"
            "\n"
            "\n"
            "      ld ra, 50(sp)\n"
            "/*test:\n"
            "\n"
            "  r*/et\n"
            "  bne a0,a3,loop\n"
            "\t\n"
            "  ",
            {
                {"ld"},   {"a0"}, {','},    {0x10},   {'('},
                {"s0"},   {')'},  {'\n'},  //
                {"main"}, {':'},  {'\n'},  //
                {"addi"}, {"t0"}, {','},    {"t1"},   {','},
                {10},     {'\n'},                              //
                {"loop"}, {':'},  {"call"}, {"test"}, {'\n'},  //
                {"ld"},   {"ra"}, {','},    {50},     {'('},
                {"sp"},   {')'},  {'\n'},  //
                {"et"},   {'\n'},          //
                {"bne"},  {"a0"}, {','},    {"a3"},   {','},
                {"loop"}, {'\n'},  //
                {},
            }},
       }) {
    CAPTURE(str);
    stringstream stream{str};
    lexer l{stream};
    for (const auto& t : tokens) {
      auto success = t == l.next_token();
      // CHECK(t == l.next_token());
      CHECK(success);
    }
    CHECK(!l);
  }
}
