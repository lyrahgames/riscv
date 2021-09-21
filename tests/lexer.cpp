#include <iomanip>
#include <iostream>
#include <sstream>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/riscv/assembler/deprecated.hpp>
#include <lyrahgames/riscv/assembler/lexer.hpp>

using namespace std;
using namespace lyrahgames::riscv;

SCENARIO("Testing for Lower- and Uppercase Letters") {
  char c = 0;
  char d = 0;
  for (; c < 'A'; ++c) {
    CHECK(!is_letter(c));
    CHECK(!is_uppercase_letter(c));
    CHECK(!is_lowercase_letter(c));
  }
  for (c = 'A', d = 'a'; c <= 'Z'; ++c, ++d) {
    CHECK(is_letter(c));
    CHECK(is_uppercase_letter(c));
    CHECK(!is_lowercase_letter(c));
    CHECK(c == uppercase_letter(c));
    CHECK(d == lowercase_letter(c));
  }
  for (; c < 'a'; ++c) {
    CHECK(!is_letter(c));
    CHECK(!is_uppercase_letter(c));
    CHECK(!is_lowercase_letter(c));
  }
  for (c = 'a', d = 'A'; c <= 'z'; ++c, ++d) {
    CHECK(is_letter(c));
    CHECK(!is_uppercase_letter(c));
    CHECK(is_lowercase_letter(c));
    CHECK(d == uppercase_letter(c));
    CHECK(c == lowercase_letter(c));
  }
  for (; c != 0; ++c) {
    CHECK(!is_letter(c));
    CHECK(!is_uppercase_letter(c));
    CHECK(!is_lowercase_letter(c));
  }
}

SCENARIO("Testing for and Parsing Character Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!is_digit(c));
  for (; c <= '9'; ++c, ++n) {
    CHECK(is_digit(c));
    CHECK(n == digit(c));
  }
  for (; c != 0; ++c) CHECK(!is_digit(c));
}

SCENARIO("Testing for and Parsing Binary Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!binary_digit(c));
  for (; c <= '1'; ++n, ++c) CHECK(n == binary_digit(c).value());
  for (; c != 0; ++c) CHECK(!binary_digit(c));
}

SCENARIO("Testing for and Parsing Octal Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!octal_digit(c));
  for (; c <= '7'; ++n, ++c) CHECK(n == octal_digit(c).value());
  for (; c != 0; ++c) CHECK(!octal_digit(c));
}

SCENARIO("Testing for and Parsing Decimal Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!decimal_digit(c));
  for (; c <= '9'; ++n, ++c) CHECK(n == decimal_digit(c).value());
  for (; c != 0; ++c) CHECK(!decimal_digit(c));
}

SCENARIO("Testing for and Parsing Hexadecimal Digits") {
  int n = 0;
  char c = 0;
  for (; c < '0'; ++c) CHECK(!hexadecimal_digit(c));
  for (; c <= '9'; ++c, ++n) CHECK(n == hexadecimal_digit(c).value());
  for (; c < 'A'; ++c) CHECK(!hexadecimal_digit(c));
  for (c = 'A', n = 10; c <= 'F'; ++c, ++n)
    CHECK(n == hexadecimal_digit(c).value());
  for (; c < 'a'; ++c) CHECK(!hexadecimal_digit(c));
  for (c = 'a', n = 10; c <= 'f'; ++c, ++n)
    CHECK(n == hexadecimal_digit(c).value());
  for (; c != 0; ++c) CHECK(!hexadecimal_digit(c));
}

SCENARIO("Ignoring Starting Whitespace") {
  for (auto str : {"test", " test", "\ttest", "  test", "\t\ttest", "\t  test",
                   "\t\t  \ttest"})
    CHECK(string("test") == ignore_space(str));
}

SCENARIO("Parsing Integer Literals") {
  for (auto [str, value] : {
           std::pair{"123", 123},
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
    czstring last;
    CAPTURE(str);
    CAPTURE(last);

    const auto match = int_literal_match(str, last);
    REQUIRE(match);
    CHECK(value == match.value());
  }

  for (auto str : {
           "",      "+",    "-",    "+'",   "'123",  "123'",   "123a",
           "+ 123", "a123", "12a3", "0oc8", "0o193", "0b0121", "0b",
           "0b'",   "0x",   "0x'",  "0o",   "0o'",   "0bB",
       }) {
    czstring last;
    CAPTURE(str);
    CAPTURE(last);

    const auto match = int_literal_match(str, last);
    REQUIRE(!match);
  }
}

SCENARIO("Parsing Identifiers") {
  for (auto str : {".", "_", "a", "X", ".L1", "_1", "next", "next_2", "next1",
                   "test_label"}) {
    czstring last;
    CAPTURE(str);
    CAPTURE(last);

    const auto match = identifier_match(str, last);
    REQUIRE(match);
    CHECK(str == match.value());
  }

  for (auto str : {"", "1forward", "+test", "test-label"}) {
    czstring last;
    CAPTURE(str);
    CAPTURE(last);

    const auto match = identifier_match(str, last);
    REQUIRE(!match);
  }
}

SCENARIO("Getting New Tokens") {
  {
    auto str = "ld a0, 0x10 ( s0)";
    auto next = str;
    auto t = next_token(str, next);
    CHECK(get<identifier>(t) == "ld");
    t = next_token(next, next);
    CHECK(get<identifier>(t) == "a0");
    t = next_token(next, next);
    CHECK(get<separator>(t) == ',');
    t = next_token(next, next);
    CHECK(get<int_literal>(t) == 0x10);
    t = next_token(next, next);
    CHECK(get<separator>(t) == '(');
    t = next_token(next, next);
    CHECK(get<identifier>(t) == "s0");
    t = next_token(next, next);
    CHECK(get<separator>(t) == ')');
    t = next_token(next, next);
    CHECK(holds_alternative<monostate>(t));
  }
}

SCENARIO("Tokenization") {
  {
    auto str = "ld a0, 0x10 ( s0)";
    auto end = str;
    auto tokens = scan(str, end);

    CAPTURE(str);
    CAPTURE(end);

    CHECK(!*end);
    int n = 0;
    CHECK(get<identifier>(tokens[n++]) == "ld");
    CHECK(get<identifier>(tokens[n++]) == "a0");
    CHECK(get<separator>(tokens[n++]) == ',');
    CHECK(get<int_literal>(tokens[n++]) == 0x10);
    CHECK(get<separator>(tokens[n++]) == '(');
    CHECK(get<identifier>(tokens[n++]) == "s0");
    CHECK(get<separator>(tokens[n++]) == ')');
    CHECK(holds_alternative<monostate>(tokens[n++]));
  }
}

SCENARIO("Lexer") {
  auto str =
      "ld a0, 0x10 ( s0)\n"
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
      " ";
  auto stream = stringstream{str};
  lexer lex{stream};
  int i = 0;
  while (lex) {
    lex.scan_line();
    // for (auto it = begin(lex.token_buffer); *it != lexer::token{}; ++it) {
    //   auto& t = *it;
    //   cout << t;
    //   if (t == lexer::token{'\n'}) cout << '\n';
    // }
    cout << setw(5) << i << ": ";
    for (auto& t : lex.token_buffer) cout << t;
    cout << '\n';
    // cout << lex.buffer << '\n';
    // cout << lex.tokens << '\n';
    // cout << '\n';
    // auto t = lex.next_token();
    // cout << t;
    ++i;
  }
  cout << '\n';
}
