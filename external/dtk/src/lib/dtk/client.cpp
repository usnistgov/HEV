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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <direct.h>
# include <winsock2.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <float.h>
# include <unistd.h>
# include <errno.h>
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "dtkSocket.h"
#include "dtkTCPSocket.h"
#include "dtkClient.h"
#include "serverProtocol.h"
#include <string>


#define DTKCLIENT_CONNECTSEG_TIMEOUT  ((unsigned int) 20) // seconds

#ifdef DTK_ARCH_WIN32_VCPP
#  define SEPARATOR ";"
#else
#  define SEPARATOR ":"
#endif

static char *get_addressPort(const char *connection)
{
  static char ret[128];
  if(connection && connection[0])
    {
      // fill in any missing address or port
      if(connection[0] == ':') // have just port
	{
	  snprintf(ret,128,"%s%s",DTK_DEFAULT_SERVER_ADDRESS,connection);
	}
      else
	{
	  char *s = (char *) connection;
	  for(;*s;s++)
	    if(*s == ':')
	      break;
	  if(*s) // connection should be a full address:port
	    snprintf(ret,128,"%s",connection);
	  else // connection should be just the address without
	    // port
	    {
	      snprintf(ret,128,"%s:%s",connection,DTK_DEFAULT_SERVER_PORT);
	    }
	}
    }
  else
    snprintf(ret,128,"%s",DTK_DEFAULT_SERVER_CONNECTION);
  
  return ret;
}


dtkClient::dtkClient(const char *connection_in) :
  dtkBase(DTKCLIENT_TYPE)
{
  // initialize all data
  tcp = NULL;

  /////////////////////

  // get address_port or DTK_CONNECTION
  // address_port can be like  :23421, foo.com:dtk,
  // foo.com:12334, foo.com, or 127.125.23.56:1234

  connection = (char *) connection_in;

  if(!connection || !(connection[0]))
    {
      connection = getenv("DTK_CONNECTION");
      connection = dtk_strdup(get_addressPort(connection));
    }
  else
    connection = dtk_strdup(get_addressPort(connection));

  tcp = new dtkTCPSocket(connection, PF_INET);
  if(!tcp || tcp->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,"dtkClient::dtkClient(\"%s\")"
		 " failed with address:port set to \"%s\".\n",
		 connection_in, connection);
      if(tcp)
	{
	  delete tcp;
	  tcp = NULL;
	}
      return;
    }

  connection = dtk_strdup(connection);

  validate();
}

void dtkClient::clean_up(void)
{
  if(tcp)
    {
      delete tcp;
      tcp = NULL;
    }
  if(connection)
    {
      free(connection);
      connection = NULL;
    }

  invalidate();
}

dtkClient::~dtkClient(void)
{
  clean_up();
}


