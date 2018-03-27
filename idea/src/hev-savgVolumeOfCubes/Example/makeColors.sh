#! /bin/bash

case $# in
3)
      export XSIZE=$1
      export YSIZE=$2
      export ZSIZE=$3
      ;;
*)
      echo "Usage: makeColors.sh n m o" > /dev/tty
      exit
esac


awk '
BEGIN {
    n=ENVIRON["XSIZE"];
    m=ENVIRON["YSIZE"];
    o=ENVIRON["ZSIZE"];

    rgb[0]="0 0 .5";
    rgb[1]="0 0 1";
    rgb[2]="0 .5 0";
    rgb[3]="0 1 0";
    rgb[4]="0 .5 .5";
    rgb[5]="0 1 1";
    rgb[6]=".5 0 0";
    rgb[7]="1 0 0";
    rgb[8]=".5 0 .5";
    rgb[9]="1 0 1";
    rgb[10]=".5 .5 0";
    rgb[11]="1 1 0";

    c=0;
    for (k=0; k<o; k++) {
	for (j=0; j<m; j++) {
	    for (i=0; i<n; i++) {
		r=i/n;
		g=j/m;
		b=k/o;
		#print i,j,k, r,g,b;
		print rgb[c++];
		if (c>11) {c=0;}
	    }
	}
    }
}
'
