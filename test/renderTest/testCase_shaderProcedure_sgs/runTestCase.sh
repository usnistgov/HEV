#! /bin/bash

# runTestCase.sh
# June 17, 2015
# Steve Satterfield

# echo ${0}: Start...   # Debug
LOG=/tmp/$(basename $0 .sh)_${USER}_$$.log
TC=$( basename $(pwd) )

# echo ${0}: Log: ${LOG}   TC: ${TC}   # Debug

export OSG_FILE_PATH=$OSG_FILE_PATH:$HEV_IDEA_DIR/testSuite
export PATH=$HEVROOT/test/renderTest/bin:$PATH 
# echo ${0}: Path env vars set...   # Debug

# Remove previous frameGrabber images to ensure we are later testing
# against newly created images
#echo ${0}: Removing olf frame grabber files...   # Debug
rm -f /tmp/frameGrabber-${USER}*
#echo ${0}: ...done. Start irisfly...   # Debug


# Generate new images
irisfly  --background 1 1 1  desktopWindowTraits512.iris   shaderProcedure.iris  shaderProcedureControls.timeline.iris  >  $LOG 2>&1
# echo ${0}: ..done.   # Debug

# Compare new images against base line
exitCode=0

# echo ${0}: Comparing images...   # Debug
pamCount=$(ls -1 baseLine_*.pam | wc -l)
for ((i=0; i<$pamCount; i++))
do
    n=$(printf "%06d" $i)

    newfile="/tmp/frameGrabber-${USER}-snap-perspective-${n}.pam"
    basefile="baseLine_${n}.pam"

    # Do the test
    compare-binary $newfile $basefile >>  $LOG  2>&1

    # Check the result
    if [ $? -ne 0 ]
    then
	echo "${TC} base line $i: FAILED"
	echo "   Generated file differs from base line $i"
	echo "   Baseline: $basefile"
	echo "   Generated: $newfile"
	exitCode=1
    else
	echo "testCase_shaderProcedure base line $i: PASSED"
    fi
done
# echo ${0}: ...Done, Exiting   # Debug
# Return exit code
exit $exitCode
