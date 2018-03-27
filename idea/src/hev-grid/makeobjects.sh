# unit box
savg-cube | savg-rgb .300 .300 .300 | savg-wireframe  > foo.savg

# interior sub-lines of unit box
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0  >> foo.savg
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 90 0 0 >> foo.savg
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 0 0 90 >> foo.savg

# +X
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-translate 0 1 0 | savg-rotate -90 0 0 >> foo.savg
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 0 0 90 | savg-translate 0 1 0 | savg-rotate -90 0 0 >> foo.savg

# -X
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-translate 0 1 0 | savg-rotate 90 0 0 >> foo.savg
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 0 0 90 | savg-translate 0 1 0 | savg-rotate 90 0 0 >> foo.savg

# +Y
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-translate 0 1 0  >> foo.savg
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 0 0 90 | savg-translate 0 1 0 >> foo.savg

# -Y
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-translate 0 -1 0  >> foo.savg
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 0 0 90 | savg-translate 0 -1 0 >> foo.savg

# +Z
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-translate 0 1 0 | savg-rotate 0 90 0 >> foo.savg
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 0 0 90 | savg-translate 0 1 0 | savg-rotate 0 90 0 >> foo.savg

# -Z
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-translate 0 1 0 | savg-rotate 0 -90 0 >> foo.savg
savg-line | savg-rgb .400 .400 .400 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 0 0 90 | savg-translate 0 1 0 | savg-rotate 0 -90 0 >> foo.savg

cat foo.savg | savg-scale .5 | savg-rgb .500 .500 .500 > bar.savg
cat bar.savg >> foo.savg

cat foo.savg | savg-scale .25 | savg-rgb .600 .600 .600 > bar.savg
cat bar.savg >> foo.savg

osgOptimize --osgLightingOff foo.savg hev-cubeObject.osg

# axes
rm -f foo.savg
for y in .1 .2 .3 .4 .6 .8 1; do
    savg-line | savg-rgb .700 .700 .700 | savg-scale 2 | savg-translate -1 0 0 | savg-translate 0 $y 0 >> foo.savg
    savg-line | savg-rgb .700 .700 .700 | savg-scale 2 | savg-translate -1 0 0 | savg-rotate 0 0 90 | savg-translate 0 $y 0 >> foo.savg
done
osgOptimize --osgLightingOff foo.savg hev-axesObject.osg

# picker object
cat whiteMarker.savg | savg-rgb 1 0 0 > foo.savg
osgOptimize foo.savg hev-pickerObject.osg

# nearest object
w=".04"
echo -e "lines 1 1 1 1\n-$w 0 0\n$w 0 0\nlines\n0 -$w 0\n0 $w 0\nlines\n0 0 -$w\n0 0 $w" > foo.savg
osgOptimize --osgLightingOff foo.savg hev-nearestObject.osg

# proximity object
rm -f bar.savg
w=".02"
s="-.2 -.1 0 .1 .2"
echo -e "lines .666 .666 .666 1\n-$w 0 0\n$w 0 0\nlines\n0 -$w 0\n0 $w 0\nlines\n0 0 -$w\n0 0 $w" > foo.savg
for x in $s; do
    for y in $s; do
	for z in $s; do
	    savg-translate $x $y $z < foo.savg >> bar.savg
	done
    done
done
osgOptimize --osgLightingOff bar.savg hev-proximityObject.osg

# background grid objects
rm -f bar.savg
w=".01"
s="-1 -.8 -.6 -.4 -.2 0 .2 .4 .6 .8  1"
echo -e "lines .333 .333 .333 1\n-$w 0 0\n$w 0 0\nlines\n0 -$w 0\n0 $w 0\nlines\n0 0 -$w\n0 0 $w" > foo.savg
for x in $s ; do
    for y in $s ; do
	for z in  $s ; do
	    savg-translate $x $y $z < foo.savg >> bar.savg
	done
    done
done
osgOptimize --osgLightingOff bar.savg hev-backgroundObject.osg

rm -f foo.savg bar.savg
