#! /bin/bash

cat piPoints.raw | savg-translate -58.62840271 -11.17425060 -9.45026302 | \
savg-scale .026049162347 .026049162347 .026049162347 | \
savg-rotate 0 90 0 > piPoints.savg

