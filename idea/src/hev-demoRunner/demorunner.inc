
function cleanup {
    jk=$(jobs -p)
    if [ ! -z $jk ]
    then
	echo killing $jk
	kill $jk
    fi

    if [ ! -z $IRIS_CONTROL_FIFO ]
    then
	echo Terminating iris-viewer
	echo TERMINATE > $IRIS_CONTROL_FIFO
    fi
}

trap 'cleanup' 0

