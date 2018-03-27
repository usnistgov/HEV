#! /bin/bash

# build osg files needed used by virtualCautionTape.iris

# 1.94-(1/10) => 1.84/2 => .92

# 1 - .087 => .913

echo doing Front
hev-texturedRectangle  --min -.02 -.92 --max +.02 +.92  cautionTapeFront.png  tmp2.osgb
iris-convert --nolighting tmp2.osgb virtualCautionTapeFront.osgb

rm -f tmp1.osgb tmp2.osgb
