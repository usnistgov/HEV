#!/bin/sh

remote=$1
shift

shm=idea/hev-collabButton

# create and zero out the shared memory
hev-writeInts -c 2 $shm 0 0

# start a dtk server if one isn't already running
if ( ! dtk-pingServer >/dev/null) ; then
    serverStarted=yup
    dtk-server -d
    dtk-msgNotice starting DTK server
fi

# cleanup on exit
trap "if [ -n \"$serverStarted\" ] ; \
      then dtk-msgNotice shutting down server ; \
           dtk-shutdownServer ; \
           dtk-destroySharedMem $shm ; \
      fi" EXIT

# try to connect to the remote server
dtk-msgNotice trying to connect to DTK server $remote
while ( ! dtk-connectServer $remote )
do sleep 2
    dtk-msgNotice trying again to connect to DTK server $remote
done

while (! dtk-connectRemoteSharedMem $shm $remote )
do sleep 2
    dtk-msgNotice trying again to connect shared memory files
done
dtk-msgNotice connected local shared memory $shm to remote shared memory

$*
