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
#include <cstdlib>
#include <iostream>
#include <sstream>

constexpr int64_t* z = nullptr;
register void(**ops)() asm("r13");
register int64_t *pc asm("r14"), *base asm("r15");

template <int M = -1, int... N, class F, class... T>
void op(int opcode, F f, void (*)(T...) = 0) {
  if constexpr (M == -1) {
    op<0>(opcode, f, +f);
  } else if constexpr (sizeof...(N) == sizeof...(T)) {
    ops[100 * M + opcode] = +[] {
      pc += sizeof...(N) + 1;
      (+*(F*)(0))(N & 8 ? pc[-N % 4] : (N & 4 ? base : +z)[pc[-N % 4]]...);
      ops[*pc]();
    };
  } else {
    op<M * 10 + 0, sizeof...(N) + 1, N...>(opcode, f, +f);
    op<M * 10 + 1, sizeof...(N) + 9, N...>(opcode, f, +f);
    op<M * 10 + 2, sizeof...(N) + 5, N...>(opcode, f, +f);
  }
}

int main(int argc, char** argv) {
  ops = [] { static void(*ops[22209])(); return ops; }();
  op( 1, [](int64_t a, int64_t b, int64_t& r) { r = a + b             ; });  // add
  op( 2, [](int64_t a, int64_t b, int64_t& r) { r = a * b             ; });  // mul
  op( 7, [](int64_t a, int64_t b, int64_t& r) { r = a < b             ; });  // lt
  op( 8, [](int64_t a, int64_t b, int64_t& r) { r = a == b            ; });  // eq
  op( 5, [](int64_t a, int64_t b            ) { if (a) pc = z + b     ; });  // jnz
  op( 6, [](int64_t a, int64_t b            ) { if (!a) pc = z + b    ; });  // jz
  op( 9, [](int64_t a                       ) { base += a             ; });  // rel
  op( 4, [](int64_t a                       ) { std::cout << a << '\n'; });  // out
  op( 3, [](                      int64_t& r) { std::cin >> r         ; });  // in
  op(99, [](                                ) { std::exit(0)          ; });  // hlt
  pc = base = (int64_t*)mmap(0, 1LL << 46, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANON | MAP_NORESERVE | MAP_FIXED, 0, 0);
  for (std::istringstream ss(argv[1]); ss >> *pc++; ss.get()) {}
  ops[*(pc = base)]();
}
