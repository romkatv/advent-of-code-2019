#!/usr/bin/env zsh
#
# Usage: ./day-7-2.zsh ./icc.zsh "your puzzle input"

local intcode=("$@")

function amp() { print -l $*; exec "${intcode[@]}" }

for x in {56789..98765}; do
  [[ ${(j::)${(os::)x}} == 56789 ]] || continue
  coproc cat | amp $x[2] | amp $x[3] | amp $x[4] | amp $x[5] | amp $x[1] 0
  <&p >&p >&0 | tail -1
done | sort | tail -1
