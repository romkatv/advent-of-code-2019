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

namespace {

using op_table_t = std::array<void(*)(), 22209>;

std::vector<int64_t> mem;
int64_t pc, base, mode;

template <class... Ts>
constexpr size_t arity(void(*)(Ts...)) {
  return sizeof...(Ts);
}

constexpr int digit(int num, int i) {
  while (i--) num /= 10;
  return num % 10;
}

template <class Op, size_t Arity, size_t Modes>
void run() {
  std::array<int64_t, Arity> args;
  auto set_arg = [&](auto i) {
    constexpr int mode = digit(Modes, i);
    switch (mode) {
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
  std::apply([=](auto... pos) { (+*static_cast<Op*>(0))(mem[pos]...); }, args);
}

template <class Op, size_t N = 0, size_t Modes = 0>
constexpr void add_op(op_table_t& table, size_t opcode, Op op) {
  if constexpr (N == arity(+op)) {
    table[100 * Modes + opcode] = &run<Op, N, Modes>;
  } else {
    add_op<Op, N + 1, Modes * 10 + 0>(table, opcode, op);
    add_op<Op, N + 1, Modes * 10 + 1>(table, opcode, op);
    add_op<Op, N + 1, Modes * 10 + 2>(table, opcode, op);
  }
}

constexpr auto make_op_table() {
  op_table_t t = {};
  add_op(t,  1, [](int64_t a, int64_t b, int64_t& r) { r = a + b             ; });  // add
  add_op(t,  2, [](int64_t a, int64_t b, int64_t& r) { r = a * b             ; });  // mul
  add_op(t,  7, [](int64_t a, int64_t b, int64_t& r) { r = a < b             ; });  // lt
  add_op(t,  8, [](int64_t a, int64_t b, int64_t& r) { r = a == b            ; });  // eq
  add_op(t,  5, [](int64_t a, int64_t b            ) { pc = a ? b : pc       ; });  // jnq
  add_op(t,  6, [](int64_t a, int64_t b            ) { pc = a ? pc : b       ; });  // jz
  add_op(t,  9, [](int64_t a                       ) { base += a             ; });  // rel
  add_op(t,  4, [](int64_t a                       ) { std::cout << a << '\n'; });  // out
  add_op(t,  3, [](                      int64_t& r) { std::cin >> r         ; });  // in
  add_op(t, 99, [](                                ) { std::exit(0)          ; });  // hlt
  return t;
}

constexpr op_table_t op_table = make_op_table();

}  // namespace

int main(int argc, char** argv) {
  for (std::istringstream ss(argv[1]); mem.push_back(0), ss >> mem.back(); ss.get()) {}
  while (true) op_table[mem[pc]]();
}
