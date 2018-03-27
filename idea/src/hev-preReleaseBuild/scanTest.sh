#! /bin/bash


egrep      -i -e '[ \t]error[ \t]'  -e '[ \t]error$' \
              -e '[ \t]command not found[ \t]'  -e '[ \t]command not found$' | \
egrep -v -e 'dot: command not found' \


