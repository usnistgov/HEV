#! /bin/bash



rm -rf volVisApp

volumeAppBuilder --debug --lut ctable_.9.lut --dim 79 79 79 scaled_3Dcolorfield.ab
