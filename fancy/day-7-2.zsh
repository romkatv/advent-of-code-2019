#!/usr/bin/env zsh

local x program="$(<&0)"

function amp() {
  print -lr -- $*
  exec ./icc $program
}

for x in {56789..98765}; do
  [[ ${(j::)${(os::)x}} == 56789 ]] || continue
  coproc cat | amp $x[2] | amp $x[3] | amp $x[4] | amp $x[5] | amp $x[1] 0
  <&p >&p >&0 | tail -1
done | sort | tail -1
