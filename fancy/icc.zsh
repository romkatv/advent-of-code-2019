#!/usr/bin/env zsh
#
# Usage: icc.zsh <intcode> [prompt]
#
# Input/output instructions use stdin/stdout.
#
# If the second argument is specified, it's printed before reading
# the next number from stdin.
#
# Example: icc.zsh "3,0,2,0,0,0,4,0,99" "Number please: "
#
# Enter a number and press ENTER. The program will print the square
# of the number and exit.

local mem=(${(s:,:)1:?usage: icc.zsh <intcode> [prompt]})
local -i ip bp mode n

function argpos() {
  local -i m='mode % 10'
  (( mode /= 10 ))
  case $m in
    1) return '++ip'              ;;
    0) return 'mem[++ip] + 1'     ;;
    2) return 'mem[++ip] + 1 + bp';;
  esac
}

function fetch() { (( mem[argpos()]          )) }
function store() { (( mem[argpos()]=$1       )) }
function jumpc() { (( ip += ($1) * ($2 - ip) )) }

functions -M argpos 0
functions -M fetch  0

while true; do
  mode='mem[++ip] / 100'
  case $((mem[ip] % 100)); in
    99) exit 0                        ;;  # hlt
     9) bp+='fetch()'                 ;;  # rel
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
