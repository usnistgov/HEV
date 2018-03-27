#! /bin/sh

convert -pointsize 16 -stroke yellow -font Courier \
-annotate 0x0+5+30 "BrickTexture"  \
brick.rgb brickTexture.png
