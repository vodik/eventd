#!/bin/bash

(( $1 != 5 )) && exit
(( $2 != 2 )) && exit

if (( $3 == 0 )); then
  amixer -q set Master off
  mpc stop
else
  amixer -q set Master on
  mpc play
fi
