#include <iostream>

template <char... Cs> struct PosNumstr { static const char value[]; };
template <char... Cs>
constexpr char PosNumstr<Cs...>::value[] = {('0' + Cs)..., '\0'};

template <char... Cs> struct NegNumstr { static const char value[]; };
template <char... Cs>
constexpr char NegNumstr<Cs...>::value[] = {'-', ('0' + Cs)..., '\0'};

template <bool IsNeg, char... Cs> struct Numstr : PosNumstr<Cs...> {};

template <char... Cs> struct Numstr<true, Cs...> : NegNumstr<Cs...> {};

template <bool IsNeg, uintmax_t Rem, char... Cs>
struct NumstrBuilder : NumstrBuilder<IsNeg, Rem / 10, Rem % 10, Cs...> {};

template <bool IsNeg, char... Cs>
struct NumstrBuilder<IsNeg, 0, Cs...> : Numstr<IsNeg, Cs...> {};

template <typename T> constexpr uintmax_t int_abs(T n) {
  return (n < 0) ? -n : n;
}

template <int N> struct int_to_string : NumstrBuilder < N<0, int_abs(N)> {};

template <char... Cs> struct Literal {
  static constexpr char value[sizeof...(Cs) + 1] = {Cs..., '\0'};
};

template <typename CharT, CharT... Cs> constexpr const char *operator""_lit() {
  return Literal<Cs...>::value;
}

template <const char *...> struct Sstream { using stream = Sstream; };

template <typename> struct PopSstream {};
template <const char *C, const char *... Cs>
struct PopSstream<Sstream<C, Cs...>> : std::integral_constant<const char *, C>,
                                       Sstream<Cs...> {};

template <const char *, typename> struct PushSstream {};
template <const char *C, const char *... Cs>
struct PushSstream<C, Sstream<Cs...>> : Sstream<C, Cs...> {};

constexpr void fold(Sstream<>) {}

template <const char *C, const char *... Cs>
constexpr void fold(Sstream<C, Cs...>) {
  std::cout << C;
  fold(Sstream<Cs...>{});
}

template <const char *Cp, size_t Idx> constexpr auto format() {
  constexpr char c = Cp[Idx];

  if constexpr (c == '\0') {
    return Sstream<>{};
  } else {
    return typename PushSstream<Literal<c>::value,
                                decltype(format<Cp, Idx + 1>())>::stream{};
  }
}

template <const char *Cp, size_t Idx, auto T, auto... Ts>
constexpr auto format() {
  constexpr char fst = Cp[Idx];
  constexpr char snd = Cp[Idx + 1];

  if constexpr (std::is_integral_v<decltype(T)> && fst == '%' && snd == 'd') {
    return
        typename PushSstream<int_to_string<T>::value,
                             decltype(format<Cp, Idx + 2, Ts...>())>::stream{};
  } else if constexpr (std::is_same_v<decltype(T), const char *> &&
                       fst == '%' && snd == 's') {
    return typename PushSstream<T, decltype(
                                       format<Cp, Idx + 2, Ts...>())>::stream{};
  } else {
    return typename PushSstream<Literal<fst>::value,
                                decltype(
                                    format<Cp, Idx + 1, T, Ts...>())>::stream{};
  }
}

template <const char *Fmt, auto... Args> constexpr void Printf() {
  fold(format<Fmt, 0, Args...>());
}

int main() {
  Printf<"%s, %d!\n"_lit, "Hello"_lit, 42>();
  return 0;
}
