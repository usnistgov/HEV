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

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <process.h>
#else
# include <unistd.h>
# include <sys/time.h>
#endif

#include "if_windows.h"
#include "serviceClientTCP.h"
#include "serviceSlaveTCP.h"
#include "serviceInetTCP.h"
#include "serviceInetUDP.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"

#include "parse_args.h"
#include "threadManager.h"


#ifndef DTK_ARCH_WIN32_VCPP
// from daemonize.C
extern int daemonize(int timeout);
extern void signalDaemonStarted(int timeout, int success=1);
#endif

// __this_is_the_dtk_server__ is declared in
// ../lib/dtk/sharedMem_network.cpp It's so that the code in the
// libdtk can tell if the dtk-server is using libdtk
extern DTKAPI int __this_is_the_dtk_server__;

// dtkSharedMem_tcpList is from ../lib/dtk/sharedMem_network.cpp see
// more comments there.  The dtkSharedMem ojects share the TCP sockets
// with dtk-server.
extern DTKAPI dtkSocketList *dtkSharedMem_tcpList;


/*************************************************************/
/*************************************************************/

static ServiceInetTCP *serviceInetTCP = NULL;
static ServiceInetUDP *serviceInetUDP = NULL;


// from signal.cpp
extern void signal_catcher(int sig);
// from signal.cpp
extern void (*signal_intr(int signo, void (*func)(int)))(int);


static int loadDefaultServices(void)
{
  // load default server built-in services.

  serverPort = getenv("DTK_PORT");
  if(!serverPort || strcasecmp("none",serverPort))
  {
    if(!serverPort)
      serverPort = dtk_strdup(DTK_DEFAULT_SERVER_PORT);
    else
      serverPort = dtk_strdup(serverPort);

    serviceInetTCP = new ServiceInetTCP(serverPort);
    if(serviceManager.add(serviceInetTCP, "serviceInetTCP"))
      return -1; // error

    serviceInetUDP = new ServiceInetUDP(serverPort);
    if(serviceManager.add(serviceInetUDP,"serviceInetUDP"))
      return -1; // error
  }
  else // No IP networking for this server.
    serverPort = NULL;

  return 0;
}


int main(int argc, char **argv, char **env)
{
  // set the DTK shared memory connected TCP sockets list to share the
  // list in this server code. from dtkSharedMem_tcpList is from
  // ../lib/dtk/sharedMem_network.cpp
  __this_is_the_dtk_server__ = 1;
  dtkSharedMem_tcpList = &serverSocketList;

  int isDaemon = 0, timeout = DEFAULT_STARTUP_TIMEOUT;

  if(parse_nonLoading_args(argc, argv, &isDaemon, &timeout))
    return 1; // server not started

  dtkMsg.setPreMessage("%sdtk-server%s",
		       dtkMsg.color.tur,dtkMsg.color.end);

#if defined MULTI_THREADED && defined DTK_ARCH_LINUX
  // There is a bug in my version of glibc that has trouble forking
  // and then calling pthread_create().
  if(isDaemon)
    dtkMsg.add(DTKMSG_WARN, "There may be a bug in glibc that may "
	       "cause this server to not run correctly.\n");
#endif


#ifndef DTK_ARCH_WIN32_VCPP
  if(isDaemon && daemonize(timeout)) return 1; // error not started
#endif


  if(loadDefaultServices() ||
     parse_loading_args(argc, argv, &serviceManager))
    {
# ifndef DTK_ARCH_WIN32_VCPP
      if(isDaemon) // Tell parent that this failed to start.
	signalDaemonStarted(timeout,0);
# endif
      dtkMsg.add(DTKMSG_ERROR, "server startup failed.\n");
      isRunning.set(0); // error // need to cleanup.
    }
# ifndef DTK_ARCH_WIN32_VCPP
  else if(isDaemon) // Tell parent that this started.
    // This will cause the forground process to
    // return 0 for success.
    signalDaemonStarted(timeout);
# endif


  if(isRunning.get())
    {
      signal_intr(SIGINT, signal_catcher);
#ifdef DTK_ARCH_WIN32_VCPP
      // signal_intr(SIGHUP, signal_catcher);
#else
      signal_intr(SIGQUIT, signal_catcher);
#endif
      signal_intr(SIGTERM, signal_catcher);
    }

  if(isRunning.get())
    dtkMsg.add(DTKMSG_DEBUG,
#ifdef MULTI_THREADED
	       "MULTI THREADED server "
#endif
	       "pid=%ld entering main loop.\n",
	       getpid());


  while(isRunning.get())
    {

#ifdef MULTI_THREADED
      /*******************************************************
       ****************** MULTI_THREADED SERVER **************
       *******************************************************/

      // look for returning (joining) threads
      dtkService *service = threadManager.getJoiningThread();
      while(service)
	{
	  serviceManager.remove(service);
	  service = threadManager.getJoiningThread();
	}
      isRunning.wait(4);

#else /* #ifdef MULTI_THREADED */
      /*******************************************************
       *************** NOT MULTI_THREADED SERVER *************
       *******************************************************/

      // Load the current file describers
      fd_set rfds;
      if(serviceManager.loadFileDescriptors(&rfds))
	break;

      // select() is where the server blocks waiting for requests.
      if(select(serviceManager.maxFDPlusOne,
		&rfds, NULL, NULL, NULL) > 0)
	{
	  dtkService *service;
	  while(isRunning.get() && 
		(service = serviceManager.getNextService(&rfds)))
	    {
	      // This, serve(), is where it all happens.
	      int i = service->serve();
	      if(i)
		{
		  if(i == DTKSERVICE_ERROR)
		    {
		      dtkMsg.add(DTKMSG_ERROR,
				 "serving service \"%s\" failed.\n",
				 serviceManager.getName(service));
		      break; // Bail due to serve error.
		    }
		      // else i == DTKSERVICE_UNLOAD
		  
		  serviceManager.remove(service);
		}
	    }
	}
      else if(errno == EINTR) errno = 0; // select() was interrupted
      else dtkMsg.add(DTKMSG_WARN, 1,
		      "select() failed.\n");

#endif /* #else #ifdef MULTI_THREADED */

    }

  // After main loop, clean up and exit.

  // Remove services.
  serviceManager.removeAll();

  if(serviceInetUDP)
    delete serviceInetUDP;

  if(serviceInetTCP)
    delete serviceInetTCP;

  dtkMsg.add(DTKMSG_DEBUG,
	     "server pid=%ld exiting now.\n",
	     getpid());

#ifdef DTK_ARCH_WIN32_VCPP
  usleep(100000);
#endif

  return 0;
}
