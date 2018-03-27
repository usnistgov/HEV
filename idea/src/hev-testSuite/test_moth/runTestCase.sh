
#! /bin/bash              

# runTestCase.sh
# June 17, 2015 
# Steve Satterfield

LOG=/tmp/$(basename $0 .sh)_${USER}_$$.log
TC=$( basename $(pwd) )

export OSG_FILE_PATH=${OSG_FILE_PATH}:${HEV_IDEA_DIR}/testSuite:${HEV_IDEA_DIR}/testSuite/moth_data
export PATH=${HEVROOT}/test/renderTest/bin:${PATH}


# Remove previous frameGrabber images to ensure we are later testing
# against newly created images
rm -f /tmp/frameGrabber-${USER}*


# Generate new images
irisfly  --background 1 1 1  --ex moth.iris  moth.timeline.iris  >  $LOG 2>&1 #desktopWindowTrait512.iris

# Compare new images against base line
exitCode=0

pamCount=$(ls -1 baseLine_*.pam | wc -l)
for ((i=0; i<${pamCount}; i++))
do
    n=$(printf "%06d" ${i})

    newfile="/tmp/frameGrabber-${USER}-snap-perspective-${n}.pam"
    basefile="baseLine_${n}.pam"

    # Do the test
    compare-binary ${newfile} ${basefile} >>  $LOG  2>&1
    Dif=$?
    # DEBUG echo Dif = ${Dif}

    # Check the result
    if [ ${Dif} -ne 0 ]
    then
            echo "${TC} base line ${i}: FAILED (${Dif})"
            echo "   Generated file differs from base line ${i}"
            echo "   Baseline: ${basefile}"
            echo "   Generated: ${newfile}"
            exitCode=1
    else
            echo "${TC} base line ${i}: PASSED"
    fi
done

# Return exit code
exit ${exitCode}
