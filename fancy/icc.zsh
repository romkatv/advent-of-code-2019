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

emulate -L zsh -o ksh_arrays

local -a mem
IFS=, read -rA mem <<<${1:?usage: icc.zsh <intcode>}

local -i p base mode

function argpos() {
  local -i m='mode % 10'
  (( mode /= 10 ))
  case $m in
    1) return 'p++';;
    0) return 'mem[p++]';;
    2) return 'mem[p++] + base';;
  esac
}
functions -M argpos 0

function fetch() { return 'mem[argpos()]' }
functions -M fetch 0

function store() { mem[$(( argpos() ))]=$1 }

function readnum() {
  local res
  read -r res
  return res
}
functions -M readnum 0

function jumpc() { (( $1 )) || p=$2 }

function op_99() { exit 0                               }  # hlt
function op_9()  { base+=$(( fetch() ))                 }  # rel
function op_1()  { store $(( fetch() +  fetch() ))      }  # add
function op_2()  { store $(( fetch() *  fetch() ))      }  # mul
function op_7()  { store $(( fetch() <  fetch() ))      }  # lt
function op_8()  { store $(( fetch() == fetch() ))      }  # eq
function op_5()  { jumpc $(( !fetch() )) $(( fetch() )) }  # jnz
function op_6()  { jumpc $((  fetch() )) $(( fetch() )) }  # jz
function op_3()  { store $(( readnum() ))               }  # in
function op_4()  { echo - $(( fetch() ))                }  # out

while true; do
  mode='mem[p] / 100'
  op_$((mem[p++] % 100))
done
