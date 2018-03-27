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
# Find an editor
###############################

for i in $VISUAL $EDITOR
do
  if which $i 1> /dev/null
  then
    editor=$i
    break
  fi
done

if [ "$editor" = "" ]
then
  if which emacs 1> /dev/null
  then
    editor=emacs
  fi
fi

if [ "$editor" = "" ]
then
  for i in gvim vim vi nedit xedit joe
  do
    if which $i 1> /dev/null
    then
      editor="$i"
      break
    fi
  done
fi

if [ "$editor" = "" ]
then
# You have no editor so you're screwed.
  editor=ed
fi


###############################
# Find a file
###############################
file=$1

if [ "$file" = "" ]
then
  file=config.h
fi

# Save the original if it has not
# been save already.
if [ ! -f ${file}.org ]
then
  cp ${file} ${file}.org
fi


###############################
# run the editor
###############################

if [ "$editor" = emacs ] || [ "$editor" = xedit ]
then
    $editor -title "$editor $file in `pwd`" $file
    exit
fi

if [ "$editor" = vi ] \
|| [ "$editor" = vim ] \
|| [ "$editor" = joe ]
then
    xterm -rv -title "$editor $file in `pwd`" -e $editor $file
    exit
fi

$editor $file
