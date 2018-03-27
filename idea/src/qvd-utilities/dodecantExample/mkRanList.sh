#! /bin/bash

# create a list of 45963 random radius value .35 to .85

awk -v seed=$RANDOM '
BEGIN  {
    srand(seed);

    m=45963;

    for (i=0; i< m; i++) {
	r=.35+(rand()*.5);
       print r;
    }
}'

