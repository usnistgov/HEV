#!/bin/bash

LOG=/tmp/$(basename $0 .sh)_${USER}_$$.log
TC=$(basename $(pwd))

export OSG_FILE_PATH=${OSG_FILE_PATH}:${HEV_IDEA_DIR}/testSuite
export PATH=${HEVROOT}/test/renderTest/bin:${PATH}

savg-line > /tmp/savg-line.savg
diff ${HEV_IDEA_DIR}/testSuite/savgUtilities_savg-line.savg \
    /tmp/savg-line.savg > $LOG 2>&1

if [ $? -eq 1 ]; then
    echo "savg-line output differs: check ${LOG}"
    exit 1
fi

savg-rgb 0.2 0.4 0.6 < /tmp/savg-line.savg > /tmp/savg-rgb.savg
diff ${HEV_IDEA_DIR}/testSuite/savgUtilities_savg-rgb.savg \
    /tmp/savg-rgb.savg > $LOG 2>&1

if [ $? -eq 1 ]; then
    echo "savg-rgb output differs: check ${LOG}"
    exit 1
fi

savg-alpha 0.8 < /tmp/savg-rgb.savg > /tmp/savg-alpha.savg
diff ${HEV_IDEA_DIR}/testSuite/savgUtilities_savg-alpha.savg \
    /tmp/savg-alpha.savg > $LOG 2>&1

if [ $? -eq 1 ]; then
    echo "savg-alpha output differs: check ${LOG}"
    exit 1
fi

savg-translate  3 2 1 < /tmp/savg-alpha.savg > /tmp/savg-translate.savg
diff ${HEV_IDEA_DIR}/testSuite/savgUtilities_savg-translate.savg \
    /tmp/savg-translate.savg > $LOG 2>&1

if [ $? -eq 1 ]; then
    echo "savg-translate output differs: check ${LOG}"
    exit 1
fi

savg-rm-rgba < /tmp/savg-translate.savg > /tmp/savg-rm-rgba.savg
diff ${HEV_IDEA_DIR}/testSuite/savgUtilities_savg-rm-rgba.savg \
    /tmp/savg-rm-rgba.savg > $LOG 2>&1

if [ $? -eq 1 ]; then
    echo "savg-rm-rgba output differs: check ${LOG}"
    exit 1
fi
