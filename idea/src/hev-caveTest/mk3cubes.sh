#! /bin/sh

hev-savgCube | hev-savgTransformation --scale .25 .25 .25 --xyz  0  1 0 | savg-color -r 1 -b 1 -g 1  > 3cubes.savg
hev-savgCube | hev-savgTransformation --scale .25 .25 .25 --xyz -1  0 0 | savg-color -r 0 -b 1 -g 1 >> 3cubes.savg
hev-savgCube | hev-savgTransformation --scale .25 .25 .25 --xyz  0  0 -1 | savg-color -r 1 -b 1 -g 0 >> 3cubes.savg


