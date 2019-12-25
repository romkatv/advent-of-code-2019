#!/usr/bin/env zsh
#
# Simple, clean and concise implementation of Intcode Computer in zsh.
#
# Usage: icc.zsh <intcode>
#
# Input/output instructions use stdin/stdout.
#
# Example: icc.zsh 3,0,102,2,0,0,4,0,99 <<<21
# Output:  42

local mem REPLY
local -i pc base mode

IFS=, read -rA mem <<<${1:?usage: icc.zsh <intcode>}

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
     3) read -r; store REPLY          ;;  # in
     4) echo - $((fetch()))           ;;  # out
  esac
done
