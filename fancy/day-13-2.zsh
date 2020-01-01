#!/usr/bin/env zsh
#
# Usage: ./day-13-2.zsh <<<"your puzzle input"

coproc ${ZSH_SCRIPT:h}/icc.zsh ${"$(<&0)"/#1/2} $'in\n'

while read -p x; do
  case $x in
    in) print -p $((ball > paddle ? 1 : ball < paddle ? -1 : 0));;
    -1) read -p dummy; read -p score;;
     *) read -p y; read -p tile
        [[ $tile == 3 ]] && paddle=$x
        [[ $tile == 4 ]] && ball=$x;;
  esac
done

echo $score
