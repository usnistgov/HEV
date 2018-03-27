#! /bin/bash


cat > virtualCautionTape.iris <<EOF
LOAD vCautionTapeLeft  ${HEVROOT}/idea/etc/hev-screenCautionTape/data/virtualCautionTapeLeft.osgb

DCS vCautionTapeLeftDCS -.87 -.337 -.995 90 -90 0

ADDCHILD vCautionTapeLeft vCautionTapeLeftDCS

ADDCHILD vCautionTapeLeftDCS scene

# -----------------

LOAD vCautionTapeFront ${HEVROOT}/idea/etc/hev-screenCautionTape/data/virtualCautionTapeFront.osgb

DCS vCautionTapeFrontDCS .03 .88 -.995 0 -90 -90

ADDCHILD vCautionTapeFront vCautionTapeFrontDCS

ADDCHILD vCautionTapeFrontDCS scene
EOF

