#! /bin/bash

LUT=$1

../hev-createLUT < ${LUT}.dat > ${LUT}.lut

../hev-LUT2plot ${LUT}.lut ${LUT}Plot.png

display ${LUT}Plot.png





