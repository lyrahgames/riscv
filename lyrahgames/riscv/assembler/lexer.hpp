#pragma once
#include <cstdint>
//
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
//
#include <iomanip>
#include <iostream>
//
#include <lyrahgames/riscv/assembler/token.hpp>
#include <lyrahgames/riscv/assembler/utility.hpp>

namespace lyrahgames::riscv {

struct lexer {
  using stream = std::istream;
  using char_type = char;
  using char_traits = stream::traits_type;
  using int_type = typename char_traits::int_type;
  using token = std::variant<std::monostate, char, std::string, int>;

  lexer(std::istream& s) : source{s} {}

  operator bool() { return !eof_reached; }

  static constexpr bool is_null(int_type c) { return c == int_type('\0'); }

  static constexpr bool is_eof(int_type c) { return c == char_traits::eof(); }

  static constexpr bool is_end(int_type c) { return is_null(c) || is_eof(c); }

  static constexpr bool is_newline(int_type c) { return c == int_type('\n'); }

  static constexpr bool is_space(int_type c) {
    return (c == int_type(' ')) || (c == int_type('\t'));
  }

  static constexpr bool is_separator(int_type c) {
    return (c == int_type(',')) || (c == int_type('(')) ||
           (c == int_type(')')) || (c == int_type(':')) ||
           (c == int_type('\n'));
  }

  static constexpr bool is_lexeme_end(int_type c) {
    return !c || is_space(c) || is_separator(c);
  }

  static constexpr bool is_lowercase_letter(int_type c) {
    return (int_type('a') <= c) && (c <= int_type('z'));
  }

  static constexpr bool is_uppercase_letter(int_type c) {
    return (int_type('A') <= c) && (c <= int_type('Z'));
  }

  static constexpr auto lowercase_letter(int_type c) -> int_type {
    return c | 0b0010'0000;
  }

  static constexpr auto uppercase_letter(int_type c) -> int_type {
    return c & 0b1101'1111;
  }

  static constexpr bool is_letter(int_type c) {
    return is_uppercase_letter(uppercase_letter(c));
  }

  static constexpr bool is_digit(int_type c) {
    return (int_type('0') <= c) && (c <= int_type('9'));
  }

  static constexpr auto digit(int_type c) -> int_type {
    return c - int_type('0');
  }

  static constexpr auto binary_digit(int_type c) -> std::optional<int_type> {
    if ((int_type('0') == c) || (c == int_type('1'))) return digit(c);
    return {};
  }

  static constexpr auto octal_digit(int_type c) -> std::optional<int_type> {
    if ((int_type('0') <= c) && (c <= int_type('7'))) return digit(c);
    return {};
  }

  static constexpr auto decimal_digit(int_type c) -> std::optional<int_type> {
    if (is_digit(c)) return digit(c);
    return {};
  }

  static constexpr auto hexadecimal_digit(int_type c)
      -> std::optional<int_type> {
    if (is_digit(c)) return digit(c);
    const auto t = uppercase_letter(c);
    if ((int_type('A') <= t) && (t <= int_type('F')))
      return t - int_type('A') + 10;
    return {};
  }

  static constexpr bool is_identifier_start(int_type c) {
    return is_letter(c) || (c == int_type('.')) || (c == int_type('_'));
  }

  static constexpr bool is_identifier_tail(int_type c) {
    return is_identifier_start(c) || is_digit(c);
  }

  static constexpr bool is_binary_prefix(int_type c) {
    return uppercase_letter(c) == int_type('B');
  }

  static constexpr bool is_octal_prefix(int_type c) {
    return uppercase_letter(c) == int_type('O');
  }

  static constexpr bool is_hexadecimal_prefix(int_type c) {
    return uppercase_letter(c) == int_type('X');
  }

  static constexpr bool is_plus(int_type c) { return c == int_type('+'); }

  static constexpr bool is_minus(int_type c) { return c == int_type('-'); }

  static constexpr bool is_number_separator(int_type c) {
    return c == int_type('\'');
  }

  static constexpr bool is_line_comment_start(int_type first, int_type second) {
    return (first == int_type('/')) && (second == int_type('/'));
  }

  static constexpr bool is_line_comment_end(int_type c) {
    return is_newline(c) || is_end(c);
  }

  static constexpr bool is_multiline_comment_start(int_type first,
                                                   int_type second) {
    return (first == int_type('/')) && (second == int_type('*'));
  }

