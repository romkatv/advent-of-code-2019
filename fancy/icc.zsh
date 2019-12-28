#!/usr/bin/env zsh
#
# Usage: icc.zsh <intcode> [prompt]
#
# Input/output instructions use stdin/stdout.
#
# If the second argument is specified, it's passed through `eval`
# before reading the next number from stdin.
#
# Example: icc.zsh "3,0,2,0,0,0,4,0,99" "echo -n 'Number please: '"
#
# Enter a number and press ENTER. The program will print the square
# of the number and exit.

local mem=(${(s:,:)1:?usage: icc.zsh <intcode> [prompt]})
local -i pc base mode num

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
     3) eval $2; read num; store num  ;;  # in
     4) echo - $((fetch()))           ;;  # out
  esac
done