int dtkClient::loadService(const char *file, const char *name,
			   const char *arg)
{
  if(isInvalid())
    return
      dtkMsg.add(DTKMSG_ERROR,0,-1,
		 "dtkClient::loadService(\"%s\",\"%s\",\"%s\")"
		 " failed: invalid dtkClient object.\n",
		 file, name, arg);

  if(!file || !(file[0]))
    return
      dtkMsg.add(DTKMSG_ERROR,0,-1,
		 "dtkClient::loadService(\"%s\",\"%s\",\"%s\")"
		 " failed: bad filename, \"%s\".\n",
		 file, name, arg, file);

  // I get the full path of the DSO service file if it can be found.
  // This way even if the DTK server cannot find the DSO file this way
  // find it for the server.  The simplest case that this fixes is
  // when the DTK client program is running from a directory that
  // contains the DSO file, but the DTK server has no idea about the
  // where-abouts of the DSO file.

  char *DSOFile = NULL;

  if(file[0] == '/') DSOFile = strdup(file);
  else // Get the full path of the service DSO file if the file is in
  // the DTK_SERVICE_PATH or the current working directory.
  {
    char *path = getenv("DTK_SERVICE_PATH");
    if(path)
	{
      path = (char *) dtk_malloc(strlen(path) + 3);
      sprintf(path, "%s:.", getenv( "DTK_SERVICE_PATH" ) );
	}

    char *file_so = NULL;

    size_t len = strlen(file);
    if(len < (size_t) 3 || strcmp(&(file[len-3]),".so"))
    {
      file_so = (char *) dtk_malloc(strlen(file) + 4);
      sprintf(file_so,"%s.so",file);
      DSOFile = file_so;
    }
    else
    DSOFile = (char *) file;

    DSOFile = dtk_getFileFromPath(path, DSOFile, (mode_t) 0111);

    if(file_so) free(file_so);
    if(path) free(path);
  }

  if(!DSOFile)
    DSOFile = strdup(file);

  // Compose the request.
  if(!name)
    name = "";
  if(!arg)
    arg = "";

  buffer[0] = _DTKREQ_LOADSERVICE;

  size_t length =
    dtk_splice(DTKCLIENT_BUFF_SIZE-1, &(buffer[1]),
	       DSOFile, name, arg, NULL);

  if(DSOFile)
    free(DSOFile);

  if(!length)
    return
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "dtkClient::loadService() failed: "
		 "total request length is longer than %d bytes.\n",
		 DTKCLIENT_BUFF_SIZE);

  length++; // add the buffer[0]= _DTKREQ_LOADSERVICE

  if(tcp->write(buffer,length) != (ssize_t) length)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::loadService(\"%s\","
		      "\"%s\",\"%s\") failed.\n",
		      file, name, arg);

  if(tcp->read(buffer,DTKCLIENT_BUFF_SIZE) < (ssize_t)1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::loadService(\"%s\","
		      "\"%s\",\"%s\") failed to get "
		      "a response from the DTK server.\n",
		      file, name, arg);

  if(*buffer == _DTKRESP_SUCCESS) // success
    return dtkMsg.add(DTKMSG_NOTICE,0,0,"%s\n", &(buffer[1]));
  else // error
    return dtkMsg.add(DTKMSG_WARN,0,-1, "%s\n", &(buffer[1]));
}

int dtkClient::loadConfig( const char* file )
{
  if(isInvalid())
    return
      dtkMsg.add(DTKMSG_ERROR,0,-1,
		 "dtkClient::loadConfig(\"%s\")"
		 " failed: invalid dtkClient object.\n",
		 file);

  if(!file || !(file[0]))
    return
      dtkMsg.add(DTKMSG_ERROR,0,-1,
		 "dtkClient::loadConfig(\"%s\")"
		 " failed: bad filename, \"%s\".\n",
		 file);

  // I get the full path of the configuration file if it can be found.
  // This way even if the DTK server cannot find the config file this way
  // find it for the server.  The simplest case that this fixes is
  // when the DTK client program is running from a directory that
  // contains the config file, but the DTK server has no idea about the
  // where-abouts of the config file.

  char *configFile = NULL;
  std::string path;

  if(file[0] == '/')
    configFile = strdup(file);
  else // Get the full path of the service DSO file if the file is in
  // the DTK_SERVICE_CONFIG_PATH or the current working directory.
  {
    if(getenv( "DTK_SERVICE_CONFIG_PATH" ))
    {
      path = getenv( "DTK_SERVICE_CONFIG_PATH" );
    }
	if(getenv( "DTK_SERVICE_PATH" ))
    {
      if( path.size() )
        path += SEPARATOR;
      path += getenv( "DTK_SERVICE_PATH" );
    }

    configFile = (char *) file;

    configFile = dtk_getFileFromPath(path.c_str(), configFile, (mode_t) 0000);

  }

  if(!configFile)
    configFile = strdup(file);
  // Compose the request.

  unsigned int cwd_size = 1024;
  char* cwd = new char[cwd_size];
  if( !cwd )
  {
    return dtkMsg.add( DTKMSG_ERROR, 0, -1, "Unable to allocate memory for buffer.\n" );
  }
  while( !getcwd( cwd, cwd_size - 1 ) && cwd_size <= DTKCLIENT_BUFF_SIZE )
  {
    if( errno == ERANGE )
    {
      delete cwd;
      cwd_size += 1024;
      cwd = new char[cwd_size];
      if( !cwd )
      {
        return dtkMsg.add( DTKMSG_ERROR, 0, -1, "Unable to allocate memory for buffer.\n" );
      }
    }
    else if( errno == ENOMEM )
    {
      return dtkMsg.add( DTKMSG_ERROR, 0, -1, "Buffer memory cannot be mapped.\n" );
    }
    else if( errno == ENOENT )
    {
      return dtkMsg.add( DTKMSG_ERROR, 0, -1, "Directory does not exist - possibly deleted.\n" );
    }
    else if( errno == EFAULT )
    {
      return dtkMsg.add( DTKMSG_ERROR, 0, -1, "A memory access violation occurred.\n" );
    }
    else
    {
      return dtkMsg.add( DTKMSG_ERROR, 0, -1, "Unknown error getting current working directory.\n" );
    }
  }

  char* svc_path = getenv( "DTK_SERVICE_PATH" );
  char* cal_path = getenv( "DTK_CALIBRATION_PATH" );
  char* cal_conf_path = getenv( "DTK_CALIBRATION_CONFIG_PATH" );

  if( cwd )
  {
    path += "CWD=";
    path += cwd;
  }
  if( svc_path )
  {
    path += "SVC=";
    path += svc_path;
  }
  if( cal_path )
  {
    path += "CAL=";
    path += cal_path;
  }
  if( cal_conf_path )
  {
    path += "CALCONF=";
    path += cal_conf_path;
  }

  buffer[0] = _DTKREQ_LOADCONFIG;
  size_t length = dtk_splice(DTKCLIENT_BUFF_SIZE-1, &(buffer[1]),
      configFile, path.c_str(), NULL);

  if(configFile)
    free(configFile);

  if(!length)
    return
      dtkMsg.add(DTKMSG_WARN, 0,-1,
          "dtkClient::loadService() failed: "
          "total request length is longer than %d bytes.\n",
          DTKCLIENT_BUFF_SIZE);

  length++; // add the buffer[0]= _DTKREQ_LOADSERVICE

  if(tcp->write(buffer,length) != (ssize_t) length)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
        "dtkClient::loadService(\"%s\") failed.\n",
        file);

  if(tcp->read(buffer,DTKCLIENT_BUFF_SIZE) < (ssize_t)1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
        "dtkClient::loadService(\"%s\") failed to get "
        "a response from the DTK server.\n",
        file);

  if(*buffer == _DTKRESP_SUCCESS) // success
    return dtkMsg.add(DTKMSG_NOTICE,0,0,"%s\n", &(buffer[1]));
  else // error
    return dtkMsg.add(DTKMSG_WARN,0,-1, "%s\n", &(buffer[1]));
}