  static constexpr bool is_multiline_comment_end(int_type first,
                                                 int_type second) {
    return (first == int_type('*')) && (second == int_type('/'));
  }

  void ignore_space() {
    while (is_space(source.peek())) source.ignore();
  }

  bool multiline_comment_match() {
    auto first = source.get();
    auto second = source.peek();
    if (!is_multiline_comment_start(first, second)) {
      source.unget();
      return false;
    }

    do {
      first = source.get();
      second = source.peek();
      if (is_end(second))
        throw std::runtime_error("Failed to match end of multiline comment.");
    } while (!is_multiline_comment_end(first, second));

    source.ignore();
    return true;
  }

  bool line_comment_match() {
    auto first = source.get();
    auto second = source.peek();
    if (!is_line_comment_start(first, second)) {
      source.unget();
      return false;
    }

    do {
      source.ignore();
    } while (!is_line_comment_end(source.peek()));

    return true;
  }

  auto identifier_match() -> std::optional<std::string> {
    // First character must not be a digit and string should not be empty.
    if (!is_identifier_start(source.peek())) return {};
    std::string result{};
    result += char(source.get());
    // Following characters can also include digits.
    while (!is_lexeme_end(source.peek())) {
      if (!is_identifier_tail(source.peek())) return {};
      result += char(source.get());
    }
    return result;
  }

  auto int_literal_match() -> std::optional<int> {
    // Sign Extension
    bool sign = false;
    if (is_plus(source.peek()))
      source.ignore();
    else if (is_minus(source.peek())) {
      sign = true;
      source.ignore();
    }

    // First Digit
    if (!is_digit(source.peek())) return {};
    int result = digit(source.get());

    // Number Base Decision
    int base = 10;
    auto base_digit = decimal_digit;
    bool started = true;
    if (!result) {
      if (is_hexadecimal_prefix(source.peek())) {
        base = 16;
        base_digit = hexadecimal_digit;
        source.ignore();
        started = false;
      } else if (is_binary_prefix(source.peek())) {
        base = 2;
        base_digit = binary_digit;
        source.ignore();
        started = false;
      } else if (is_octal_prefix(source.peek())) {
        base = 8;
        base_digit = octal_digit;
        source.ignore();
        started = false;
      }
    }

    // The break condition could be generalized.
    while (!started || !is_lexeme_end(source.peek())) {
      // Allow simple prime character to separate numbers anywhere.
      if (is_number_separator(source.peek())) {
        source.ignore();
        started = false;
        continue;
      }
      // Get the digit and calculate the number.
      const auto digit = base_digit(source.peek());
      if (!digit) return {};
      result = base * result + digit.value();
      source.ignore();
      started = true;
    }
    result = sign ? -result : result;

    return result;
  }

  auto scan_token() -> token {
    while (true) {
      ignore_space();
      if (line_comment_match()) continue;
      if (multiline_comment_match()) continue;
      if (is_separator(source.peek())) return token{char(source.get())};
      if (const auto m = identifier_match()) return m.value();
      if (const auto m = int_literal_match()) return m.value();
      return {};
    }
  }

  void scan_line() {
    token_buffer.clear();
    token t;
    do {
      t = scan_token();
      while ((t != token{}) && (t != token{'\n'})) {
        token_buffer.push_back(t);
        t = scan_token();
      }
    } while (token_buffer.empty() && (t != token{}));
    if (!token_buffer.empty())
      token_buffer.push_back({'\n'});
    else
      eof_reached = true;
    // Sentinel
    token_buffer.push_back({});
  }

  bool eof_reached = false;
  stream& source;
  std::string line_buffer;
  std::vector<token> token_buffer;
};

inline std::ostream& operator<<(std::ostream& os, const lexer::token& t) {
  using namespace std;
  struct {
    void operator()(std::monostate s) const { os << "end"; }
    void operator()(std::string id) const { os << "id: " << id; }
    void operator()(int n) const { os << "int: " << n; }
    void operator()(char c) const {
      if (c == '\n')
        os << "\\n";
      else
        os << c;
    }
    std::ostream& os;
  } printer{os};
  os << " [";
  visit(printer, t);
  return os << "] ";
}

}  // namespace lyrahgames::riscv

#include <lyrahgames/riscv/assembler/lexer.ipp>
