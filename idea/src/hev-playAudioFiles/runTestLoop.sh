#! /bin/bash


while true
do
    for ((i=1; i<24; i++))
    do
	echo $i
    done
done | ./hev-playAudioFiles wav/*.wav




