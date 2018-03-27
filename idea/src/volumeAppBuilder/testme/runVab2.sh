#! /bin/bash


rm -r volVisApp

volumeAppBuilder $* --scale 100 --lut ctable_.01.lut --dim 79 79 79 step*.ab

