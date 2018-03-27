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


# This is used to start and stop the dtk-server for the IS900 in the
# CAVE at Virginia Tech.


if [ "${DTK_SPEW}" = "" ]
then
  export DTK_SPEW=NOTICE
fi

function Usage {
  echo "Usage: $0 {start|stop|reset}"
  exit 1
}


if [ "$1" = "" ]
then
  Usage
fi


case "$1" in
  'start')
      ${_DTK_INSTALL_BIN}dtk-server --verbose --daemon is900
      exit $?
   ;;
  'reset')
      ${_DTK_INSTALL_BIN}dtk-unloadService is900
      ${_DTK_INSTALL_BIN}dtk-loadService is900
      exit $?
   ;;
  'stop')
     ${_DTK_INSTALL_BIN}dtk-shutdownServer
     exit $?
   ;;
esac

Usage
