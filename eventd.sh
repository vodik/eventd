#!/bin/bash

(( $1 != 1 || $3 != 1 )) && exit

(( $2 == 115 )) && exec ponymix increase 5
(( $2 == 114 )) && exec ponymix decrease 5
