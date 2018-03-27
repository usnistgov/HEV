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

    s[0]=.25;
    s[1]=.5;
    s[2]=.75;
    s[3]=-1;

    c=0;
    for (k=0; k<o; k++) {
	for (j=0; j<m; j++) {
	    for (i=0; i<n; i++) {
		r=i/n;
		g=j/m;
		b=k/o;
		#print i,j,k, r,g,b;
		print s[c],s[c],s[c];
                c++;
		if (c>3) {c=0;}
	    }
	}
    }
}
'
