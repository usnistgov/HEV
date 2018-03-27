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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dtk.h>


#include "if_windows.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"
#include "serviceClientTCP.h"


ServerRespond::ServerRespond(void)
{
  shmList = NULL;
  response1 = &(response[1]);
}


ServerRespond::~ServerRespond(void)
{
  // Free the DTK shared memory list.  The dtkShareMem
  // serverShareMem will cleanup dtkSharedMem objects.
  while(shmList)
    {
      struct sharedMemList *l = shmList;
      shmList = shmList->next;
      delete l->sharedMem;
      free(l);
    }
}


int ServerRespond::removeAddressFromWroteLists(const char *addr)
{
  removeAddressFromWroteListsMutex.lock();
  struct sharedMemList *l = shmList;
  for(;l;l=l->next)
    {
      dtkSegAddrList *addrList =  l->sharedMem->getSegAddrList();
      if(addrList && addrList->get(addr))
	l->sharedMem->removeFromWriteList(addr);
    }
  removeAddressFromWroteListsMutex.unlock();
  return 0;
} 


// This class uses this to keep a list of dtkSharedMem objects and not
// make new ones.
dtkSharedMem *ServerRespond::getSharedMem(size_t size, const char *name)
{
  struct sharedMemList *prev=NULL, *l = shmList;
  for(;l;l=l->next)
    {
      if(!strcmp(l->sharedMem->getName(),
		 dtkSharedMem_getFullPathName(name)))
	{
	  if(l->sharedMem->getSize() == size)
	    return l->sharedMem;
	  else
	    return NULL; // wrong size
	}
      prev = l;
    }

  dtkSharedMem *shm = new dtkSharedMem(size, name);
  if(!shm || shm->isInvalid()) return NULL;

  // Add the dtkSharedMem to the end of the list.
  l = (struct sharedMemList *) dtk_malloc(sizeof(struct sharedMemList));
  l->next = NULL;
  l->sharedMem = shm;

  if(prev) prev->next = l;
  else shmList = l;

  return shm;
}

// This class uses this to keep a list of dtkSharedMem objects and not
// make new ones.
dtkSharedMem *ServerRespond::getSharedMem(const char *name)
{
  struct sharedMemList *prev=NULL, *l = shmList;
  for(;l;l=l->next)
    {
      if(!strcmp(l->sharedMem->getName(),
		 dtkSharedMem_getFullPathName(name)))
	{
	  return l->sharedMem;
	}
      prev = l;
    }

  dtkSharedMem *shm = new dtkSharedMem(name);
  if(!shm || shm->isInvalid()) return NULL;

  // Add the dtkSharedMem to the end of the list.
  l = (struct sharedMemList *) dtk_malloc(sizeof(struct sharedMemList));
  l->next = NULL;
  l->sharedMem = shm;

  if(prev) prev->next = l;
  else shmList = l;

  return shm;
}

char *ServerRespond::fail(size_t *size, const char *s, ...)
{
  if(s)
    {
      va_list ap;
      va_start(ap,s);
      vsnprintf(response1, RESPONSE_BUF_SIZE, s, ap);
      va_end(ap);
    }
  else
    *response1 = '\0';

  response[0] = _DTKRESP_FAILED;
  *size = strlen(response) + 1;
  return response;
}

char *ServerRespond::
success(size_t *size, const char *s, ...)
{
  if(s)
    {
      va_list ap;
      va_start(ap,s);
      vsnprintf(response1, RESPONSE_BUF_SIZE, s, ap);
      va_end(ap);
    }
  else
    *response1 = '\0';

  response[0] = _DTKRESP_SUCCESS;
  *size = strlen(response) + 1;
  return response;
}


