#!/bin/sh

a="$1 $2 $3"; shift 3
b="$1 $2 $3"; shift 3
c="$1 $2 $3"; shift 3
d="$1 $2 $3"; shift 3

cat << EOF > tetrahedronT.savg
polygon .5 .5 .5 .25
$a
$b
$c

polygon .5 .5 .5 .25
$a
$c
$d

polygon .5 .5 .5 .25
$a
$d
$b

polygon .5 .5 .5 .25
$b
$d
$c
EOF


