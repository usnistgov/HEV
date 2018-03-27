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
#include "dtkDSO.h"
#include "_dtkService.h"
#include "_dtkConfigService.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtkService.h"

dtkConfigService::dtkConfigService( const char* configfile )
{
  m_calibrator = NULL;
  m_calibratorLoaderFunc = NULL;
}

dtkConfigService::~dtkConfigService()
{
  if( m_calibrator )
  {
    int (*calibrationUnloaderFunc)(dtkCalibration *m_calibrator) =
        (int (*)(dtkCalibration *)) dtkDSO_getUnloader( m_calibratorLoaderFunc );

    if( !calibrationUnloaderFunc )
    {
      dtkMsg.add(DTKMSG_WARN,
          "dtkConfigService::~dtkConfigService(): unloading dtkCalibration\n"
          "DSO file failed to get unloader function.\n");
    }
    else if( calibrationUnloaderFunc( m_calibrator ) == DTKDSO_UNLOAD_ERROR )
    {
      dtkMsg.add(DTKMSG_WARN,
          "dtkConfigService::~dtkConfigService(): unloading dtkCalibration\n"
          "DSO file failed: unloader function failed.\n");
    }
    dtkDSO_unload( m_calibratorLoaderFunc );
  }
}

