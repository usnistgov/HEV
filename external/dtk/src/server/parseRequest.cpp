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
// All but server command requests are parsed here, exept for
// _DTKSREQ_CONNECTSERVERS which is parsed and processed in
// serviceSlaveTCP.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

#include "if_windows.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"


static char returnBuff[64];

static char *badrequest(size_t *size_out)
{
  snprintf(returnBuff,64,"%cbad DTK server request format",
	   _DTKRESP_BADREQUEST);
  *size_out = strlen(returnBuff) + 1;
  return returnBuff;
}


// returns 0 is there are num '\0' chars in s.
// returns 1 if not.
static int check_0chars(const char *s, int num, size_t size)
{
  size_t z = 0;
  for(int i=0;i<num;i++)
    {
      for(;*s && z<size ;s++)
	z++;
      s++;
      z++;
    }
  if(z > size || *(--s))
    return 1;
  return 0;
}

// prints commands in or out of the server.
void serverDebugPrintCommand(int isInput, const char *address, 
			    const char *buff, size_t size)
{
  if(!dtkMsg.isSeverity(DTKMSG_DEBUG)) return;
  if(isInput)
    dtkMsg.add(DTKMSG_DEBUG,"%s READ%s(size=%d) from %s%s%s (",
	       dtkMsg.color.rgrn, dtkMsg.color.end, size,
	       dtkMsg.color.yel, address,
	       dtkMsg.color.end);
  else
    dtkMsg.add(DTKMSG_DEBUG,"%sWROTE%s(size=%d)   to %s%s%s (",
	       dtkMsg.color.rvil, dtkMsg.color.end, size,
	       dtkMsg.color.yel, address,
	       dtkMsg.color.end);

  size_t i=0;
  for(;i<size;i++)
    {
      if(buff[i])
	dtkMsg.append("%c", buff[i]);
      else
	dtkMsg.append("\\0");
    }
  dtkMsg.append(")\n");
}


