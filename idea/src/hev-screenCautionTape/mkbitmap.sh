#! /bin/bash

# Experiment with making a bitmap suitable for xsetroot which I thought
# I would use to put the caution tape into hev-initXscreens, but
# as far as I can tell, xsetroot only allows .xbm with is B/W.
#
# So this idea didn't work out

# Make a bit map suitable for xsetroot

convert -size 1600x1200 xc:"#0000aa" -depth 8 tmp0.png

composite cautionTapeLeft.png  tmp0.png -geometry +0+156    tmp1.png

composite cautionTapeFront.png tmp1.png -geometry +1393+156 tmp2.png
