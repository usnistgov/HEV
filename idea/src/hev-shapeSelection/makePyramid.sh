#!/bin/sh

a="0 0 1.73205080756887729352"
b=" 1  1 -1"
c=" 1 -1 -1"
d="-1 -1 -1"
e="-1  1 -1"

cat << EOF > pyramidT.savg
polygon .5 .5 .5 .25
$a
$c
$b

polygon .5 .5 .5 .25
$a
$d
$c

polygon .5 .5 .5 .25
$a
$e
$d

polygon .5 .5 .5 .25
$a
$b
$e

polygon .5 .5 .5 .25
$b
$c
$d
$e
$b
EOF
