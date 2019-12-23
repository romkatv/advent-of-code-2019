#!/usr/bin/env zsh

integer m=119315717514047 n=101741582076661 start=2020

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
echo $((add(mul(add(x, start), pow(pow(k, m-2), n)), -x)))
