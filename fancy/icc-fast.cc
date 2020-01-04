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
  auto arg = [&](int i) -> int64_t& {
    switch (Mode / (1 + 9 * (i > 0) + 90 * (i > 1)) % 10) {
      case 1: return     mem[pc - Arity + i];
      case 0: return mem[mem[pc - Arity + i]];
      case 2: return mem[mem[pc - Arity + i] + base];
    }
  };
  pc += Arity + 1;
  if constexpr (Arity == 0) (+*static_cast<F*>(0))();
  if constexpr (Arity == 1) (+*static_cast<F*>(0))(arg(0));
  if constexpr (Arity == 2) (+*static_cast<F*>(0))(arg(0), arg(1));
  if constexpr (Arity == 3) (+*static_cast<F*>(0))(arg(0), arg(1), arg(2));
  table[mem[pc]]();
};

int main(int argc, char** argv) {
  for (std::istringstream ss(argv[1]); ss >> mem[pc++]; ss.get()) {}
  table[mem[pc = 0]]();
}
