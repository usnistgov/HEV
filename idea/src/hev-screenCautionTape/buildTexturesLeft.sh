#! /bin/bash

# build osg files needed used by virtualCautionTape.iris

# 2.58-(1/10) => 2.48/2 => 1.24

# 4/92 => 0.04/2 => .02

# 12/124 => .087

# .25+.087 => .337

echo doing Left
hev-texturedRectangle --min -1.24 -.02 --max +1.24 +.02  cautionTapeLeft.png  tmp1.osgb
iris-convert --nolighting tmp1.osgb virtualCautionTapeLeft.osgb

rm -f tmp1.osgb tmp2.osgb