// writing to the a shared memory from the network.
char *ServerRespond::
writeSharedMem(size_t *size_out, char request_type, void *data, size_t size,
	     const char *name, int byte_order, const char *fromServerAddr)
{
  dtkSharedMem *shm = getSharedMem(size, name);
  if(shm && !(shm->writeLocal(data, byte_order)))
    {
      return NULL;
    }

  // ADD REMOVE sharedMem connection return command.
  // MUST add the writers server address:port to the protocol.

  if(!shm)
    dtkMsg.add(DTKMSG_WARN,"request to write shared memory \"%s\""
	       " with size=%d failed: sharedMem not found.\n",
	       name, size);
  else
    dtkMsg.add(DTKMSG_WARN,"request to write shared memory \"%s\""
	       " with size=%d failed: local write failed.\n",
	       name, size);

  return NULL;
}

char *ServerRespond::
loadService(size_t *size_out, const char *file,
	    const char *name, const char *arg)
{
  if(!file || !(file[0]))
    return fail(size_out,"Bad service file");

  int i;
  if((i=serviceManager.check(file,name)))
    {
      if(i==1)
	{
	  return fail(size_out,"is already loaded.");
	}
      if(i==2)
	return fail(size_out,"The service from file "
		    "\"%s\" is already loaded,"
		    " try naming it.",
		    file);
    }

  if(!(name[0])) name = NULL;
  if(!(arg[0])) arg = NULL;

  if(serviceManager.add(file, name, arg))
    return fail(size_out,"failed to load.");

  return success(size_out,"service loaded.");
}

char *ServerRespond::loadConfig(size_t *size_out, const char *config_file, const char* path,
    const char* cwd, const char* svc_path, const char* cal_path, const char* cal_conf_path)
{
  if(!config_file || !(config_file[0]))
    return fail(size_out,"Bad configuration file");

  if(serviceManager.loadConfig(config_file,path,cwd,svc_path,cal_path,cal_conf_path))
    return fail(size_out,"failed to load.");

  return success(size_out,"service loaded.");
}

char *ServerRespond::
unloadService(size_t *size_out, const char *name)
{
  if(!name || !(name[0]))
    return fail(size_out,"bad service name.");

  if(!serviceManager.check(name))
    return fail(size_out,"is not loaded.");

  if(serviceManager.remove(name))
    return fail(size_out,"failed to unload.");

  return success(size_out,"unloaded.");
}

char *ServerRespond::
resetService(size_t *size_out, const char *name)
{
  if(!name || !(name[0]))
    return fail(size_out,"bad service name.");

  dtkService *s = serviceManager.get(name);

  if(!s)
    return fail(size_out,"service \"%s\" is not loaded.", name);

  s->reset();

  return success(size_out,"reset called.");
}

char *ServerRespond::
checkService(size_t *size_out, const char *name)
{
  if(!name || !(name[0]))
    return fail(size_out,"bad service name.");

  if(serviceManager.check(name))
    return success(size_out,"service loaded.");
  else
    return fail(size_out,"a service named \"%s\" is not loaded.", name);
}

// (0) From client to server
char *ServerRespond::
connectServer(size_t *size_out, const char *addressPort,
	      const char *fromClientAddr)
{
  dtkSocket *sock = serverSocketList.get(addressPort);
  if(sock)
    {
      return success(size_out, "servers already connected.");
    }
  else
    {
      // dtk_getCommonAddressPort() will fill in Port with
      // DEFAULT_DTK_SERVER_PORT if it is missing in addressPort.
      char *addrPort = dtk_getCommonAddressPort(addressPort);
      ServiceClientTCP *s = new ServiceClientTCP(addrPort);
      if(s->dtkService::fd < 0)
	{
	  delete s;
	  return fail(size_out,"failed to connect to \"%s\".",
		      addrPort);
	}
      sock = s;
    }

  // Compose request to the DTK server at addressPort.

  response[0] = _DTKSREQ_CONNECTSERVERS;

  size_t length =
    dtk_splice(RESPONSE_BUF_SIZE-1, &(response[1]), serverPort,
	       fromClientAddr, NULL);

  if(!length)
    {
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "ServerRespond::connectServer(\"%s\") failed: "
		 "total request length is longer than %d bytes:\n"
		 "file=%s line=%d.\n",
		 addressPort, RESPONSE_BUF_SIZE,__FILE__,__LINE__);
      return fail(size_out,"file=%s line=%d of DTK server"
		  " needs work, buffer too small.",
		  __FILE__,__LINE__);
    }
  length++; // add the buffer[0] = _DTKSREQ_CONNECTSERVERS to length

  if(sock->write(response,length) != (ssize_t) length)
    {
      dtkMsg.add(DTKMSG_WARN,0,-1,
		 "ServerRespond::connectServer"
		 "(\"%s\") failed.\n", addressPort);
      return fail(size_out,"socket write to \"%s\" failed.", addressPort);
    }

  serverDebugPrintCommand(0,addressPort,response,length);

  return NULL; // successful so far. Will respond to client later.
}

