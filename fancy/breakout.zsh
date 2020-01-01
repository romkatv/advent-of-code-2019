#!/usr/bin/env zsh
#
# Terminal-based Breakout game.

local -ra tiles=(0 7 4 2 1)
local -rA keys=(a -1 d 1)

exec 2>/dev/null
trap 'kill $!; echoti cnorm; stty echo' EXIT INT QUIT ILL PIPE TERM ZERR
coproc ${ZSH_SCRIPT:h}/icc.zsh "$(<${ZSH_SCRIPT:h}/breakout.ic)" $'in\n'
echoti clear; echoti civis; stty -echo

while read -p x; do
  case $x in
    in) sleep 0.5
        local joystick=0
        while read -rkt x; do joystick=${keys[${(L)x}]:-0}; done
        print -p $joystick;;
    -1) read -p dummy; read -p score
        echoti home
        print -n "Score: $score";;
     *) read -p y; read -p tile
        echoti cup $((y+2)) $x
        print -nP "%K{$tiles[tile+1]} %k";;
  esac
done
