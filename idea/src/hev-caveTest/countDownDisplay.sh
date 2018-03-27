#! /bin/bash

echo nodemask tube  on > $IRIS_CONTROL_FIFO
echo nodemask tube0 on  > $IRIS_CONTROL_FIFO
sleep 2

# fade right to left
echo nodemask t5 on > $IRIS_CONTROL_FIFO
play -q 05_five.wav  remix 1v0.0 1v1.0 &
usleep 500000
echo nodemask t5 off > $IRIS_CONTROL_FIFO

echo nodemask t4 on > $IRIS_CONTROL_FIFO
play -q 04_four.wav  remix 1v0.2 1v0.8 &
usleep 500000
echo nodemask t4 off > $IRIS_CONTROL_FIFO

echo nodemask t3 on > $IRIS_CONTROL_FIFO
play -q 03_three.wav remix 1v0.4 1v0.6 &
usleep 500000
echo nodemask t3 off > $IRIS_CONTROL_FIFO

echo nodemask t2 on > $IRIS_CONTROL_FIFO
play -q 02_two.wav   remix 1v0.6 1v0.4 &
usleep 500000
echo nodemask t2 off > $IRIS_CONTROL_FIFO

echo nodemask t1 on > $IRIS_CONTROL_FIFO
play -q 01_one.wav   remix 1v0.8 1v0.2 &
usleep 500000
echo nodemask t1 off > $IRIS_CONTROL_FIFO

echo nodemask t0 on > $IRIS_CONTROL_FIFO
play -q 00_zero.wav  remix 1v1.0 1v0.0


sleep 1
echo nodemask t0 off > $IRIS_CONTROL_FIFO
echo nodemask tube  off  > $IRIS_CONTROL_FIFO
echo nodemask tube0 off  > $IRIS_CONTROL_FIFO
