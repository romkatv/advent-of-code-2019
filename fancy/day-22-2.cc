#include <stdint.h>
#include <iostream>
#include <string>

const int64_t m = 119315717514047, n = 101741582076661, needle = 2020;

const auto combine = [](auto f, int64_t unit, int64_t a, int64_t b) {
  for (int64_t r = unit;; b >>= 1, a = f(a, a)) {
    if (!b) return r;
    if (b & 1) r = f(r, a);
  }
};

static int64_t add(int64_t a, int64_t b) { return (m + (a + b) % m) % m; }
static int64_t mul(int64_t a, int64_t b) { return combine(add, 0, a, b); }
static int64_t pow(int64_t a, int64_t b) { return combine(mul, 1, a, b); }

int main() {
  int64_t k = 1, b = 0, x;
  for (std::string s; std::getline(std::cin, s);) {
    if (s.find("inc") + 1) { k = mul(k, x = std::stoll(s.substr(20))); b = mul(b, x)  ; }
    if (s.find("cut") + 1) { b = add(b, -std::stoll(s.substr(4)))    ; b = b          ; }
    if (s.find("new") + 1) { k = add(0, -k)                          ; b = add(-1, -b); }
  }
  x=mul(b, pow(k-1, m-2));
  std::cout << add(mul(add(x, needle), pow(pow(k, m-2), n)), -x) << std::endl;
}
