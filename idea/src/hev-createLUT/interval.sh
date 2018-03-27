#! /bin/sh

# print the index for a specified number of iterval between 0 256

case $# in
1)
     i=$1
     ;;
*)
     echo "Usage: ./interval.sh interval"
     exit
     ;;
esac

echo $i | awk '
{
    d=int(256/$1);
    for (i=0;i<256;i=i+d) {
	print i;
    }
}'
