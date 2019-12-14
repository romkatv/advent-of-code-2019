#!/usr/bin/env zsh

() {
  emulate -L zsh
  setopt no_aliases err_exit no_unset extended_glob pipe_fail warn_create_global ksh_arrays

  [[ $# == <1-4> ]]

  local -a _icc_mem
  IFS=, read -rA _icc_mem <<<$1
  [[ -z "${(@)_icc_mem:#(|-)<->}" ]]

  local _icc_input=${2:-'$((_icc_read()))'} _icc_output=${3:-_icc_print} _icc_done=${4:-:}
  local -i _icc_pc _icc_base _icc_mode

  [[ -z "${functions[(I)_icc_*]}" ]]
  trap 'unfunction -m "_icc_*"' EXIT

  function _icc_at() {
    (( $1 >= 0 ))
    unsetopt err_exit
    return ${_icc_mem[$1]:-0}
  }
  functions -M _icc_at 1

  function _icc_argpos() {
    local -i res
    case $(( _icc_mode % 10 )) in
      0) res='_icc_at(_icc_pc++)';;
      1) res='_icc_pc++';;
      2) res='_icc_base + _icc_at(_icc_pc++)';;
      *) exit 1
    esac
    (( res >= 0 && res < 1 << 31 ))
    (( _icc_mode /= 10, 1 ))
    unsetopt err_exit
    return res
  }
  functions -M _icc_argpos 0

  function _icc_fetch() { 
    local -i res='_icc_at(_icc_argpos())'
    unsetopt err_exit
    return res
  }
  functions -M _icc_fetch 0

  function _icc_store() {
    [[ $1 == (|-)<-> ]]
    (( _icc_mode % 10 != 1 ))
    _icc_mem[$(( _icc_argpos() ))]=$1
  }

  function _icc_read() {
    local res
    echo -nE - $prompt
    read -r res
    [[ $res == (|-)<-> ]]
    unsetopt err_exit
    return res
  }
  functions -M _icc_read 0

  function _icc_print() {
    echo -E - $1
  }

  function _icc_jump() { (( $1 )) || _icc_pc=$2 }

  [[ -z "${functions[(I)op_<->]}" ]]

  function _icc_99() { unsetopt err_exit; return 1                        }  # hlt
  function _icc_9()  { _icc_base+=$(( _icc_fetch() ))                     }  # rel
  function _icc_1()  { _icc_store $(( _icc_fetch() +  _icc_fetch() ))     }  # add
  function _icc_2()  { _icc_store $(( _icc_fetch() *  _icc_fetch() ))     }  # mul
  function _icc_7()  { _icc_store $(( _icc_fetch() <  _icc_fetch() ))     }  # lt
  function _icc_8()  { _icc_store $(( _icc_fetch() == _icc_fetch() ))     }  # eq
  function _icc_5()  { _icc_jump $(( !_icc_fetch() )) $(( _icc_fetch() )) }  # jnz
  function _icc_6()  { _icc_jump $((  _icc_fetch() )) $(( _icc_fetch() )) }  # jz
  function _icc_3()  { _icc_store "${(e)_icc_input}"                      }  # in
  function _icc_4()  { eval "$_icc_output $(( _icc_fetch() ))"            }  # out

  while true; do
    local -i _icc_instr=_icc_at(_icc_pc++)
    (( _icc_instr > 0 ))
    _icc_mode='_icc_instr / 100'
    local _icc_op=_icc_$((_icc_instr % 100))
    (( ${+functions[$_icc_op]} ))
    $_icc_op || break
    (( _icc_mode == 0 ))
  done

  eval "$_icc_done $_icc_mem"
} "$@"
