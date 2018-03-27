/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
#include <dtk.h>

#include "if_windows.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"


/*************************************************************/
/****       stuff declared extern in dtk/dtkService.h     ********/
/*************************************************************/

ServiceManager serviceManager;
char *serverPort = NULL;

// keeps a list of TCP connected sockets
dtkSocketList serverSocketList;


/*************************************************************/
/****          stuff declared extern in dtk-server.h      ****/
/*************************************************************/

// object to respond to server requests.
ServerRespond serverRespond;

// The main while() loop thingy-erator.  It's a class so that it can
// be threadSafe or not, depending whither or not MULTI_THREADED is
// defined or not.
Condition isRunning;

// extern in dtk-server.h
ServerMutex socketListMutex;

#ifdef MULTI_THREADED
pthread_t mainThread = pthread_self();
#endif
