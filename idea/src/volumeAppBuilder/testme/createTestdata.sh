#! /bin/bash

# create 5 data files (quick and dirty)

cp scaled_3Dcolorfield.ab step1.ab

cat step1.ab | awk '{if ($1 < .9) {print $1+.1;} else {print 1;}}' > step2.ab
cat step2.ab | awk '{if ($1 < .9) {print $1+.1;} else {print 1;}}' > step3.ab
cat step3.ab | awk '{if ($1 < .9) {print $1+.1;} else {print 1;}}' > step4.ab
cat step4.ab | awk '{if ($1 < .9) {print $1+.1;} else {print 1;}}' > step5.ab

