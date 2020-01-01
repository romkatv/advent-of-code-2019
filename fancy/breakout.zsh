#!/usr/bin/env zsh
#
# Terminal-based Breakout game.
#
# Usage: ./breakout.zsh "your puzzle input"

local -ra tiles=(0 7 4 2 1)
local -rA keys=(a -1 d 1)

trap 'echoti cnorm; stty echo' EXIT INT QUIT ILL PIPE TERM ZERR
echoti clear; echoti civis; stty -echo
coproc ./icc.zsh ${1/#1/2} $'in\n'

while read -p x; do
  case $x in
    in) sleep 0.25
        local joystick=0
        while read -rkt x; do joystick=${keys[${(L)x}]:-0}; done
        print -p $joystick;;
    -1) read -p dummy; read -p score
        echoti home
        print -nP "Score: $score%E";;
     *) read -p y; read -p tile
        echoti cup $((y+2)) $x
        print -nP "%K{$tiles[tile+1]} %k";;
  esac
done
