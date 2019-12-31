// Fast implementation of Intcode Computer in C++.
//
// Compile: g++ -std=c++17 -O3 -o icc-fast icc-fast.cc
// Usage: icc-fast "intcode"
//
// Input/output instructions use stdin/stdout.
//
// Example: icc-fast "3,0,102,2,0,0,4,0,99" <<<"21"
// Output:  42

#include <stdint.h>
#include <sys/mman.h>
#include <array>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <tuple>

static int64_t pc, base, *mem = static_cast<int64_t*>(mmap(
    0, 1LL << 46, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_NORESERVE, 0, 0));

template <class F, int Arity, int Mode> void run();

template <int N = -1, int Mode = -1, class Table, class F, class... Args>
constexpr void op(Table& table, int opcode, F f, void (*)(Args...) = 0) {
  if constexpr (N == -1) {
    op<0, 0>(table, opcode, f, +f);
  } else if constexpr (N == sizeof...(Args)) {
    table[100 * Mode + opcode] = &run<F, N, Mode>;
  } else {
    op<N + 1, Mode * 10 + 0>(table, opcode, f, +f);
    op<N + 1, Mode * 10 + 1>(table, opcode, f, +f);
    op<N + 1, Mode * 10 + 2>(table, opcode, f, +f);
  }
}

constexpr auto table = []() {
  std::array<void(*)(), 22209> t = {};
  op(t,  1, [](int64_t a, int64_t b, int64_t& r) { r = a + b             ; });  // add
  op(t,  2, [](int64_t a, int64_t b, int64_t& r) { r = a * b             ; });  // mul
  op(t,  7, [](int64_t a, int64_t b, int64_t& r) { r = a < b             ; });  // lt
  op(t,  8, [](int64_t a, int64_t b, int64_t& r) { r = a == b            ; });  // eq
  op(t,  5, [](int64_t a, int64_t b            ) { pc = a ? b : pc       ; });  // jnz
  op(t,  6, [](int64_t a, int64_t b            ) { pc = a ? pc : b       ; });  // jz
  op(t,  9, [](int64_t a                       ) { base += a             ; });  // rel
  op(t,  4, [](int64_t a                       ) { std::cout << a << '\n'; });  // out
  op(t,  3, [](                      int64_t& r) { std::cin >> r         ; });  // in
  op(t, 99, [](                                ) { std::exit(0)          ; });  // hlt
  return t;
}();

template <class F, int Arity, int Mode>
void run() {
  std::array<int64_t, Arity> args;
  auto set_arg = [&](int i, int m) {
    switch (Mode / m % 10) {
      case 1: args[i] =            pc + i + 1 ; break;
      case 0: args[i] =        mem[pc + i + 1]; break;
      case 2: args[i] = base + mem[pc + i + 1]; break;
    }
  };
  if (Arity > 0) set_arg(0, 1);
  if (Arity > 1) set_arg(1, 10);
  if (Arity > 2) set_arg(2, 100);
  pc += Arity + 1;
  std::apply([=](auto... pos) { (+*static_cast<F*>(0))(mem[pos]...); }, args);
  table[mem[pc]]();
};

int main(int argc, char** argv) {
  for (std::istringstream ss(argv[1]); ss >> mem[pc++]; ss.get()) {}
  table[mem[pc = 0]]();
}
