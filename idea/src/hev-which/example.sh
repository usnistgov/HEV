#! /bin/sh

# example usage

hev-which -q
h=$?

if [ $h -eq 20 ]
then
    echo HEV IRIS environment is set up
fi

