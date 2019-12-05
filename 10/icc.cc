#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#define CHECK(expr...)                                                                       \
  if (!(expr)) {                                                                             \
    ::std::cerr << __FILE__ << ':' << __LINE__ << ": CHECK(" << #expr << ")" << ::std::endl; \
    ::std::abort();                                                                          \
  } else                                                                                     \
    (void)0

namespace {

using ::std::int32_t;
using ::std::int64_t;
using ::std::size_t;

std::string ReadProgram(int argc, char** argv) {
  std::string res;
  if (argc == 1) {
    CHECK(std::getline(std::cin, res));
  } else if (!std::strcmp(argv[1], "-c")) {
    CHECK(argc == 3);
    res = argv[2];
  } else {
    if (*argv[1] == '-') {
      CHECK(!std::strcmp(argv[1], "--"));
      --argc;
      ++argv;
    }
    CHECK(argc == 2);
    std::ifstream strm(argv[1]);
    CHECK(std::getline(strm, res));
  }
  return res;
}

int32_t ParseInt32(std::string_view s) {
  int32_t res = 0;
  CHECK(std::from_chars(s.data(), s.data() + s.size(), res, 10).ec == std::errc());
  return res;
}

std::vector<int32_t> ParseProgram(std::string_view src) {
  std::vector<int32_t> res;
  size_t start = 0;
  while (true) {
    size_t end = src.find(',', start);
    if (end == std::string_view::npos) {
      res.push_back(ParseInt32(src.substr(start)));
      return res;
    }
    res.push_back(ParseInt32(src.substr(start, end - start)));
    start = end + 1;
  }
  return res;
}

struct State {
  std::vector<int32_t> mem;
  size_t p = 0;
};

class Arg {
 public:
  template <class R>
  static Arg Make(std::vector<int32_t>& mem, size_t& p, int32_t& mode) {
    Arg res;
    CHECK(p < mem.size());
    static_assert(std::is_same<R, int32_t>() || std::is_same<R, int32_t*>());
    switch (mode % 10) {
      case 0:
        CHECK(mem[p] >= 0 && static_cast<size_t>(mem[p]) < mem.size());
        res.p_ = &mem[mem[p]];
        break;
      case 1:
        res.p_ = &mem[p];
        CHECK(std::is_same<R, int32_t>());
        break;
      default:
        CHECK(false);
    }
    mode /= 10;
    ++p;
    return res;
  }

  operator int32_t() const { return *p_; }
  operator int32_t*() const { return p_; }

 private:
  int32_t* p_;
};

template <class... Args>
void EvalInstruction(State& state, void (*f)(State&, Args...)) {
  auto& mem = state.mem;
  auto& p = state.p;
  CHECK(p + sizeof...(Args) < mem.size());
  CHECK(mem[p] >= 0);
  int32_t mode = mem[p] / 100;
  ++p;
  std::array<Arg, sizeof...(Args)> args = {{Arg::Make<Args>(mem, p, mode)...}};
  CHECK(mode == 0);
  std::apply(f, std::tuple_cat(std::tie(state), args));
}

int32_t Narrow(int64_t n) {
  CHECK(n >= std::numeric_limits<int32_t>::min());
  CHECK(n <= std::numeric_limits<int32_t>::max());
  return n;
}

void JumpIf(State& state, bool cond, int32_t pos) {
  if (!cond) return;
  CHECK(pos >= 0 && static_cast<size_t>(pos) < state.mem.size());
  state.p = pos;
}

void Op_Add(State&, int32_t a, int32_t b, int32_t* out) { *out = Narrow(int64_t{a} + int64_t{b}); }
void Op_Mul(State&, int32_t a, int32_t b, int32_t* out) { *out = Narrow(int64_t{a} * int64_t{b}); }
void Op_Input(State&, int32_t* out) { CHECK(std::cin >> *out); }
void Op_Output(State&, int32_t a) { CHECK(std::cout << a << '\n'); }
void Op_JumpNZ(State& state, int32_t a, int32_t pos) { JumpIf(state, a != 0, pos); }
void Op_JumpZ(State& state, int32_t a, int32_t pos) { JumpIf(state, a == 0, pos); }
void Op_LT(State&, int32_t a, int32_t b, int32_t* out) { *out = a < b; }
void Op_EQ(State&, int32_t a, int32_t b, int32_t* out) { *out = a == b; }

void Eval(State& state) {
  auto& mem = state.mem;
  auto& p = state.p;
  while (p != mem.size()) {
    CHECK(p < mem.size());
    CHECK(mem[p] >= 0);
    switch (mem[p] % 100) {
      case 99: return;
      case 1: EvalInstruction(state, Op_Add); break;
      case 2: EvalInstruction(state, Op_Mul); break;
      case 3: EvalInstruction(state, Op_Input); break;
      case 4: EvalInstruction(state, Op_Output); break;
      case 5: EvalInstruction(state, Op_JumpNZ); break;
      case 6: EvalInstruction(state, Op_JumpZ); break;
      case 7: EvalInstruction(state, Op_LT); break;
      case 8: EvalInstruction(state, Op_EQ); break;
      default: CHECK(false);
    }
  }
}

}  // namespace

int main(int argc, char** argv) {
  std::string src = ReadProgram(argc, argv);
  State state = {ParseProgram(src)};
  Eval(state);
}
