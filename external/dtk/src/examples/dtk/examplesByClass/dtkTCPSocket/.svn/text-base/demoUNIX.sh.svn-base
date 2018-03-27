#!/bin/sh

# The DIVERSE Toolkit
# Copyright (C) 2000 - 2003  Virginia Tech
# 
# This is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License (GPL) as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
# 
# This software is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software, in the top level source directory in a file
# named "COPYING.GPL"; if not, see it at:
# http://www.gnu.org/copyleft/gpl.html or write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.



if  ! dtk-config --test
then
  echo "\`dtk-config' is not in your PATH."
  echo "Find it and put it in your PATH and"
  echo "run \`$0' again."
  exit 1
fi

if ! make
then
  echo "building failed."
  exit 1
fi


font=-adobe-courier-bold-*-*--14-*-*-*-*-*-*-*
delta=210
y=50

for run in "./server unix foo" "./client unix foo"
do
  xterm -rv -font $font  -T "$run" -n "$run" -geometry 100x12+0+$y -e $run &
  let y=$y+$delta
  echo "starting: $run"
  sleep 1
done


echo
echo "control-c (kill -INT) on each window to quit"
echo
