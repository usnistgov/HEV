#!/bin/sh

pages=`find $HEVROOT/idea/man -type f`

for f in $pages
do  p=`basename $f`.pdf
    if [ ! -e "$p" -o "$f" -nt "$p" ]
    then dtk-msgNotice -p IDEA building $p 
	 groff -man $f |  ps2pdf - > $p
    fi
done