int dtkClient::unloadService(const char *name)
{
  if(isInvalid())
    dtkMsg.add(DTKMSG_ERROR,0,-1,
	       "dtkClient::unloadService(\"%s\")"
	       " failed: invalid dtkClient object.\n",
	       name);
  
  if(!name || !(name[0]))
    return dtkMsg.add(DTKMSG_ERROR,0,-1,
		      "dtkClient::unloadService(\"%s\")"
		      " failed: bad name, \"%s\".\n",
		      name, name);

  // Compose the request.

  buffer[0] = _DTKREQ_UNLOADSERVICE;

  size_t length =
    dtk_splice(DTKCLIENT_BUFF_SIZE-1, &(buffer[1]), name, NULL);

  if(!length)
    return
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "dtkClient::unloadService() failed: "
		 "total request length is longer than %d bytes.\n",
		 DTKCLIENT_BUFF_SIZE);

  length++; // add the buffer[0]= _DTKREQ_UNLOADSERVICE
  
  if(tcp->write(buffer,length) != (ssize_t) length)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::unloadService(\"%s\") failed.\n",
		      name);

  if(tcp->read(buffer,DTKCLIENT_BUFF_SIZE) < (ssize_t)1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::unloadService(\"%s\")"
		      "failed to get a response"
		      " from the DTK server.\n",
		      name);

  if(*buffer == _DTKRESP_SUCCESS) // success
    return dtkMsg.add(DTKMSG_NOTICE,0,0,"%s\n", &(buffer[1]));
  else // error
    return dtkMsg.add(DTKMSG_WARN,0,-1, "%s\n", &(buffer[1]));
}


