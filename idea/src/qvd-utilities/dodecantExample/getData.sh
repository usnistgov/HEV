#! /bin/bash

src=~wgeorge/nobackup/QDPD/Six-blade-Rheom/work-2013-03-13-11.28.53-sph-42-50-hex-vane-T1000-hex/Output-t1120.007112

l=`qvd-frameList 1 113 10`

for f in $src/dod*.qvisdat $src/vane.qvisdat
do
    b=`basename $f`
    echo $b

    qvd-extractFrames --renumber $l < $f > $b



done
