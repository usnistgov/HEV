#! /bin/bash


for ((i=0; i<101; i=i+1))
do
    length=$((i*3))
    percent=$i
    f=$(printf "pbar%03d" $i)

    echo ${f} ${precent} ${length}

    sed -e "s/LENGTH/${length}/" \
	-e "s/PERCENT/${percent}/" < proto.MVG > ${f}.mvg

    convert ${f}.mvg ${f}.png

    rm ${f}.mvg
done
exit

animate *.png