// (1) From server to server that failed do to not being a networked
// server.
char *ServerRespond::connectServerFail(size_t *size_out,
				       const char *addressPort,
				       const char *fromClientAddr)
{
  response[0] = _DTKSSREQ_CONNECTSERVERS;

  *size_out =
    dtk_splice(RESPONSE_BUF_SIZE-1, &(response[1]), addressPort,
	       fromClientAddr,
	       "remote server is not a networked server.",
	       NULL);
  
  if(!(*size_out))
    {
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "ServerRespond::connectServerFail() failed: "
		 "total request length is longer than %d bytes:\n"
		 "file=%s line=%d.\n",
		 RESPONSE_BUF_SIZE,__FILE__,__LINE__);
      return fail(size_out,"file=%s line=%d of DTK server"
		  " needs work, buffer too small.",
		  __FILE__,__LINE__);
    }
  (*size_out)++; // add the buffer[0] = _DTKSSREQ_CONNECTSERVERS
  return response;
}

// (2) From server to server back to client
char *ServerRespond::
connectServer(size_t *size_out, const char *addressPort,
	      const char *fromClientAddr, const char *errorStr)
{
  size_t length;

  if(errorStr[0] != '\0')
    {
      serverSocketList.remove(addressPort);
      response[0] = _DTKRESP_FAILED;
      length =
	dtk_splice(RESPONSE_BUF_SIZE-1, &(response[1]), errorStr, NULL);
    }
  else
    {
      response[0] = _DTKRESP_SUCCESS;
      length =
	dtk_splice(RESPONSE_BUF_SIZE-1, &(response[1]),
		   "servers connected", NULL);
    }

  if(!length)
    {
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "ServerRespond::connectServer(\"%s\") failed: "
		 "total request length is longer than %d bytes:\n"
		 "file=%s line=%d.\n",
		 addressPort, RESPONSE_BUF_SIZE,__FILE__,__LINE__);
      return fail(size_out,"file=%s line=%d of DTK server"
		  " needs work, buffer too small.",
		  __FILE__,__LINE__);
    }
  length++; // add the buffer[0] to length

  dtkSocket *sock = serverSocketList.get(fromClientAddr);
  if(sock)
    {
      sock->write(response,length);
      serverDebugPrintCommand(0,fromClientAddr,response,length);
    }

  return NULL;
}


