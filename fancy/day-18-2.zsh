#!/usr/bin/env zsh
#
# Usage: day-18-2.zsh <<<"your puzzle input"

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
      (( mask(x) & k )) || visit $d $((k | mask(x))) $*[3,i-1] $x $*[i+1,-1]
    done
  done
}

for x in {1..$#maze}; do
  [[ $maze[x] == ('#'|'.') ]] && continue
  (( doors[1 + ((x-1) % w >= c%w) + 2 * ((x-1) / w >= c/w)] |= mask(x, 65) ))
  bfs "visit-cells $x" $x
done

bfs visit-states 0 $((c-w-1)) $((c-w+1)) $((c+w-1)) $((c+w+1))
