#! /bin/bash

cat > line.savg <<EOF
lines
-1 0 0
+1 0 0
EOF

cat line.savg | hev-tubify -r .0075 -f SAVG | savg-color -r 1 -g 1 -b .5 -a 1 > tube.savg
rm -f line.savg

s=.1
y=-.01
z=-.01
savg-text "5" | savg-scale $s $s $s | savg-translate  1  $y $z | savg-color -r 1 -g 1 -b .5 -a 1 > 5.savg
savg-text "4" | savg-scale $s $s $s | savg-translate  .6 $y $z | savg-color -r 1 -g 1 -b .5 -a 1 > 4.savg
savg-text "3" | savg-scale $s $s $s | savg-translate  .2 $y $z | savg-color -r 1 -g 1 -b .5 -a 1 > 3.savg
savg-text "2" | savg-scale $s $s $s | savg-translate -.2 $y $z | savg-color -r 1 -g 1 -b .5 -a 1 > 2.savg
savg-text "1" | savg-scale $s $s $s | savg-translate -.6 $y $z | savg-color -r 1 -g 1 -b .5 -a 1 > 1.savg
savg-text "0" | savg-scale $s $s $s | savg-translate -1  $y $z | savg-color -r 1 -g 1 -b .5 -a 1 > 0.savg



cat > fade.iris <<EOF
dcs  tube 0 .1 .4   45 0 0  1.5
load tube0 tube.savg
addchild tube0 tube
addchild tube scene
nodemask tube off
nodemask tube0 off

load t5 5.savg
load t4 4.savg
load t3 3.savg
load t2 2.savg
load t1 1.savg
load t0 0.savg

nodemask t5 off
nodemask t4 off
nodemask t3 off
nodemask t2 off
nodemask t1 off
nodemask t0 off

addchild t5 tube
addchild t4 tube
addchild t3 tube
addchild t2 tube
addchild t1 tube
addchild t0 tube
EOF