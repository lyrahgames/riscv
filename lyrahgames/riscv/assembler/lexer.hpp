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
#include <lyrahgames/riscv/assembler/int_register.hpp>
#include <lyrahgames/riscv/assembler/memory_address.hpp>
#include <lyrahgames/riscv/assembler/operand.hpp>
#include <lyrahgames/riscv/assembler/token.hpp>
#include <lyrahgames/riscv/assembler/utility.hpp>

namespace lyrahgames::riscv {

constexpr bool is_space(char c) {
  return (c == ' ') || (c == '\t');
}

constexpr bool is_separator(char c) {
  return (c == ',') || (c == '(') || (c == ')') || (c == ':');
}

constexpr bool is_lexeme_end(char c) {
  return !c || is_space(c) || is_separator(c);
}

constexpr bool is_lowercase_letter(char c) {
  return ('a' <= c) && (c <= 'z');
}

constexpr bool is_uppercase_letter(char c) {
  return ('A' <= c) && (c <= 'Z');
}

constexpr char lowercase_letter(char c) {
  return c | 0b0010'0000;
}

constexpr char uppercase_letter(char c) {
  return c & 0b1101'1111;
}

constexpr bool is_letter(char c) {
  return is_uppercase_letter(uppercase_letter(c));
}

constexpr bool is_digit(char c) {
  return ('0' <= c) && (c <= '9');
}

constexpr auto digit(char c) -> int {
  return c - '0';
}

constexpr auto binary_digit(char c) -> std::optional<int> {
  if (('0' == c) || (c == '1')) return digit(c);
  return {};
}

constexpr auto octal_digit(char c) -> std::optional<int> {
  if (('0' <= c) && (c <= '7')) return digit(c);
  return {};
}

constexpr auto decimal_digit(char c) -> std::optional<int> {
  if (is_digit(c)) return digit(c);
  return {};
}

constexpr auto hexadecimal_digit(char c) -> std::optional<int> {
  if (is_digit(c)) return digit(c);
  const auto t = uppercase_letter(c);
  if (('A' <= t) && (t <= 'F')) return t - 'A' + 10;
  return {};
}

constexpr auto ignore_space(czstring str) -> czstring {
  while (is_space(*str)) ++str;
  return str;
}

/// Parses given string into an integer literal.
/// If successful, the optional result contains the actual integer value.
/// If the given string could not be parsed, the optional result contains nothing.
/// The iterator reference 'end' contains the position when parsing stopped.
/// It may be used to further parse other elements or do some error handling.
constexpr auto int_literal_match(czstring_iterator str, czstring_iterator& end)
    -> std::optional<int>;

/// Parses given string into an identifier.
/// If successful, the optional result contains a 'string_view'
/// to the underlying string.
constexpr auto identifier_match(czstring_iterator str, czstring_iterator& end)
    -> std::optional<std::string_view>;

/// Tries to match any separator and returns it.
constexpr auto separator_match(czstring_iterator it, czstring_iterator& end)
    -> std::optional<char>;

/// Basic function for generating the next following token of a given string.
///
constexpr auto next_token(czstring_iterator it, czstring_iterator& next)
    -> token {
  token result{};
  if (const auto m = separator_match(it, next))
    result = m.value();
  else if (const auto m = identifier_match(it, next))
    result = m.value();
  else if (const auto m = int_literal_match(it, next))
    result = m.value();
  next = ignore_space(next);
  return result;
}

/// Generates a whole token stream of a given string.
///
inline auto scan(czstring str, czstring_iterator& next) -> token_list {
  token_list result{};
  str = ignore_space(str);

  auto t = next_token(str, next);
  while (!std::holds_alternative<std::monostate>(t)) {
    result.push_back(t);
    t = next_token(next, next);
  }
  // Sentinel for Token Stream if Scanning did not result in errors.
  result.push_back({});
  return result;
}

#include <lyrahgames/riscv/assembler/lexer.ipp>

}  // namespace lyrahgames::riscv
