#! /bin/bash

./mkRanList.sh > radiusList.dat

rm -r vaneRheometerVis

qvd-rheometerAppBuilder --verbose --noclean --gpuSpheres  --vane 6 \
 --listRadius radiusList.dat \
 vane.qvisdat dodecant*.qvisdat
