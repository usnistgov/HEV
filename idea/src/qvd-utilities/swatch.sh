#! /bin/bash

b=.4   # "black"
c=.7   # "color"
w=.85  # "white"

i=.5
j=.3

swatch    0  0 $c  &  # blue
swatch   $i $i $c  &

swatch    0 $c  0  &  # green
swatch   $i $c $i  &

swatch    0 $w $w  &  # cyan
swatch    0 $c $c  &

swatch   $c  0  0  &  # red
swatch   $c $j $j  &

swatch   $c  0 $c  &  # magenta
swatch   $i  0 $i  &

swatch   $w $w  0  &  # yellow
swatch   $c $c  0  &
