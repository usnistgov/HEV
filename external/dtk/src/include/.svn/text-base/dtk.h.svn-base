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
 */

/** \file dtk.h
 * 
 * This is the main DTK header file.  It includes most of the header
 * files for the DTK C++ API.  Some template classes are not declared
 * from here.
 */

#ifndef __dtk_h__
#define __dtk_h__

#if defined _WIN32 && !defined(DTK_ARCH_CYGWIN)/* the MS Visual C++ compiler defines _WIN32 */
#  ifndef DTK_ARCH_WIN32_VCPP
#    define DTK_ARCH_WIN32_VCPP
#  endif
#endif


#ifdef DTK_ARCH_WIN32_VCPP /* MS windows port */
/******************************************
 *         On MS Windows
 ******************************************/

#include "dtk/_config.h"
#include <winsock2.h>
#include <winbase.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <limits.h>
#include <dtk/windowsdefs.h>

#include <stdio.h>

#include <dtk/dtkConfigure.h>

#include <dtk/serverProtocol.h>

#include <dtk/dtkColor.h>
#include <dtk/dtkMessage.h>
#include <dtk/types.h>
#include <dtk/dtkBase.h>
#include <dtk/dtkFilter.h>
#include <dtk/filterList.h>
#include <dtk/utils.h>
#include <dtk/dtkRWLock.h>
#include <dtk/dtkConditional.h>

//#include <dtk/dtkRealtime.h>

#include <dtk/dtkTime.h>
#include <dtk/dtkDistribution.h>
#include <dtk/dtkVec3.h>
#include <dtk/dtkCoord.h>
#include <dtk/dtkMatrix.h>
#include <dtk/dtkSocket.h>
#include <dtk/dtkUDPSocket.h>
#include <dtk/dtkTCPSocket.h>
#include <dtk/dtkDSO.h>
#include <dtk/dtkFLTKOptions.h>
#include <dtk/dtkVRCOSharedMem.h>
#include <dtk/dtkReadBuffer.h>
#include <dtk/dtkSocketList.h>
#include <dtk/dtkSegAddrList.h>
#include <dtk/dtkSharedMem.h>
#include <dtk/dtkSharedTime.h>

#include <dtk/dtkAugment.h>
#include <dtk/dtkDisplay.h>
#include <dtk/dtkRecord.h>
#include <dtk/dtkInput.h>
#include <dtk/dtkDequeuer.h>
#include <dtk/dtkInButton.h>
#include <dtk/dtkInValuator.h>
#include <dtk/dtkInSelector.h>
#include <dtk/dtkInLocator.h>

#include <dtk/dtkManager.h>
#include <dtk/dtkBasicDisplay.h>
#include <dtk/dtkInCave.h>

#include <dtk/dtkNav.h>
#include <dtk/dtkNavList.h>

#include <dtk/_dtkService.h>
#include <dtk/_dtkConfigService.h>
#include <dtk/_dtkCalibration.h>
#include <dtk/dtkClient.h>


#else /* not #ifdef DTK_ARCH_WIN32_VCPP   UNIX's */
/******************************************
 *         On UNIX's
 ******************************************/


#include <dtk/_config.h>

#include <sys/types.h>
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
/* for PF_UNIX and PF_INET */
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <float.h>

#ifdef DTK_ARCH_IRIX
   typedef uint64_t u_int64_t;
#endif

#include <dtk/dtkConfigure.h>

#include <dtk/serverProtocol.h>

#include <dtk/dtkColor.h>
#include <dtk/dtkMessage.h>
#include <dtk/types.h>
#include <dtk/dtkBase.h>
#include <dtk/dtkFilter.h>
#include <dtk/filterList.h>
#include <dtk/utils.h>
#include <dtk/dtkRWLock.h>
#include <dtk/dtkConditional.h>
#include <dtk/dtkRealtime.h>
#include <dtk/dtkTime.h>
#include <dtk/dtkDistribution.h>
#include <dtk/dtkVec3.h>
#include <dtk/dtkCoord.h>
#include <dtk/dtkMatrix.h>
#include <dtk/dtkSocket.h>
#include <dtk/dtkUDPSocket.h>
#include <dtk/dtkTCPSocket.h>
#include <dtk/dtkDSO.h>
#include <dtk/dtkFLTKOptions.h>
#include <dtk/dtkVRCOSharedMem.h>
#include <dtk/dtkReadBuffer.h>
#include <dtk/dtkSocketList.h>
#include <dtk/dtkSegAddrList.h>
#include <dtk/dtkSharedMem.h>
#include <dtk/dtkSharedTime.h>

#include <dtk/dtkAugment.h>
#include <dtk/dtkDisplay.h>
#include <dtk/dtkRecord.h>
#include <dtk/dtkInput.h>
#include <dtk/dtkDequeuer.h>
#include <dtk/dtkInButton.h>
#include <dtk/dtkInValuator.h>
#include <dtk/dtkInSelector.h>
#include <dtk/dtkInLocator.h>

#include <dtk/dtkManager.h>
#include <dtk/dtkBasicDisplay.h>
#include <dtk/dtkInCave.h>

#include <dtk/dtkNav.h>
#include <dtk/dtkNavList.h>

#include <dtk/_dtkService.h>
#include <dtk/_dtkConfigService.h>
#include <dtk/_dtkCalibration.h>
#include <dtk/dtkClient.h>

#ifdef DTK_ARCH_LINUX
#  include <dtk/dtkUSBInput.h>
#endif

/****** template classes that are not included here *******/
//#include <dtk/dtkDiffEq.h>
//#include <dtk/dtkRK4.h>

#endif /* #ifdef DTK_ARCH_WIN32_VCPP */

#endif /* #ifndef __dtk_h__ */
