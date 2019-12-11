// Fast implementation of Intcode Computer in C++.
//
// Compile: g++ -std=c++17 -O3 -o icc-fast icc-fast.cc
// Usage: icc-fast <intcode>
//
// Input/output instructions use stdin/stdout.
//
// Example: icc-fast 3,0,102,2,0,0,4,0,99 <<<21
// Output:  42

#include <stdint.h>
#include <stddef.h>
#include <array>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

using table_t = std::array<void(*)(), 22209>;

static std::vector<int64_t> mem;
static int64_t pc, base, mode;

template <class... Ts>
constexpr size_t arity(void(*)(Ts...)) {
  return sizeof...(Ts);
}

template <class F, size_t Arity, size_t M4>
void run() {
  std::array<int64_t, Arity> args;
  auto set_arg = [&](auto i) {
    switch ((M4 >> 2 * i) & 3) {
      case 1: args[i] =            pc + i + 1 ; return;
      case 0: args[i] =        mem[pc + i + 1]; break;
      case 2: args[i] = base + mem[pc + i + 1]; break;
    }
    if (mem.size() <= args[i]) mem.resize(args[i] << 1);
  };
  if constexpr (Arity > 0) set_arg(std::integral_constant<int, 0>());
  if constexpr (Arity > 1) set_arg(std::integral_constant<int, 1>());
  if constexpr (Arity > 2) set_arg(std::integral_constant<int, 2>());
  pc += Arity + 1;
  std::apply([=](auto... pos) { (+*static_cast<F*>(0))(mem[pos]...); }, args);
}

template <class F, size_t N = 0, size_t M10 = 0, size_t M4 = 0>
constexpr void op(table_t& table, size_t opcode, F f) {
  if constexpr (N == arity(+f)) {
    table[100 * M10 + opcode] = &run<F, N, M4>;
  } else {
    op<F, N + 1, M10 * 10 + 0, M4 << 2 | 0>(table, opcode, f);
    op<F, N + 1, M10 * 10 + 1, M4 << 2 | 1>(table, opcode, f);
    op<F, N + 1, M10 * 10 + 2, M4 << 2 | 2>(table, opcode, f);
  }
}

constexpr auto make_table() {
  table_t t = {};
  op(t,  1, [](int64_t a, int64_t b, int64_t& r) { r = a + b             ; });  // add
  op(t,  2, [](int64_t a, int64_t b, int64_t& r) { r = a * b             ; });  // mul
  op(t,  7, [](int64_t a, int64_t b, int64_t& r) { r = a < b             ; });  // lt
  op(t,  8, [](int64_t a, int64_t b, int64_t& r) { r = a == b            ; });  // eq
  op(t,  5, [](int64_t a, int64_t b            ) { pc = a ? b : pc       ; });  // jnq
  op(t,  6, [](int64_t a, int64_t b            ) { pc = a ? pc : b       ; });  // jz
  op(t,  9, [](int64_t a                       ) { base += a             ; });  // rel
  op(t,  4, [](int64_t a                       ) { std::cout << a << '\n'; });  // out
  op(t,  3, [](                      int64_t& r) { std::cin >> r         ; });  // in
  op(t, 99, [](                                ) { std::exit(0)          ; });  // hlt
  return t;
}

constexpr table_t table = make_table();

int main(int argc, char** argv) {
  for (std::istringstream ss(argv[1]); mem.push_back(0), ss >> mem.back(); ss.get()) {}
  while (true) table[mem[pc]]();
}
