#! /bin/bash

# fade right to left
play -q 05_five.wav  remix 1v0.0 1v1.0 &
usleep 500000
play -q 04_four.wav  remix 1v0.2 1v0.8 &
usleep 500000
play -q 03_three.wav remix 1v0.4 1v0.6 &
usleep 500000
play -q 02_two.wav   remix 1v0.6 1v0.4 &
usleep 500000
play -q 01_one.wav   remix 1v0.8 1v0.2 &
usleep 500000
play -q 00_zero.wav  remix 1v1.0 1v0.0


