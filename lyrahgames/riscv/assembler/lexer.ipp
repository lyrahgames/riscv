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
