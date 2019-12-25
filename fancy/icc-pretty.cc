// Simple implementation of Intcode Computer in C++.
//
// Compile: g++ -std=c++17 -O3 -o icc-pretty icc-pretty.cc
// Usage: icc-pretty "intcode"
//
// Input/output instructions use stdin/stdout.
//
// Example: icc-pretty "3,0,102,2,0,0,4,0,99" <<<"21"
// Output:  42

#include <stdint.h>
#include <array>
#include <vector>
#include <iostream>
#include <sstream>
#include <tuple>

static std::vector<int64_t> mem;
static int64_t pc, base, mode;

template <class... Ts>
static void run(void (*op)(Ts...)) {
  std::array<int64_t, sizeof...(Ts)> args;
  for (int64_t i = 0; i != args.size(); mode /= 10, ++pc, ++i) {
    switch (mode % 10) {
      case 1: args[i] =            pc ; break;
      case 0: args[i] =        mem[pc]; break;
      case 2: args[i] = base + mem[pc]; break;
    }
    if (mem.size() <= args[i]) mem.resize(args[i] << 1);
  }
  std::apply([=](auto... pos) { op(mem[pos]...); }, args);
}

int main(int argc, char** argv) {
  for (std::istringstream ss(argv[1]); mem.push_back(0), ss >> mem.back(); ss.get()) {}
  while (true) {
    mode = mem[pc] / 100;
    switch (mem[pc++] % 100) {
      case 1: run(+[](int64_t a, int64_t b, int64_t& r) { r = a + b             ; }); break;  // add
      case 2: run(+[](int64_t a, int64_t b, int64_t& r) { r = a * b             ; }); break;  // mul
      case 7: run(+[](int64_t a, int64_t b, int64_t& r) { r = a < b             ; }); break;  // lt
      case 8: run(+[](int64_t a, int64_t b, int64_t& r) { r = a == b            ; }); break;  // eq
      case 5: run(+[](int64_t a, int64_t b            ) { pc = a ? b : pc       ; }); break;  // jnz
      case 6: run(+[](int64_t a, int64_t b            ) { pc = a ? pc : b       ; }); break;  // jz
      case 9: run(+[](int64_t a                       ) { base += a             ; }); break;  // rel
      case 4: run(+[](int64_t a                       ) { std::cout << a << '\n'; }); break;  // out
      case 3: run(+[](                      int64_t& r) { std::cin >> r         ; }); break;  // in
      case 99: return 0;                                                              break;  // hlt
    }
  }
}
