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


###############################
# find the Pager
###############################
pager="$PAGER"

if [ "$pager" = "" ]
then
  for i in less view more rvim vim vi
  do
    if which $i 1> /dev/null
    then
      pager=$i
      break
    fi
  done
fi
###############################
# Find a file
###############################
file=$1

if [ "$file" = "" ]
then
  file=README
fi



if [ $pager = less ] \
|| [ $pager = more ] \
|| [ $pager = vim ] \
|| [ $pager = vi ] \
|| [ $pager = view ]
then
  echo "trying to view file `pwd`/$file"
  xterm -rv +sb -title "$pager $file in `pwd`" -e $pager $file
  exit
fi

echo "Running in `pwd`: $pager $file"

if $pager $file
then
  exit
fi

# if all else fails
cat $file