/////////////////////////////////////////////////////////////////////
// serverParseRequest() will parse the request and call the
// appropriate function or method to process the request.
// serverParseRequest() returns a response to the request.
// The returned value is always a single NULL terminated string.
////////////////////////////////////////////////////////////////////
char *serverParseRequest(const char *buff, size_t size,
			 size_t *size_out,
			 const char *fromAddrPort_in)
{
  if(*buff != _DTKREQ_WRITE)
    serverDebugPrintCommand(1,fromAddrPort_in,buff,size);

  /****************************************************************
   ***************** main server request switch *******************
   ****************************************************************/
  switch(*buff)
    {
      // Write to the DTK shared memory.
    case _DTKREQ_WRITE:
      {
	if(check_0chars(buff,2,size))
	  {
	    dtkMsg.add(DTKMSG_WARN, "got a bad shared memory"
		       " write request "
		       " from \"%s\" to an undetermined "
		       "shared memory file.\n",
		       fromAddrPort_in);
	    return NULL;
	  }
	char request_type = *buff;
	buff++;
	int byte_order = (*buff == _DTK_NET_BYTEORDER)?01:00;
	buff++;
	char *name = (char *) buff;
	buff += strlen(buff) + 1;
	size_t segSize = (size_t) strtoul(buff, NULL, 10);
	if(strlen(name) + strlen(buff) + (size_t) 4 + segSize !=
	   size)
	  {
	    dtkMsg.add(DTKMSG_WARN, "got bad shared memory write request "
		       " from \"%s\" to file \"%s\".",
		       fromAddrPort_in, name);
	    return NULL;
	  }

	buff += strlen(buff) + 1;

	// I've got to add code to this to tell the user if local
	// shared memory writing fails.
	return serverRespond.writeSharedMem(size_out, request_type,
					  (void *) buff,
					  segSize, name, byte_order,
					  fromAddrPort_in);
      }
      break;

    case _DTKREQ_LOADSERVICE:
      {
	// Check for bad request format
	// There must be three '\0' within size of buff
	if(check_0chars(buff,3,size))
	  return badrequest(size_out);
	char *file = (char *)++buff;
	char *name = (char *) (buff += strlen(file) + 1);
	char *arg = (char *) (buff += strlen(name) + 1);
	return serverRespond.loadService(size_out,file,name,arg);
      }
      break;

    case _DTKREQ_LOADCONFIG:
    {
      // Check for bad request format
      // There must be three '\0' within size of buff
      if(check_0chars(buff,1,size))
        return badrequest(size_out);
      char *config_file = (char *)++buff;
      char *path = (char*) (buff += strlen(config_file) + 1);
      char *cal_conf_path = strstr( path, "CALCONF=" );
      if( cal_conf_path )
      {
        *cal_conf_path = '\0';
        cal_conf_path += 8;
      }
      char *cal_path = strstr( path, "CAL=" );
      if( cal_path )
      {
        *cal_path = '\0';
        cal_path += 4;
      }
      char *svc_path = strstr( path, "SVC=" );
      if( svc_path )
      {
        *svc_path = '\0';
        svc_path +=4;
      }
      char *cwd = strstr( path, "CWD=" );
      if( cwd )
      {
        *cwd = '\0';
        cwd += 4;
      }

      return serverRespond.loadConfig(size_out,config_file,path,
          cwd,svc_path,cal_path,cal_conf_path);
    }
    break;

    case _DTKREQ_UNLOADSERVICE:
      {
	// Check for bad request format
	// There must be one '\0' within size of buff
	if(check_0chars(buff,1,size))
	  return badrequest(size_out);
	char *name = (char *) ++buff;
	return serverRespond.unloadService(size_out,name);
      }
      break;

    case _DTKREQ_RESETSERVICE:
      {
	// Check for bad request format
	// There must be one '\0' within size of buff
	if(check_0chars(buff,1,size))
	  return badrequest(size_out);
	char *name = (char *) ++buff;
	return serverRespond.resetService(size_out,name);
      }
      break;

    case _DTKREQ_CHECKSERVICE:
      {
	// Check for bad request format
	// There must be one '\0' within size of buff
	if(check_0chars(buff,1,size))
	  return badrequest(size_out);
	char *name = (char *) ++buff;
	return serverRespond.checkService(size_out,name);
      }
      break;

    case _DTKREQ_SHUTDOWN:
      {
	if(size != (size_t) 1)
	  return badrequest(size_out);
	returnBuff[0] = _DTKRESP_SUCCESS;
	returnBuff[1] = '\0';
	*size_out = 2;
	// tell the server to exit.
	isRunning.set(0);
#ifdef MULTI_THREADED
	// wake up the main thread.
	isRunning.signal();
#endif
	return returnBuff;
      }
      break;

      //#define _DTKREQ_CONNECTREMOTESEG 'c'
      //senddata={'c',"localName","remoteAddr:Port","remoteName"}
      //The server will send a _DTKSREQ_CONNECTREMOTESEG to the
      //remoteAddr:Port server.  connect remote shared mem: client
      //request to server.
    case _DTKREQ_CONNECTREMOTESEG:
      {
	if(check_0chars(buff,3,size))
	  return badrequest(size_out);
	char *name = (char *) ++buff;
	char *addressPort = (char *) (buff += strlen(buff) + 1);
	char *remoteName = (char *) (buff + strlen(buff) + 1);
	return serverRespond.connectSharedMem(size_out,
					    name,
					    addressPort,
					    remoteName,
					    fromAddrPort_in);
      }
      break;

      //#define _DTKSREQ_CONNECTREMOTESEG         's'
      // This is a server to server request for making remote
      // shared memory.
      // senddata={'s',"localName","remoteName","sizeBase10",
      // "clientRequestFrom"}
      // "clientRequestFrom" is sent to and from in order to know where to
      // put the responce status back to.
      // the response be a _DTKSSREQ_CONNECTREMOTESEG sent to the other server.
      
      // connect remote shared mem: server request to server.
      // In response to _DTKREQ_CONNECTREMOTESEG.

      // This IS THE REMOTE SERVER
    case _DTKSREQ_CONNECTREMOTESEG:
      {
	if(check_0chars(buff,4,size))
	  return badrequest(size_out);
	char *name = (char *) ++buff;
	char *remoteName = (char *) (buff += strlen(buff) + 1);
	size_t size = (size_t) strtoul(buff += strlen(buff) + 1, NULL, 10);
	char *fromClientAddrPort = (char *) (buff + strlen(buff) + 1);
	return serverRespond.connectSharedMem(size_out,
					    size,
					    name,
					    fromAddrPort_in,
					    remoteName,
					    fromClientAddrPort);
      }
      break;

      //#define _DTKSSREQ_CONNECTREMOTESEG        'O'
      // This is a server to server request that
      // is a response from _DTKSREQ_CONNECTREMOTESEG
      // senddata={'s',"localName",
      //               "remoteName","clientRequestFrom","errorString"}
      // This is the final in the _DTK*REQ_CONNECTREMOTESEG series of
      // command requests.  All this stuff is needed to be sent back because
      // the servers do not keep a record of commands that are not fully
      // processed.

      // connect remote shared mem: server request (response) to
      // server.  In response to _DTKSREQ_CONNECTREMOTESEG.
    case _DTKSSREQ_CONNECTREMOTESEG:
      {
	if(check_0chars(buff,4,size))
	  return badrequest(size_out);
	char *name = (char *) ++buff;
	char *remoteName = (char *) (buff += strlen(buff) + 1);
	char *fromClientAddrPort = (char *) (buff += strlen(buff) + 1);
	char *errorString = (char *) (buff + strlen(buff) + 1);
	return serverRespond.connectSharedMem(size_out,
					    name,
					    fromAddrPort_in,
					    remoteName,
					    fromClientAddrPort,
					    errorString);
      }
      break;

    case _DTKREQ_CONNECTSERVERS:
      {
	if(check_0chars(buff,1,size))
	  return badrequest(size_out);
	char *serverAddrPort = (char *) ++buff;
	return serverRespond.connectServer(size_out, serverAddrPort, fromAddrPort_in);
      }
      break;

      // This is a failed request that should have been answered in a
      // serviceSlaveTCP object, but it's here because it failed.
    case _DTKSREQ_CONNECTSERVERS:
      {
	if(check_0chars(buff,2,size))
	  return badrequest(size_out);
	char *serverAddrPort = (char *) ++buff;
	char *fromAddrPort = (char *) (buff + strlen(buff) + 1);
	return serverRespond.connectServerFail(size_out, serverAddrPort,
					       fromAddrPort);
      }
      break;

    case _DTKSSREQ_CONNECTSERVERS:
      {
	if(check_0chars(buff,1,size))
	  return badrequest(size_out);
	char *serverAddrPort = (char *) ++buff;
	char *fromAddrPort = (char *) (buff += strlen(buff) + 1);
	char *errorString = (char *) (buff + strlen(buff) + 1);
	return serverRespond.connectServer(size_out, serverAddrPort,
					   fromAddrPort, errorString);
      }
      break;

    case _DTKREQ_SEGWRITELIST:
      {
	if(check_0chars(buff,1,size))
	  return badrequest(size_out);
	char *name = (char *) ++buff;
	return serverRespond.sharedMemWriteList(size_out, name);
      }
      break;


    case _DTKRESP_BADREQUEST:
    case _DTKRESP_FAILED:
    case _DTKRESP_SUCCESS:
      *size_out = 0;
      return NULL;
      break;

    default:
      return badrequest(size_out);
    }

  return NULL;
}

