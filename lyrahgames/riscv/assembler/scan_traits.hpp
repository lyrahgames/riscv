#pragma once
#include <optional>
#include <string>

namespace lyrahgames::riscv {

template <typename T>
struct scan_traits;

template <>
struct scan_traits<char> {
  using char_type = char;
  using char_traits = std::char_traits<char>;
  using int_type = typename char_traits::int_type;

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
    return is_newline(c) || is_null(c) || is_eof(c);
  }

  static constexpr bool is_multiline_comment_start(int_type first,
                                                   int_type second) {
    return (first == int_type('/')) && (second == int_type('*'));
  }

  static constexpr bool is_multiline_comment_end(int_type first,
                                                 int_type second) {
    return (first == int_type('*')) && (second == int_type('/'));
  }
};

}  // namespace lyrahgames::riscv
