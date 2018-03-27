#! /bin/bash

cat dodecant201_page_00001.savg | awk -v seed=$RANDOM '
BEGIN{srand(seed);}
{
    if ($1 == "POINTS") {
       print $1;
   } else {
       r=rand()-.5;
       print $1,$2,$3,$4,$5,$6,$7+r;
   }
}'


