#! /bin/bash

# Kill background children processes on exit
trap 'kill $(jobs -p)' 0

dtk-gnomonDisplay head &
dtk-gnomonDisplay wand &

wait

