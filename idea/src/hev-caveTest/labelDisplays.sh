#! /bin/bash

trap 'kill $(jobs -p)' 0


H=$(hev-identifyImmersive)
case $H in
rave)
        DN="0 1 2 3"
	s[0]="Console Monitor"
	s[1]="Front Projector"
	s[2]="Left  Projector"
	s[3]="Floor Projector"
	;;
powerwall)
        DN="0 1"
	s[0]="Console Monitor"
	s[1]="Front Projector"
        ;;
*)
        DN="0"
	s[0]="Console Monitor"
        ;;
esac

tempName=/tmp/labelDisplays.sh.$$

for i in $DN
do
    d=":0.${i}"
    export DISPLAY=$d
    echo "X DISPLAY=$d should be on ${s[$i]}" > $tempName

    hev-messageBox -geometry +500+50 --scale 2 < $tempName &

    rm -f $tempName

    # I would have rather done something like:
    # echo message | hev-messageBox ...
    # but the pipe seems to hide the hev-messageBox from the kill command

done

wait


