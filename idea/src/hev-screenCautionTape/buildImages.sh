#! /bin/bash

# Script to create cautionTapeLeft.png and cautionTapeFront.png
#
# Run manually if it necessary to re-create cautionTapeLeft.png
# or cautionTapeFront.png 
# 
# The cautionTapeOriginal.jpg I found with google
#
# 1200 pixels high/92 inches => approx 13 pixels/inch
#
# cautionTapeOriginal.jpg is 3938x464
# which implies 303" x 36"
#
# To convert it to a 4" image scale by .11111
#
# resize to 3938*4/36 x 464*4/36 => 437x52
#
# 1600/437 => 3.66
#
# So crop original to 400x51 and join 4 copies
#
# SGS, 9/15/2014

convert cautionTapeOriginal.jpg -resize 437x52 -crop 400x51+0+0 ctape4inch.png

convert ctape4inch.png ctape4inch.png -gravity west +append c2x.png

#convert c2x.png c2x.png -gravity west +append                   cautionTapeLeft.png
 convert c2x.png c2x.png -gravity west +append -crop 1444x51+0+0 cautionTapeLeft.png

#convert cautionTapeLeft.png -crop 1200x51+0+0                   -rotate 90 cautionTapeFront.png
 convert cautionTapeLeft.png -crop 1200x51+0+0 -crop 1044x51+0+0 -rotate 90 cautionTapeFront.png

rm -f ctape4inch.png  c2x.png
