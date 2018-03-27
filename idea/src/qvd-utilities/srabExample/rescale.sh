#! /bin/bash

# ./savg has the original rocks
# ./savg.66 should be an empty directory

cd savg
for f in *.savg
do
    echo $f
    savg-scale .66 .66 .66 < $f > ../savg.66/$f
done
