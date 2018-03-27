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
#ifdef MULTI_THREADED
# ifdef DTK_ARCH_WIN32_VCPP
#  include <winbase.h>
# else
#  include <pthread.h>
# endif
#endif

#define DEFAULT_STARTUP_TIMEOUT   5
#define MIN_STARTUP_TIMEOUT       1
#define MAX_STARTUP_TIMEOUT       300

// This class IsRunning is just some that I have a way to signal if
// the server is running or not.  This will be thread safe if
// MULTI_THREADED is defined. See source in condition.cpp.

class DTKSERVERAPI Condition
{
 public:

  Condition(void);
#ifdef DTK_ARCH_WIN32_VCPP
  ~Condition(void);
#endif

  void set(int val);
  int get(void);

#ifdef MULTI_THREADED
  void signal(void);
  void wait(unsigned int msecs=(unsigned int)1000);
#endif

 private:

  ServerMutex mutex;

#ifdef MULTI_THREADED
# ifdef DTK_ARCH_WIN32_VCPP
  HANDLE semaphore;
# else
  pthread_mutex_t waitMutex;
  pthread_cond_t  waitCondition;
# endif
#endif

  int val;
};




// from get_dso_path.C
extern DTKSERVERAPI char *get_service_path(void);
extern DTKSERVERAPI char *get_service_config_path(void);
extern DTKSERVERAPI char *get_calibration_path(void);
extern DTKSERVERAPI char *get_calibration_config_path(void);

extern DTKSERVERAPI dtkSocketList serverSocketList;
extern DTKSERVERAPI ServerRespond serverRespond;
extern DTKSERVERAPI dtkSocketList serverSocketList;
// isRunning is decleared in globels.cpp.
extern DTKSERVERAPI Condition isRunning;


// Used by DTK server's built-in socket services for calling server
// functions serverParseRequest(), serverDebugPrintCommand(), and
// methods in the serverRespond object in a thread safe way.
extern DTKSERVERAPI ServerMutex serverMutex;

extern DTKSERVERAPI ServerMutex socketListMutex;


#ifdef MULTI_THREADED
extern DTKSERVERAPI pthread_t mainThread;
#endif


// This buffer size is used in the built-in sockets services
// serviceClientTCP.cpp, serviceInetUDP.cpp, and serviceSlaveTCP.cpp
// for the size of the read buffer.
#define DTKSERVER_BUFFER_SIZE   ((size_t) 4096)


// serverParseRequest() will parse the request and call the
// appropriate function or method to process the request.
// serverParseRequest() returns a response to the request.
extern DTKSERVERAPI char *serverParseRequest(const char *request, size_t size,
				size_t *size_out,
				const char *fromAddrPort="");

// debugging command printer.
extern DTKSERVERAPI void
serverDebugPrintCommand(int isInput,         /* 1 for input, 0 for output */
			const char *address, /* to or from address */
			const char *buff,    /* what */
			size_t size          /* size of what */
			);

