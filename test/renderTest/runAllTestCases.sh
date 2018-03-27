#! /bin/bash

# runAllTestCases.sh
# May 21, 2015
# Steve Satterfield

topdir=$(pwd)

for d in testCase_*
do
    if [ -d $d ]
    then
	cd $d
	if [ -x runTestCase.sh ]
	then
	    echo "Running test case: $d"
	    ./runTestCase.sh
	    if [ $? -eq 0 ]
	    then
		echo "Results $d: PASSED"
	    else
                echo "Results $d: FAILED"
	    fi
        fi
    fi
    echo
    cd $topdir
done


    
