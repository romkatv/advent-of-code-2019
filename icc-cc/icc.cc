#include <array>
#include <cstdint>
#include <deque>
#include <iostream>
#include <sstream>
#include <tuple>

using namespace std;

static deque<int64_t> mem;
static int64_t pc, base, mode;

static int64_t* at(int64_t pos) {
  if (pos >= mem.size()) mem.resize(mem.size() << 1);
  return &mem[pos];
}

static int64_t* next() {
  int m = mode % 10;
  mode /= 10;
  switch (m) {
    case 1: return at(pc++);
    case 0: return at(*at(pc++));
    case 2: return at(*at(pc++) + base);
  }
}

template <class... Ts>
static void exec(void (*op)(Ts*...)) {
  array<int64_t*, sizeof...(Ts)> args = {{(&exec<Ts>, next())...}};
  apply(op, args);
}

int main(int argc, char** argv) {
  istringstream strm(argv[1]);
  for (int64_t x; strm >> x; strm.get()) mem.push_back(x);
  for (int64_t op; (op = mem[pc++]) != 99;) {
    mode = op / 100;
    switch (op % 100) {
      case 1: exec(+[](int64_t* a, int64_t* b, int64_t* c) { *c = *a + *b;       }); break;  // add
      case 2: exec(+[](int64_t* a, int64_t* b, int64_t* c) { *c = *a * *b;       }); break;  // mul
      case 3: exec(+[](int64_t* a                        ) { cin >> *a;          }); break;  // in
      case 4: exec(+[](int64_t* a                        ) { cout << *a << '\n'; }); break;  // out
      case 5: exec(+[](int64_t* a, int64_t* b            ) { pc = *a ? *b : pc;  }); break;  // jnz
      case 6: exec(+[](int64_t* a, int64_t* b            ) { pc = *a ? pc : *b;  }); break;  // jz
      case 7: exec(+[](int64_t* a, int64_t* b, int64_t* c) { *c = *a < *b;       }); break;  // lt
      case 8: exec(+[](int64_t* a, int64_t* b, int64_t* c) { *c = *a == *b;      }); break;  // eq
      case 9: exec(+[](int64_t* a                        ) { base += *a;         }); break;  // rel
    }
  }
}
