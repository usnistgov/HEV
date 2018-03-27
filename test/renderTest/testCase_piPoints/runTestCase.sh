#! /bin/bash

# runTestCase.sh
# June 17, 2015
# Steve Satterfield

LOG=/tmp/$(basename $0 .sh)_${USER}_$$.log
TC=$( basename $(pwd) )

export OSG_FILE_PATH=$OSG_FILE_PATH:$HEV_IDEA_DIR/testSuite
export PATH=$HEVROOT/test/renderTest/bin:$PATH 


# Remove previous frameGrabber images to ensure we are later testing
# against newly created images
rm -f /tmp/frameGrabber-${USER}*


# Generate new images
irisfly --ex sideViews.timeline.iris piPoints.savg >  $LOG 2>&1 


# Compare new images against base line
exitCode=0

pngCount=$(ls -1 baseLine_*.pam | wc -l)
for ((i=0; i<$pngCount; i++))
do
    n=$(printf "%06d" $i)

    newfile="/tmp/frameGrabber-${USER}-snap-perspective-${n}.pam"
    basefile="baseLine_${n}.pam"

    # Do the test
    hev-testRenderCmp $newfile $basefile >>  $LOG  2>&1

    # Check the result
    if [ $? -ne 0 ]
    then
	echo "${TC} base line $i: FAILED"
	echo "   Generated file differs from base line $i"
	echo "   Baseline: $basefile"
	echo "   Generated: $newfile"
	exitCode=1
    else
	echo "testCase_piPoints base line $i: PASSED"
    fi
done

# Return exit code
exit $exitCode
