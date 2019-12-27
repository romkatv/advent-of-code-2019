#!/usr/bin/env zsh

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
  local state q=("0 $*[2,-1]")
  local -i dist i m
  while (( $#q > i )); do
    local -a node=(${=q[++i]})
    (( node[1] == m )) && m=0
    if [[ $queued[${node[2,-1]}] == $node[1] ]]; then
      local reply=()
      $=1 $node
      for dist state in $reply; do
        (( dist += node[1] ))
        (( dist < ${queued[$state]-dist+1} )) || continue
        queued[$state]=$dist
        q+="$dist $state"
        (( dist < m )) && m=0
      done
    fi
    if (( m == 0 )); then
      q=(${(n)q:$i})
      m=${q[$#q/16+1]%% *}
      i=0
    fi
  done
}

function adjacent-cells() {
  [[ $maze[$3] == '.' || $2 == 0 ]] || adjacent[$1]+="$2 $3 "
  [[ $maze[$3] == '.' || $1 == $3 ]] || return
  for 1 in -1 1 -$w $w; do
    [[ $maze[$1+$3] == '#' ]] || reply+=(1 $(($1+$3)))
  done
}

function adjacent-nodes() {
  (( $3 && mask($4) )) && rcache[$1]+="$3 $4 "
  for 3 4 in ${=adjacent[$4]}; do
    (( !mask($4, 65) || $2 & mask($4, 65) )) && reply+=($3 $4)
  done
}

function adjacent-states() {
  echo -E - $* >&2
  local -i i d x k=$2
  for i in {3..$#}; do
    local m="$((k & doors[i-2])):${*[i]}"
    if [[ ! -v rcache[$m] ]]; then
      rcache[$m]=''
      bfs "adjacent-nodes $m $k" ${*[i]}
    fi
    for d x in ${=rcache[$m]}; do
      (( mask(x) & k )) && continue
      reply+=($d "$((k | mask(x)))${${i:#3}:+ }${*[3,i-1]} $x${${i:#$#}:+ }${*[i+1,-1]}")
    done
  done
  #exit
  (( $#reply )) && return
  echo $1
  exit
}

for x in {1..$#maze}; do
  [[ $maze[x] == ('#'|'.') ]] && continue
  (( doors[1 + ((x-1) % w >= c%w) + 2 * ((x-1) / w >= c/w)] |= mask(x, 65) ))
  bfs "adjacent-cells $x" $x
done

bfs adjacent-states 0 $((c-w-1)) $((c-w+1)) $((c+w-1)) $((c+w+1))
