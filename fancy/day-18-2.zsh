#!/usr/bin/env zsh

local -ri w=81 h=81 c='40*81+41'
local maze="${$(<&0)//$'\n'}"
local -A adjacent rcache
local -a doors=(0 0 0 0)

maze[c-w-1,c-w+1]="@#@"
maze[c+0-1,c+0+1]="###"
maze[c+w-1,c+w+1]="@#@"

function kmask() {
  local c=$maze[$1]
  return '#c >= 97 && #c <= 122 ? 1 << (#c - 97) : 0'
}
function dmask() {
  local c=$maze[$1]
  return '#c >= 65 && #c <= 90 ? 1 << (#c - 65) : 0'
}
functions -M kmask 1
functions -M dmask 1

function bfs() {
  local -A visited
  local state q=("0 $*[2,-1]")
  local -i dist i m
  while (( $#q > i )); do
    local -a node=(${=q[++i]})
    (( node[1] == m )) && m=0
    if [[ ! -v visited[${node[2,-1]}] ]]; then
      visited[${node[2,-1]}]=""
      (( node[1] )) && echo -E - $node
      typeset -g reply=()
      $=1 $node[2,-1]
      for dist state in $reply; do
        (( dist += node[1] ))
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
  [[ $maze[$2] == '.' || $1 == $2 ]] || return
  local -i x
  for x in -1 1 -$w $w; do
    [[ $maze[$2+x] == '#' ]] || reply+=(1 $(($2+x)))
  done
}

function adjacent-nodes() {
  local -i d x
  for d x in ${=adjacent[$2]}; do
    (( !dmask(x) || $1 & dmask(x) )) && reply+=($d $x)
  done
}

function adjacent-states() {
  local -i i d x
  for i in {2..$#}; do
    local m="$(($1 & doors[i-1])):${*[i]}"
    if [[ ! -v rcache[$m] ]]; then
      rcache[$m]=''
      bfs "adjacent-nodes $1" ${*[i]} | while read -r d x; do
        (( kmask(x) )) && rcache[$m]+="$d $x "
      done
    fi
    for d x in ${=rcache[$m]}; do
      (( kmask(x) & $1 )) && continue
      local state=($*)
      (( state[1] |= kmask(x), state[i] = x ))
      reply+=($d "$state")
    done
  done
}

local x y d k
for x in {1..$#maze}; do
  [[ $maze[x] == ('#'|'.') ]] && continue
  (( doors[1 + ((x - 1) % w >= c % w) + 2 * ((x - 1) / w >= c / w)] |= dmask(x) ))
  bfs "adjacent-cells $x" $x | while read -r d y; do
    [[ $maze[y] == '.' ]] || adjacent[$x]+="$d $y "
  done
done

bfs adjacent-states 0 $((c-w-1)) $((c-w+1)) $((c+w-1)) $((c+w+1)) | while read -r d k x; do
  if (( k == 1 << 26 - 1 )); then
    echo $d
    break
  fi
done
