#pragma once
#include <optional>
//
#include <lyrahgames/riscv/assembler/token.hpp>
#include <lyrahgames/riscv/assembler/utility.hpp>

namespace lyrahgames::riscv {

constexpr bool is_space(char c) { return (c == ' ') || (c == '\t'); }

constexpr bool is_separator(char c) {
  return (c == ',') || (c == '(') || (c == ')') || (c == ':') || (c == '\n');
}

constexpr bool is_lexeme_end(char c) {
  return !c || is_space(c) || is_separator(c);
}

constexpr bool is_lowercase_letter(char c) { return ('a' <= c) && (c <= 'z'); }

constexpr bool is_uppercase_letter(char c) { return ('A' <= c) && (c <= 'Z'); }

constexpr char lowercase_letter(char c) { return c | 0b0010'0000; }

constexpr char uppercase_letter(char c) { return c & 0b1101'1111; }

constexpr bool is_letter(char c) {
  return is_uppercase_letter(uppercase_letter(c));
}

constexpr bool is_digit(char c) { return ('0' <= c) && (c <= '9'); }

constexpr auto digit(char c) -> int { return c - '0'; }

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

constexpr bool is_identifier_start(char c) {
  return is_letter(c) || (c == '.') || (c == '_');
}

constexpr bool is_identifier_tail(char c) {
  return is_identifier_start(c) || is_digit(c);
}

constexpr auto ignore_space(czstring str) -> czstring {
  while (is_space(*str)) ++str;
  return str;
}

/// Parses given string into an integer literal.
/// If successful, the optional result contains the actual integer value.
/// If the given string could not be parsed, the optional result contains
/// nothing. The iterator reference 'end' contains the position when parsing
/// stopped. It may be used to further parse other elements or do some error
/// handling.
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
  next = ignore_space(it);
  if (const auto m = separator_match(next, next)) return m.value();
  if (const auto m = identifier_match(next, next)) return m.value();
  if (const auto m = int_literal_match(next, next)) return m.value();
  return {};
}

/// Generates a whole token stream of a given string.
///
inline auto scan(czstring str, czstring_iterator& next) -> token_list {
  token_list result{};
  token t{};
  next = str;
  do {
    t = next_token(next, next);
    result.push_back(t);
  } while (t != token{});
  return result;
}

}  // namespace lyrahgames::riscv

// Implementation
namespace lyrahgames::riscv {

constexpr auto int_literal_match(czstring_iterator str, czstring_iterator& it)
    -> std::optional<int> {
  it = str;

  // Sign Extension
  bool sign = false;
  if (*it == '+')
    ++it;
  else if (*it == '-') {
    sign = true;
    ++it;
  }

  // First Digit
  if (!is_digit(*it)) return {};
  int result = digit(*it++);

  // Number Base Decision
  int base = 10;
  auto base_digit = decimal_digit;
  bool started = true;
  if (!result) {
    if ((*it == 'x') || (*it == 'X')) {
      base = 16;
      base_digit = hexadecimal_digit;
      ++it;
      started = false;
    } else if ((*it == 'b') || (*it == 'B')) {
      base = 2;
      base_digit = binary_digit;
      ++it;
      started = false;
    } else if ((*it == 'o') || (*it == 'O')) {
      base = 8;
      base_digit = octal_digit;
      ++it;
      started = false;
    }
  }

  // The break condition could be generalized.
  while (!started || !is_lexeme_end(*it)) {
    // Allow simple prime character to separate numbers anywhere.
    if (*it == '\'') {
      ++it;
      started = false;
      continue;
    }
    // Get the digit and calculate the number.
    const auto digit = base_digit(*it);
    if (!digit) return {};
    result = base * result + digit.value();
    ++it;
    started = true;
  }
  result = sign ? -result : result;

  return result;
}

constexpr auto identifier_match(czstring_iterator str, czstring_iterator& it)
    -> std::optional<std::string_view> {
  const auto first = str;
  it = str;

  // First character must not be a digit and string should not be empty.
  if (!(is_letter(*it) || (*it == '.') || (*it == '_'))) return {};
  ++it;
  // Following characters can also include digits.
  while (!is_lexeme_end(*it)) {
    if (!(is_letter(*it) || (*it == '.') || (*it == '_') || is_digit(*it)))
      return {};
    ++it;
  }

  return {{first, it}};
}

constexpr auto separator_match(czstring_iterator it, czstring_iterator& end)
    -> std::optional<char> {
  if (!is_separator(*it)) return {};
  char result = *it++;
  end = it;
  return result;
}

}  // namespace lyrahgames::riscv
