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
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <winsock2.h> // for struct timeval
#else
# include <sys/un.h>
# include <netinet/in.h>
# include <sys/time.h>
#endif

#ifdef DTK_ARCH_IRIX
  typedef uint64_t u_int64_t;
#endif

#if defined DTK_ARCH_IRIX || defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
#  include <sys/socket.h>
#endif


#include "types.h"
#include "dtkBase.h"
#include "dtkFilter.h"
#include "filterList.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkRWLock.h"
#include "dtkConditional.h"
#include "dtkSegAddrList.h"
#include "dtkSocket.h"
#include "dtkSocketList.h"
#include "dtkUDPSocket.h"
#include "dtkSharedMem.h"
#include "_dtkService.h"

int dtkService::init(void)
{
  return DTKSERVICE_CONTINUE;
}

int dtkService::reset(void)
{
  dtkMsg.add(DTKMSG_INFO, "service reset called for service with"
	     " file descriptor %s%d%s.\n", dtkMsg.color.tur,
	     fd, dtkMsg.color.end);
  return 0; // successful reset by default
}

char *dtkService::version(void)
{
  return const_cast<char*>(DTK_VERSION);
}

dtkService::dtkService( const char* arg )
{
#ifdef DTK_ARCH_WIN32_VCPP
  fd = NULL;
#else
  fd = -1;
#endif

  if( arg )
  {
    m_arg = new char[ strlen( arg ) + 1 ];
    strcpy( m_arg, arg );
  }
  else
    m_arg = 0;
}

dtkService::~dtkService(void)
{
#ifdef DTK_ARCH_WIN32_VCPP
  fd = NULL;
#else
  fd = -1;
#endif

  if( m_arg )
    delete [] m_arg;
}

void dtkService::print(FILE *file)
{
}

#ifdef DTK_ARCH_WIN32_VCPP
void dtkService::stop(void)
{
}
#endif