// (0) Client request to this server
char *ServerRespond::
connectSharedMem(size_t *size_out,
	       const char *name,       const char *addressPort,
	       const char *remoteName, const char *fromClientAddrPort)
{
  dtkSharedMem *shm = getSharedMem(name);

  if(!shm) return fail(size_out,
		       "local shared memory \"%s\" not found.", name);

  dtkSocket *sock = serverSocketList.get(addressPort);
  if(!sock)
    return fail(size_out,"not connected to \"%s\".",
		addressPort);

  // Compose request to the DTK server at addressPort.

  response[0] = _DTKSREQ_CONNECTREMOTESEG;

  char size_str[20];
#ifdef DTK_ARCH_WIN32_VCPP
  sprintf(size_str,"%u",shm->getSize());
#else
  sprintf(size_str,"%zd",shm->getSize());
#endif

  size_t length =
    dtk_splice(RESPONSE_BUF_SIZE-1, &(response[1]), name,
	       remoteName, size_str, fromClientAddrPort, NULL);

  if(!length)
    {
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "ServerRespond::connectSharedMem() failed: "
		 "total request length is longer than %d bytes:\n"
		 "file=%s line=%d.\n",
		 RESPONSE_BUF_SIZE,__FILE__,__LINE__);
      return fail(size_out,"file=%s line=%d of DTK server"
		  " needs work, buffer too small.",
		  __FILE__,__LINE__);
    }

  length++; // add the buffer[0] = _DTKSREQ_CONNECTREMOTESEG to length

  if(sock->write(response,length) != (ssize_t) length)
    {
      dtkMsg.add(DTKMSG_WARN,0,-1,
		 "ServerRespond::connectSharedMem"
		 "(\"%s\",\"%s\",\"%s\") failed.\n",
		 name, addressPort, remoteName);
      return fail(size_out,"socket write to \"%s\" failed.", addressPort);
    }

  serverDebugPrintCommand(0,addressPort,response,length);

  // Will respond later. Need to wait for response from other server.
  return NULL; // NULL is for successfull so far in this case.
}

// (1) server to server.  This is the remote server.
char *ServerRespond::
connectSharedMem(size_t *size_out,
	       size_t seg_size,
	       const char *name,
	       const char *remoteAddrPort,
	       const char *remoteName,
	       const char *fromClientAddrPort)
{
  dtkMsg.add(DTKMSG_DEBUG,
	     "GOT request: for shared memory(size=%d)=\"%s\""
	     " remoteAddrPort=\"%s\""
	     " remotename=\"%s\" fromclient=\"%s\"\n",
	     seg_size, name, remoteAddrPort, remoteName, fromClientAddrPort);

  dtkSharedMem *shm = getSharedMem(remoteName);

  char err_str[64];
  err_str[0] = '\0';
  response[0] = _DTKSSREQ_CONNECTREMOTESEG;

  if(!shm)
    snprintf(err_str, 64, "remote shared memory \"%s\" not found.",
	     remoteName);
  else if(shm->getSize() != seg_size)
    snprintf(err_str, 64, "remote shared memory "
#ifdef DTK_ARCH_WIN32_VCPP
	     "\"%s\" is size=%u not %u.",
#else
	     "\"%s\" is size=%zd not %zd.",
#endif
	     remoteName, shm->getSize(), seg_size);

  if(err_str[0] == '\0')
    {
      char *_remoteAddrPort = dtk_getCommonAddressPort(remoteAddrPort);
      if(!_remoteAddrPort)
	{
	  snprintf(err_str, 64, "remote address \"%s\" not found.",
		   remoteAddrPort);
	}
      else// success ... Add "name" shared memory write list.
	{
	  remoteAddrPort = _remoteAddrPort;
	  shm->addToWriteList(remoteAddrPort, name);
	}
    }

  size_t length =
    dtk_splice(RESPONSE_BUF_SIZE-1, &(response[1]),
	       name, remoteName, fromClientAddrPort, err_str, NULL);

  if(!length)
    {
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "ServerRespond::connectSharedMem() failed: "
		 "total request length is longer than %d bytes:\n"
		 "file=%s line=%d.\n",
		 RESPONSE_BUF_SIZE,__FILE__,__LINE__);
      return fail(size_out,"file=%s line=%d of DTK server"
		  " needs work, buffer too small.",
		  __FILE__,__LINE__);
    }
  length++; // for adding _DTKSSREQ_CONNECTREMOTESEG to length

  *size_out = length;

  return response;
}