int dtkClient::resetService(const char *name)
{
  if(isInvalid())
    dtkMsg.add(DTKMSG_ERROR,0,-1,
	       "dtkClient::resetService(\"%s\")"
	       " failed: invalid dtkClient object.\n",
	       name);
  
  if(!name || !(name[0]))
    return dtkMsg.add(DTKMSG_ERROR,0,-1,
		      "dtkClient::resetService(\"%s\")"
		      " failed: bad name, \"%s\".\n",
		      name, name);

  // Compose the request.

  buffer[0] = _DTKREQ_RESETSERVICE;

  size_t length =
    dtk_splice(DTKCLIENT_BUFF_SIZE-1, &(buffer[1]), name, NULL);

  if(!length)
    return
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "dtkClient::resetService() failed: "
		 "total request length is longer than %d bytes.\n",
		 DTKCLIENT_BUFF_SIZE);

  length++; // add the buffer[0]= _DTKREQ_RESETSERVICE
  
  if(tcp->write(buffer,length) != (ssize_t) length)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::resetService(\"%s\") failed.\n",
		      name);

  if(tcp->read(buffer,DTKCLIENT_BUFF_SIZE) < (ssize_t)1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::resetService(\"%s\")"
		      "failed to get a response"
		      " from the DTK server.\n",
		      name);

  if(*buffer == _DTKRESP_SUCCESS) // success
    return dtkMsg.add(DTKMSG_NOTICE,0,0,"%s\n", &(buffer[1]));
  else // error
    return dtkMsg.add(DTKMSG_WARN,0,-1, "%s\n", &(buffer[1]));
}



int dtkClient::checkService(const char *name)
{
  if(isInvalid())
    dtkMsg.add(DTKMSG_ERROR,0,-1,
	       "dtkClient::checkService(\"%s\")"
	       " failed: invalid dtkClient object.\n",
	       name);
  
  if(!name || !(name[0]))
    return dtkMsg.add(DTKMSG_ERROR,0,-1,
		      "dtkClient::checkService(\"%s\")"
		      " failed: bad name, \"%s\".\n",
		      name, name);

  // Compose the request.

  buffer[0] = _DTKREQ_CHECKSERVICE;

  size_t length =
    dtk_splice(DTKCLIENT_BUFF_SIZE-1, &(buffer[1]), name, NULL);

  if(!length)
    return
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "dtkClient::checkService() failed: "
		 "total request length is longer than %d bytes.\n",
		 DTKCLIENT_BUFF_SIZE);

  length++; // add the buffer[0]= _DTKREQ_CHECKSERVICE
  
  if(tcp->write(buffer,length) != (ssize_t) length)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::checkService(\"%s\") failed.\n",
		      name);

  if(tcp->read(buffer,DTKCLIENT_BUFF_SIZE) < (ssize_t)1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::checkService(\"%s\")"
		      "failed to get a response"
		      " from the DTK server.\n",
		      name);

  if(*buffer == _DTKRESP_SUCCESS) // success
    return dtkMsg.add(DTKMSG_NOTICE,0,0,"%s\n", &(buffer[1]));
  else // error
    return dtkMsg.add(DTKMSG_WARN,0,-1, "%s\n", &(buffer[1]));
}

