#!/usr/bin/env zsh

integer m=119315717514047 n=101741582076661 start=2020

function add() { return "(($1 + $2) % m + m) % m" }
functions -M add 2

for f g in mul add pow mul; do
  functions[$f]='
    integer a=$1 b="$2-1" res=$1
    while (( b )); do
      (( b & 1 == 0 )) || res="'$g'(res, a)"
      a="'$g'(a, a)"
      (( b >>= 1 ))
    done
    return res'
  functions -M $f 2
done

integer k=1 b=0 x
while read -r s; do
  case $s in
    *inc*) (( b = mul(b, ${s##* })     , k = mul(k, ${s##* }) ));;
    *new*) (( b = add(-1, -b)          , k = add(0, -k)       ));;
    *cut*) (( b = add(b, -1 * ${s##* }), k = k                ));;
  esac
done

(( k = pow(k, m-2), x=mul(mul(-b, k), pow(add(k, -1), m-2)) ))
echo $((add(mul(add(x, start), pow(k, n)), -x)))
