#pragma once
#include <cstdint>
//
#include <bitset>
#include <iomanip>
#include <iostream>

namespace lyrahgames::riscv {

struct int_register {
  friend constexpr auto operator<=>(const int_register&,
                                    const int_register&) noexcept = default;
  uint8_t code{};
};

constexpr int_register x0{0};
constexpr int_register x1{1};
constexpr int_register x2{2};
constexpr int_register x3{3};
constexpr int_register x4{4};
constexpr int_register x5{5};
constexpr int_register x6{6};
constexpr int_register x7{7};
constexpr int_register x8{8};
constexpr int_register x9{9};
constexpr int_register x10{10};
constexpr int_register x11{11};
constexpr int_register x12{12};
constexpr int_register x13{13};
constexpr int_register x14{14};
constexpr int_register x15{15};
constexpr int_register x16{16};
constexpr int_register x17{17};
constexpr int_register x18{18};
constexpr int_register x19{19};
constexpr int_register x20{20};
constexpr int_register x21{21};
constexpr int_register x22{22};
constexpr int_register x23{23};
constexpr int_register x24{24};
constexpr int_register x25{25};
constexpr int_register x26{26};
constexpr int_register x27{27};
constexpr int_register x28{28};
constexpr int_register x29{29};
constexpr int_register x30{30};
constexpr int_register x31{31};

constexpr auto zero = x0;

constexpr auto ra = x1;
constexpr auto sp = x2;
constexpr auto gp = x3;
constexpr auto tp = x4;

constexpr auto t0 = x5;
constexpr auto t1 = x6;
constexpr auto t2 = x7;

constexpr auto fp = x8;

constexpr auto s0 = x8;
constexpr auto s1 = x9;

constexpr auto a0 = x10;
constexpr auto a1 = x11;
constexpr auto a2 = x12;
constexpr auto a3 = x13;
constexpr auto a4 = x14;
constexpr auto a5 = x15;
constexpr auto a6 = x16;
constexpr auto a7 = x17;

constexpr auto s2 = x18;
constexpr auto s3 = x19;
constexpr auto s4 = x20;
constexpr auto s5 = x21;
constexpr auto s6 = x22;
constexpr auto s7 = x23;
constexpr auto s8 = x24;
constexpr auto s9 = x25;
constexpr auto s10 = x26;
constexpr auto s11 = x27;

constexpr auto t3 = x28;
constexpr auto t4 = x29;
constexpr auto t5 = x30;
constexpr auto t6 = x31;

inline std::ostream& operator<<(std::ostream& os, int_register r) {
  return os << 'x' << int(r.code) << "[" << std::bitset<5>(r.code) << "]";
}

}  // namespace lyrahgames::riscv