// (2) server to server than write response to client
char *ServerRespond::
connectSharedMem(size_t *size_out,
	       const char *name,
	       const char *addressPort,
	       const char *remoteName,
	       const char *fromClientAddrPort,
	       const char *errorString)
{
  dtkMsg.add(DTKMSG_DEBUG,
	     "GOT request: for sharedMem=\"%s\" remote AddressPort=\"%s\" "
	     " remotename=\"%s\" fromclient=\"%s\" errorString=<%s>\n",
	     name, addressPort, remoteName, fromClientAddrPort, errorString);

  char *_addressPort =
    dtk_getCommonAddressPort(addressPort);

  dtkSocket *sock = serverSocketList.get(fromClientAddrPort);
  if(!sock)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "can't find socket \"%s\"to client request.\n",
		 fromClientAddrPort);
    }
  else
    {
      if(errorString[0])
	snprintf(response, RESPONSE_BUF_SIZE,"%c%s",
		 _DTKRESP_FAILED, errorString);
      else
	{
	  dtkSharedMem *shm = getSharedMem(name);
	  if(shm && _addressPort)
	    {
	      int i = shm->addToWriteList(_addressPort, remoteName);
	      if(i == 0)
		sprintf(response,"%cshared memory connected.",
			_DTKRESP_SUCCESS);
	      else if(i == 1)
		sprintf(response,"%cshared memory reconnected.",
			_DTKRESP_SUCCESS);
	      else
		sprintf(response,
			"%cfailed to add to local write list.",
			_DTKRESP_FAILED);
	    }
	  else if(!shm)
	    sprintf(response,
		    "%cfailed to get local shared memory.",
		    _DTKRESP_FAILED);
	  else // !_addressPort
	    sprintf(response,
		    "%cfailed to get remote getCommonAddressPort.",
		    _DTKRESP_FAILED);
	}

      size_t length = strlen(response) + 1;
      sock->write(response, length);
      serverDebugPrintCommand(0,addressPort,response,length);
    }

  *size_out = 0;
  return NULL;
}

char *ServerRespond::sharedMemWriteList(size_t *size_out, const char *segName)
{
   char *name;
   char *addr;
   dtkSegAddrList *segAddr;

   response[0] = _DTKRESP_SEGWRITELIST;

   dtkSharedMem *shm = getSharedMem(segName);

   if(!shm)
     return fail(size_out, "shared memory write list request failed");

   switch(shm->getNetworkType())
     {
     case DTK_TCP:
       response[1] = _DTK_WRITE_TCP;
       break;
     case DTK_UDP:
     default:
       response[1] = _DTK_WRITE_UDP;
       break;
     }

   size_t size = 2;
   size_t i = dtk_splice(RESPONSE_BUF_SIZE-size,
			 &(response[size]), segName, NULL);
   size += i;
   if(!i) goto sharedMemWriteList_error;

   segAddr = shm->getSegAddrList();

   segAddr->start();
   addr = segAddr->getNextAddressPort(&name);

   for(;addr;addr = segAddr->getNextAddressPort(&name))
     {
       i = dtk_splice(RESPONSE_BUF_SIZE-size, &(response[size]),
		      addr, name, NULL);
       if(!i || (size_t) RESPONSE_BUF_SIZE < size) { i=0; break; }
       size += i;
     }

   *size_out = size;
   if(i)
     return response;

 sharedMemWriteList_error:

  dtkMsg.add(DTKMSG_WARN, 0,-1,
	     "ServerRespond::sharedMemWriteList() failed: "
	     "total request length is longer than %d bytes:\n"
	     "file=%s line=%d.\n",
	     RESPONSE_BUF_SIZE,__FILE__,__LINE__);
  return fail(size_out,"file=%s line=%d of DTK server"
	      " needs work, DTK server buffer too small.",
	      __FILE__,__LINE__);
}
