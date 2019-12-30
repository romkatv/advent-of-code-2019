# Advent of Code 2019 in Zsh

[Advent of Code](https://adventofcode.com/) is an Advent calendar of small programming puzzles. This
is the story of my participation, for the first time, in 2019.

I learned about the existence of Advent of Code through a post on Hacker News on the 1st of
December -- the day the first Advent of Code puzzle of 2019 was published. I skimmed through the
[about page](https://adventofcode.com/2019/about) and on a whim decided to see how far I could get
solving puzzles in [Zsh](https://en.wikipedia.org/wiki/Z_shell).

I've been using Zsh as my primary shell for almost a year. I would classify myself between beginner
and intermediate user. Most of my Zsh experience has been earned while developing and maintaining
[powerlevel10k](https://github.com/romkatv/powerlevel10k) -- a relatively popular open source
project written in and for Zsh. Like many, I chose Zsh as my Advent of Code language in order to
learn it better. This wish has been fulfiled but not in the way I expected.

The first few days of Advent of Code featured very simple puzzles. Direct translation of the problem
statement to Zsh was all that was required from me. There was a lot of headroom for inefficient
implementations, which allowed me to use concise solutions that may not be available in many
"proper" programming languages. For example, the day 4 puzzle asked to count numbers between `min`
and `max` whose decimal digits are non-decreasing and that contain at least one digit that follows
itself. This is just 4 lines in Zsh:

```zsh
for ((n = min; n <= max; ++n)); do
  [[ $n == ${(j::)${(os::)n}} && $n =~ '(.)\1' ]] && (( ++res ))
done
echo $res
```

`${(os::)n}` splits number `n` into array of decimal digits and sorts them. `${(j::)array}` joins
the elements of the resulting array. If the result isn't the same as `n`, we don't need to count
this number as it doesn't satisfy the monotonicity requirement. `$n =~ '(.)\1'` checks if the number
contains two identical consecutive digits.

Solutions that use Zsh-specific "vectorized" constructs not found in other languages are often more
efficient than traditional algorithms translated one-to-one to Zsh. Cutting down on the number of
operations that have to be interpreted is very important for performance.

The highlight of Advent of Code 2019 was *Intcode* -- machine code for the fictional *Intcode
Computer*. Participants were to build over 3 days a Virtual Machine that could execute programs
written in Intcode. Here's my final version:

```zsh
#!/usr/env/bin zsh

local mem=(${(s:,:)1:?usage: icc.zsh <intcode> [prompt]})
local -i pc base mode n

function argpos() {
  local -i m='mode % 10'
  (( mode /= 10 ))
  case $m in
    1) return '++pc'                ;;
    0) return 'mem[++pc] + 1'       ;;
    2) return 'mem[++pc] + 1 + base';;
  esac
}

function fetch() { (( mem[argpos()]          )) }
function store() { (( mem[argpos()]=$1       )) }
function jumpc() { (( pc += ($1) * ($2 - pc) )) }

functions -M argpos 0
functions -M fetch  0

while true; do
  mode='mem[++pc] / 100'
  case $((mem[pc] % 100)); in
    99) exit 0                        ;;  # hlt
     9) base+='fetch()'               ;;  # rel
     1) store 'fetch() +  fetch()'    ;;  # add
     2) store 'fetch() *  fetch()'    ;;  # mul
     7) store 'fetch() <  fetch()'    ;;  # lt
     8) store 'fetch() == fetch()'    ;;  # eq
     5) jumpc 'fetch() != 0' 'fetch()';;  # jnz
     6) jumpc 'fetch() == 0' 'fetch()';;  # jz
     3) echo -nE - $2; read n; store n;;  # in
     4) echo - $((fetch()))           ;;  # out
  esac
done
```

I saved this code in an executable file called `icc` -- for **I**nt**C**ode **C**omputer -- and
called it whenever puzzles required execution of Intcode programs. Here's an example:

```shell
$ icc "3,0,2,0,0,0,4,0,99" "Number please: "
Number please: 12
144
```

The first argument is an Intcode program. All Intcode programs are comma-separated integers. Intcode
Computer communicates with the outside world by reading and printing integers. In this example the
program reads an integer and prints its square. The second (optional) argument of `icc` is a prompt
that gets printed whenever `icc` wants to read a number. It lets you know when `icc` is waiting for
input as opposed to being busy computing.

This implementation doesn't take advantage of anything Zsh-specific, so it can be directly
translated into any mainstream language. With 37 lines of code written over 3 days, coding output
demanded by Advent of Code is fairly low. It'll get higher as December progresses, peaking on day
18 for me. But I'm getting ahead of myself.

The implementation of Intcode Computer wasn't a standalone exercise. Every other puzzle required
participants to *run* Intcode programs on their own Intcode Virtual Machine. These were my favorite
puzzles! For example, on day 7 we were given an Intcode Program (`3,8,1001,8,10,..` and so on, for
about 1kB) that implements an amplifier. In fact, it implements 5 different amplifiers! On start,
the program asks which amplifier to use, which has to be a number between 5 and 9. After that it
continuously reads strength of an input signal and prints the strength of the (amplified) output
signal, and eventually terminates. The objective was to find the maximum output among all circuits
of this kind:

```text
      O-------O  O-------O  O-------O  O-------O  O-------O
0 -+->| Amp A |->| Amp B |->| Amp C |->| Amp D |->| Amp E |-.
   |  O-------O  O-------O  O-------O  O-------O  O-------O |
   |                                                        |
   '--------------------------------------------------------+
                                                            |
                                                            v
                                                     (to thrusters)
```

*(Diagram from https://adventofcode.com/2019/day/7, part 2.)*

Here 5 different amplifiers are connected back-to-back, with the output of the last amplifier being
fed back to the first. Letters `A` through `E` are placeholders for numbers `5` through `9`, each
number used exactly once. The amplification process starts by feeding `0` to the first amplifier and
ends when all amplifiers terminate. The last output from the last amplifier is the circuit output.

Connecting the output of one computation to the input of another sounds exactly like Unix pipelines!
Unsurprisingly, that's what I used to solve this puzzle.

```zsh
function amp() { print -l $*; icc "3,8,1001,8,10,.." }

for n in {56789..98765}; do
  [[ ${(j::)${(os::)n}} == 56789 ]] || continue
  coproc amp $n[2] | amp $n[3] | amp $n[4] | amp $n[5] | amp $n[1] 0
  <&p >&p >&0 | tail -1
done | sort | tail -1
```

`amp [arg]...` starts an amplifier and sends it predefined integers as first inputs. The loop goes
over all permutations of `56789` and runs the circuit for each of them. After the loop, the maximum
signal value is acquired with `sort | tail -1` pipeline.

In C++ I would use `std::next_permutation` to go over all permutations. Rather than implementing
this algorithm in Zsh, I'm iterating over all numbers between 56789 and 98765 and skipping the wrong
ones. `${(j::)${(os::)n}}` is the familiar construct we've met in the solution to the day 4 puzzle.
Useful once more!

So far, all problems were recipes. You obtain the solution by following the instructions step by
step. Eventually problems turned into proper puzzles whose statements describe the solution only
indirectly, leaving it up to you to figure out how to obtain it.

As far as programming languages go, Zsh is slow. How slow exactly depends on the workload but for
the kind of computation required by Advent of Code puzzles the answer is *very slow*. A
[direct translation](https://github.com/romkatv/advent-of-code-2019/blob/master/fancy/icc-pretty.cc)
of my Zsh Intcode Computer to C++ is between 3000 and 25000 faster, depending on the program. An
[optimized C++ implementation](
https://github.com/romkatv/advent-of-code-2019/blob/master/fancy/icc-fast.cc) is 4 times faster
still. \[This is likely [the fastest Intcode Computer](
https://www.reddit.com/r/adventofcode/comments/egq9xn/2019_day_9_intcode_benchmarking_suite/fcar0oh)
in the world. I had a lot of fun writing it. Take a look if you are into C++ or high-performance
code in general. It's only 65 lines long and very readable. I've posted a short explanation of how
it works in [this comment](
https://www.reddit.com/r/adventofcode/comments/ed5bxv/intcode_primality_generator/fbghid2/) on
reddit.\]

I was committed to finding a solution to every puzzle that would run in Zsh in less than a minute
(the same algorithm would run in a few milliseconds in C++). However, *getting* to the solution
while using nothing but Zsh felt like too much. I settled on the following routine. Once I wake up
in the morning I would get to my computer, read the new Advent of Code puzzle and solve it as
quickly as possible in C++. When using a reasonably fast programming language, the first part of
each puzzle is trivial. Just literally do what the problem statement says and you'll get the answer.
My goal here was not to find a solution that I would eventually be porting to Zsh but simply to
unlock the second part of the puzzle. Every day's puzzle has two parts in it and you cannot see the
harder second part until you solve the first.

Let's consider the day 22 puzzle. In part 1 we were given a sorted deck of 10007 cards numbered from
0 to 10006. The personal input to the puzzle is a file with 100 lines of the following sort:

```text
cut 4913
deal into new stack
```

Each line describes a permutation of the deck. Here `cut 4913` means taking 4913 cards from the top
and moving them to the bottom. `deal into new stack` means reversing the order of cards. The goal
was to find the position of card `2019` in the deck after all these shuffling steps have been
applied.

Since this is part 1, I would literally do what I'm asked. It's pointless to try to optimize because
I've no idea what part 2 will be about. Here's what I would write for this puzzle:

```c++
int main() {
  const int m = 10007;
  array<int, m> x, y;
  iota(begin(x), end(x), 0);
  for (string s; getline(cin, s); x = y) {
    int a = atoi(s.substr(s.find_last_of(' ') + 1).c_str());
    if (s.find("new") + 1) {
      reverse_copy(begin(x), end(x), begin(y));
    } else if (s.find("cut") + 1) {
      rotate_copy(begin(x), begin(x) + (a + m) % m, end(x), begin(y));
    } else if (s.find("inc") + 1) {
      for (size_t i = 0; i != m; ++i) y[(i * a) % m] = x[i];
    }
  }
  cout << (find(begin(x), end(x), 2019) - begin(x)) << '\n';
}
```

This C++ program reads the input file with shuffling steps line by line and shuffles the deck as
described by the spec. No thinking required, just typing away. By using C++ I was able to unlock
the second part of the puzzle in just a few minutes thanks to C++ being fast, my experience in using
this language, and my development setup (debugger is invaluable!).

The second part of this puzzle asks to find the card that ends up at position 2020 in the deck of
119315717514047 cards after 101741582076661 rounds of applying all shuffling rules to it (below, `m`
and `n` respectively). The second part of many Advent of Code puzzles makes the problem harder in a
way that renders the straightfoward algorithm used to solve part 1 inapplicable. Day 22 was no
exception. C++ or not, you cannot create an array with 100 trillion elements, or to loop for 100
trillion iterations.

After reading the second part of the problem I would go take a shower. While showring I would think.
On all days except one I knew how to solve the problem in C++ by the time I finished showering.
Where Advent of Code requires the use of algorithms or math, they are of the basic kind. Neither
difficult to figure out nor hard to code.

Back to our terradeck. Every shuffling step can be expressed as a linear function: card `c` goes to
position `k*c + b` for some `k` and `b`. Composition of linear functions is itself a linear
function, which means that applying all 100 shuffling steps from the input file will move each card
according to some linear function. Composing this function with itself `n` times using the simple,
elegant and efficient [exponentiation by squaring](
https://en.wikipedia.org/wiki/Exponentiation_by_squaring) algorithm will once again give us a
linear function. This isn't the function we need though. In part 2 we are asked for the *inverse*
transformation. How do you invert `k*c + b` where all numbers are integers modulo `m`?

Up until this point things were pretty straightforward. You have to use an algorithm but it's both
well-known and easy to implement. The next step requires a bit of math and I've heard from others
that not having learned algebra or having forgotten it makes this puzzle very difficult.

Before showering, I'd checked if the two huge numbers in the problem statement are prime. They both
are. I remembered from high school that integers modulo prime number make a *field*, meaning
that each element except zero has multiplicative inverse. Since the field is finite, `a ** m = a`
for all `a`. This implies `1 / a = a ** (m-2)` for all non-zero `a`. This insight allows us to
inverse linear functions modulo `m` the same way as if we were dealing with real numbers. The code
is rather short albeit not exciting.

```c++
const int64_t m = 119315717514047, n = 101741582076661;

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
  int64_t k = 1, b = 0;
  for (string s; getline(cin, s);) {
    int a = atoi(s.substr(s.find_last_of(' ') + 1).c_str());
    if (s.find("inc") + 1) { b = mul(b,   a); k = mul(k,  a); }
    if (s.find("cut") + 1) { b = add(b,  -a); k = k         ; }
    if (s.find("new") + 1) { b = add(-1, -b); k = add(0, -k); }
  }
  int64_t x=mul(b, pow(k-1, m-2));
  cout << add(mul(add(x, 2020), pow(pow(k, m-2), n)), -x) << endl;
}
```

With a C++ solution working, I would check how long it takes to solve the puzzle. If it's over 50
milliseconds, it would be pointless to attempt porting the code to Zsh as its run time certainly
wouldn't fit within within the self-imposed limit of one minute per puzzle. I would have to keep
iterating in C++ until I find a more efficient algorithm.

In this case the C++ implementation yields the answer in 0.3 milliseconds. It also doesn't use
anything that tends to cause disproportionately high slowdown when ported to Zsh. The port to Zsh is
straightforward and it is fast enough, as expected.

```zsh
integer m=119315717514047 n=101741582076661

function add() { return "(($1 + $2) % m + m) % m" }
functions -M add 2

for f g in mul add pow mul; do
  functions[$f]='
    integer a=$1 b="$2-1" res=$1
    for ((;; b >>= 1, a='$g'(a, a))); do
      (( b )) || return res
      (( b & 1 == 0 )) || res="'$g'(res, a)"
    done'
  functions -M $f 2
done

integer k=1 b=0
while read -r s; do
  case $s in
    *inc*) (( b = mul(b, ${s##* })     , k = mul(k, ${s##* }) ));;
    *new*) (( b = add(-1, -b)          , k = add(0, -k)       ));;
    *cut*) (( b = add(b, -1 * ${s##* }), k = k                ));;
  esac
done

integer x='mul(b, pow(k-1, m-2))'
echo $((add(mul(add(x, 2020), pow(pow(k, m-2), n)), -x)))
```

Having a Zsh solution to part 2, it was usually trivial to devise solution to part 1 from it.

```zsh
integer m=10007 x=2019
while read -r s; do
  case $s in
    *inc*) (( x = x * ${s##* }       % m ));;
    *new*) (( x = (m + m - x - 1)    % m ));;
    *cut*) (( x = (m + x - ${s##* }) % m ));;
  esac
done
echo $x
```

Note that by day 22 solutions became longer. Part 2 required 26 lines of code to solve. There were
a few puzzles that required even more coding, with the longest solution for me falling on day 18. On
this day we were to navigate 4 disconnected mazes simultaneously with 4 robots. Spread across the
mazes are 26 doors (marked with capital letters from *A* to *Z*) with corresponding keys (*a* to
*z*). Initially all doors are locked and cannot be passed through. Once one of the robots grabs a
key, the corresponding door gets unlocked. The task is to figure out the shortest number of steps
robots can take to grab all keys. The map of the 4 mazes, including the positions of all doors and
keys, is the input file.

This is one of a few problems that was easy in C++ but resisted my attempts to make it run fast
enough in Zsh. My first port to Zsh was at least a million times slower than C++. Yikes! Normally I
would go back to C++ and find a more efficient algorithm. This time it wasn't an option as I was
certain that the asymptotic complexity was already optimal. All optimizations had to come from more
efficient coding. Not an easy task for an initiate in Zsh scripting like myself.

Here's my final gnarly solution at whooping 75 lines. Took over two hours. I don't recommend reading
it. I'm posting it to show the upper bound on the size and complexity of my solutions.

```zsh
local -i x w=81 h=81 c='40*81+41'
local maze="${$(<&0)//$'\n'}"
local -A adjacent rcache
local -a doors=(0 0 0 0)

maze[c-w-1,c-w+1]="@#@"
maze[c+0-1,c+0+1]="###"
maze[c+w-1,c+w+1]="@#@"

function mask() {
  local c=$maze[$1] lb=${2:-97}
  return '#c >= lb && #c < lb + 26 ? 1 << (#c - lb) : 0'
}
functions -M mask 1 2

function bfs() {
  local -A queued=("$*[2,-1]" 0)
  local q=("0 $*[2,-1]")
  local -i i lim
  function visit() {
    local -i dist="node[1] + $1"
    (( dist < ${queued[${*[2,-1]}]-dist+1} )) || return
    queued[${*[2,-1]}]=$dist
    q+="$dist ${*[2,-1]}"
    (( dist < lim )) && lim=0
  }
  while (( $#q > i )); do
    local -a node=(${=q[++i]})
    (( node[1] == lim )) && lim=0
    [[ $queued[${node[2,-1]}] == $node[1] ]] && $=1 $node
    if (( lim == 0 )); then
      q=(${(n)q:$i})
      lim=${q[$#q/24+1]%% *}
      i=0
    fi
  done
}

function visit-cells() {
  [[ $maze[$3] == '.' || $2 == 0 ]] || adjacent[$1]+="$2 $3 "
  [[ $maze[$3] == '.' || $1 == $3 ]] || return
  for 1 in -1 1 -$w $w; do
    [[ $maze[$1+$3] == '#' ]] || visit 1 $(($1+$3))
  done
}

function visit-nodes() {
  (( $3 && mask($4) )) && rcache[$1]+="$3 $4 "
  for 3 4 in ${=adjacent[$4]}; do
    (( !mask($4, 65) || $2 & mask($4, 65) )) && visit $3 $4
  done
}

function visit-states() {
  local -i i d x k=$2
  (( k == 1 << 26 - 1 )) && { echo $1; exit }
  for i in {3..$#}; do
    local m="$((k & doors[i-2])):${*[i]}"
    if [[ ! -v rcache[$m] ]]; then
      rcache[$m]=''
      bfs "visit-nodes $m $k" ${*[i]}
    fi
    for d x in ${=rcache[$m]}; do
      (( mask(x) & k )) || visit $d $((k | mask(x))) ${*[3,i-1]} $x ${*[i+1,-1]}
    done
  done
}

for x in {1..$#maze}; do
  [[ $maze[x] == ('#'|'.') ]] && continue
  (( doors[1 + ((x-1) % w >= c%w) + 2 * ((x-1) / w >= c/w)] |= mask(x, 65) ))
  bfs "visit-cells $x" $x
done

bfs visit-states 0 $((c-w-1)) $((c-w+1)) $((c+w-1)) $((c+w+1))
```

The struggle to implement efficient algorithms in Zsh taught me something new about the language.
The most important finding is that access into arrays and strings is not constant time. `v=a[i]` and
`a[i]=v` are 2000 times faster if `a` has length 10 than if it has length 10 million. In a few cases
I sharded my strings and arrays into 64-element chunks to speed up access.

```zsh
# `sharded-set a i v` is the equivalent of `a[i]=v`.
function sharded-set() {
  integer i=$(($2 - 1))
  typeset -g "${1}$((i>>6))[i&63+1]"=$3
}
```

Another source of difficulties when translating from C++ to Zsh is the lack of staple containers.
There is no deque, linked list or priority queue in Zsh. In fact, there are only three data types:
scalar, array and associative array. They don't compose. Elements of arrays and associative arrays
can only be scalars. You cannot create an array of arrays. There are no tuples, structs, objects,
pointers or references. This impoverished landscape makes it difficult to implement efficient
algorithms.

In the end, I've completed all puzzles in Zsh on the day there were published without hints or
looking up algorithms. On one hand, the puzzles were much easier than I expected, allowing me to
solve them in C++ with very little effort. On the other hand, porting to Zsh proved more challenging
than I thought. These two trends canceled each other out.

I'm happy with my choice of Zsh as the implementation language for Advent of Code. Even though I
haven't learned much new, I had *a lot* of fun. Without the extra challenge I probably would've
given up half way.

I'm looking forward to Advent of Code 2020!
