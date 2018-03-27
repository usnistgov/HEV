#! /bin/bash

echo compiling hev-triangulation.cpp
/usr/bin/c++ -c  -frounding-math  -o hev-triangulation.o hev-triangulation.cpp   2>&1 | grep -v -e deprecated -e "hev-triangulation.h:13," -e "from hev-triangulation.cpp:1:" -e "traits_xy_3.h:26,"
echo compiling hev-triangulation.cpp done

