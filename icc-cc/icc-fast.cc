// Compile: g++ -std=c++17 -O3 -o icc-fast icc-fast.cc
// Usage: icc-fast <intcode>
//
// Input/output instructions use stdin/stdout.
//
// Example: icc-fast 3,0,102,2,0,0,4,0,99 <<<21
// Output:  42

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

namespace {

using std::int64_t;
using std::size_t;

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

template <auto Op, size_t Modes>
void run() {
  std::array<int64_t, arity(Op)> args;
  auto set_arg = [&](auto i) {
    constexpr int mode = digit(Modes, i);
    switch (mode) {
      case 1: args[i] =            pc + i + 1 ; return;
      case 0: args[i] =        mem[pc + i + 1]; break;
      case 2: args[i] = base + mem[pc + i + 1]; break;
    }
    if (mem.size() <= args[i]) mem.resize(args[i] << 1);
  };
  if constexpr (arity(Op) > 0) set_arg(std::integral_constant<int, 0>());
  if constexpr (arity(Op) > 1) set_arg(std::integral_constant<int, 1>());
  if constexpr (arity(Op) > 2) set_arg(std::integral_constant<int, 2>());
  pc += arity(Op) + 1;
  std::apply([=](auto... pos) { Op(mem[pos]...); }, args);
}

template <auto Op, size_t N = 0, size_t Modes = 0>
constexpr void add_op(size_t opcode, op_table_t& table) {
  if constexpr (N == arity(Op)) {
    table[100 * Modes + opcode] = &run<Op, Modes>;
  } else {
    add_op<Op, N + 1, Modes * 10 + 0>(opcode, table);
    add_op<Op, N + 1, Modes * 10 + 1>(opcode, table);
    add_op<Op, N + 1, Modes * 10 + 2>(opcode, table);
  }
}

void add(int64_t a, int64_t b, int64_t& r) { r = a + b             ; }
void mul(int64_t a, int64_t b, int64_t& r) { r = a * b             ; }
void lt (int64_t a, int64_t b, int64_t& r) { r = a < b             ; }
void eq (int64_t a, int64_t b, int64_t& r) { r = a == b            ; }
void jnz(int64_t a, int64_t b            ) { pc = a ? b : pc       ; }
void jz (int64_t a, int64_t b            ) { pc = a ? pc : b       ; }
void rel(int64_t a                       ) { base += a             ; }
void out(int64_t a                       ) { std::cout << a << '\n'; }
void in (                      int64_t& r) { std::cin >> r         ; }
void hlt(                                ) { std::exit(0)          ; }

constexpr auto make_op_table() {
  op_table_t t = {};
  add_op<add>(1, t);
  add_op<mul>(2, t);
  add_op<in> (3, t);
  add_op<out>(4, t);
  add_op<jnz>(5, t);
  add_op<jz> (6, t);
  add_op<lt> (7, t);
  add_op<eq> (8, t);
  add_op<rel>(9, t);
  add_op<hlt>(99, t);
  return t;
}

constexpr op_table_t op_table = make_op_table();

}  // namespace

int main(int argc, char** argv) {
  for (std::istringstream ss(argv[1]); mem.push_back(0), ss >> mem.back(); ss.get()) {}
  while (true) op_table[mem[pc]]();
}