int dtkClient::shutdownServer(void)
{
  if(isInvalid())
    dtkMsg.add(DTKMSG_ERROR,0,-1,
	       "dtkClient::shutdownServer()"
	       " failed: invalid dtkClient object.\n");
  
  // Compose the request.

  buffer[0] = _DTKREQ_SHUTDOWN;

  if(tcp->write(buffer,(size_t)1) != (ssize_t) 1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::shutdownServer() failed.\n");

 
  if(tcp->read(buffer,DTKCLIENT_BUFF_SIZE) < (ssize_t) 1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::shutdownServer() "
		      "failed to get a response"
		      " from the DTK server.\n");

  if(*buffer == _DTKRESP_SUCCESS) // success
    {
      clean_up();
      return dtkMsg.add(DTKMSG_NOTICE,0,0,"server shutting down\n");
    }
  else // error
    return dtkMsg.add(DTKMSG_WARN,0,-1, "%s\n", &(buffer[1]));
}


int dtkClient::
connectRemoteSharedMem(const char *name,
		       const char *addressPort,
		       const char *remoteName_in)
{
  if(isInvalid())
    dtkMsg.add(DTKMSG_ERROR,0,-1,
	       "dtkClient::connectRemoteSharedMem"
	       "(\"%s\",\"%s\",\"%s\")"
	       " failed: invalid dtkClient object.\n",
	       name, addressPort, remoteName_in);

  if(!name || !(name[0]) || !addressPort || !(addressPort[0]))
    return dtkMsg.add(DTKMSG_ERROR,0,-1,
		      "dtkClient::connectRemoteSharedMem"
		      "(\"%s\",\"%s\",\"%s\")"
		      " failed: bad argument.\n",
		      name, addressPort, remoteName_in);

  // first connect the servers if they haven't already.
  if(connectServer(addressPort)) return -1;

  // Compose the request.

  buffer[0] = _DTKREQ_CONNECTREMOTESEG;

  char *remoteName = (char *) remoteName_in;
  if(!remoteName) remoteName = (char *) name;

  char *addrPort = get_addressPort(addressPort);
  addrPort = dtk_getCommonAddressPort(addrPort);
  if(!addrPort)
    addrPort = get_addressPort(addressPort);

  size_t length =
    dtk_splice(DTKCLIENT_BUFF_SIZE-1, &(buffer[1]), name,
	       addrPort, remoteName, NULL);

  if(!length)
    return
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "dtkClient::connectRemoteSharedMem() failed: "
		 "total request length is longer than %d bytes.\n",
		 DTKCLIENT_BUFF_SIZE);

  length++; // add the buffer[0] = _DTKREQ_CONNECTREMOTESEG;
  
  if(tcp->write(buffer,length) != (ssize_t) length)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::connectRemoteSharedMem"
		      "(\"%s\",\"%s\",\"%s\") failed.\n",
		      name, addressPort, remoteName_in);

  if(tcp->read(buffer,DTKCLIENT_BUFF_SIZE,
	       DTKCLIENT_CONNECTSEG_TIMEOUT) < (ssize_t)1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::connectRemoteSharedMem"
		      "(\"%s\",\"%s\",\"%s\")\n"
		      "   failed to get a response"
		      " from the DTK server.\n",
		      name, addressPort, remoteName_in);

  if(*buffer == _DTKRESP_SUCCESS) // success
    return dtkMsg.add(DTKMSG_NOTICE,0,0,"%s\n", &(buffer[1]));
  else // error
    return dtkMsg.add(DTKMSG_WARN,0,-1,"%s\n", &(buffer[1]));
}


int dtkClient::connectServer(const char *addressPort)
{
  if(isInvalid())
    dtkMsg.add(DTKMSG_ERROR,0,-1,
	       "dtkClient::connectServer(\"%s\")"
	       " failed: invalid dtkClient object.\n",
	       addressPort);
  
  if(!addressPort || !(addressPort[0]))
    return dtkMsg.add(DTKMSG_ERROR,0,-1,
		      "dtkClient::connectServer(\"%s\")"
		      " failed: bad addressPort, \"%s\".\n",
		      addressPort, addressPort);

  char *addrPort = get_addressPort(addressPort);
  addrPort = dtk_getCommonAddressPort(addrPort);
  if(!addrPort)
    addrPort = get_addressPort(addressPort);

  if(!strcmp(addrPort, connection))
    return dtkMsg.add(DTKMSG_ERROR,0,-1,
		      "dtkClient::connectServer(\"%s\")"
		      " failed: can't connect this server to"
		      " itself \"%s\".\n",
		      addressPort, connection);

  // Compose the request.

  buffer[0] = _DTKREQ_CONNECTSERVERS;

  size_t length =
    dtk_splice(DTKCLIENT_BUFF_SIZE-1, &(buffer[1]), addrPort, NULL);

  if(!length)
    return
      dtkMsg.add(DTKMSG_WARN, 0,-1,
		 "dtkClient::connectServer() failed: "
		 "total request length is longer than %d bytes.\n",
		 DTKCLIENT_BUFF_SIZE);

  length++; // add the buffer[0]= _DTKREQ_CHECKSERVICE
  
  if(tcp->write(buffer,length) != (ssize_t) length)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::connectServers(\"%s\") failed.\n",
		      addressPort);

  if(tcp->read(buffer,DTKCLIENT_BUFF_SIZE) < (ssize_t)1)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkClient::connectServer(\"%s\")"
		      "failed to get a response"
		      " from the DTK server.\n",
		      addressPort);

  if(*buffer == _DTKRESP_SUCCESS) // success
    return dtkMsg.add(DTKMSG_NOTICE,0,0,"%s\n", &(buffer[1]));
  else // error
    return dtkMsg.add(DTKMSG_WARN,0,-1, "%s\n", &(buffer[1]));
}
