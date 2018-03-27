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

/* The client buffer size */
#define DTKCLIENT_BUFF_SIZE  ((size_t) 4096)

/*! \class dtkClient dtkClient.h dtk.h
  * \brief The client part of the DTK client/server architecture
  *
  * This class uses an Internet socket to connect to the
  * DTK server.  The methods in this class are used to send
  * requests to the DTK server and get responses.
  */
class DTKAPI dtkClient : public dtkBase
{
public:

  /*!
   * \param addressPort is the IP address and port to use to connect
   * this client to the DTK server.
   *
   * If \e addressPort is NULL it uses
   * environment varable DTK_CONNECTION if it is defined, else it uses
   * DEFAULT_DTK_SERVER_CONNECTION, which is defined in the header
   * file dtk/_config.h, which is included in dtk.h.  If \e
   * addressPort is non-NULL it will fill in any missing port or
   * address information from DEFAULT_DTK_SERVER_PORT or
   * DEFAULT_DTK_SERVER_ADDRESS as defined in the header file
   * dtk/_config.h.
   */
  dtkClient(const char *addressPort=NULL);

  /*! destructor
   */
  virtual ~dtkClient(void);

  //! Connect two DTK servers
  /*!
   * Connect this clients DTK server to a remote DTK server at the
   * Internet (IP) address and port given by the string in \e
   * addressPort.  This connection between two DTK servers may remain
   * after this clients connection is terminated.
   *
   * \param addressPort For example \e addressPort = "foo.com:1234".
   * Something like \e addressPort = "foo.com" would use the default
   * DTK server port.
   */
  int connectServer(const char *addressPort);

  //! Load a service into the server
  /*!
   * \param file Load a DTK service via a dynamic shared object (DSO)
   * file given by \e file. If the file is not a full path file name
   * the environment variable DTK_SERVICE_PATH with be used as a
   * search path, else if environment variable DTK_SERVICE_PATH is not
   * set the current directory and then the service DSO installation
   * directory are searched. The service DSO installation directory
   * may be found from the output of `dtk-config --service-dso-dir'
   * which may be changed by setting the environment variable
   * DTK_ROOT.
   *
   * \param name \e name is the optional name of the service to be
   * loaded, which may be used to refer to the loaded service in
   * future commands. The default name is the base name of the file
   * loaded without any ".so" postfix. DTK services get named at the
   * time of loading and do not name themselves. This name is unique
   * for a ecah service on a given DTK server.
   *
   * \param arg \e arg is an augment that is sent to the
   * service as it is loaded.
   *
   * \return  Returns 0 on success, and non-zero on error.
   */
  int loadService(const char *file, const char *name=NULL,
		  const char *arg=NULL);

  //! Load service(s) from a service configuration file
  /*!
   * \param file config_file is the configuration file to be loaded
   * which should contain listings of DSOs to load typically derived from
   * dtkConfigService.
   *
   * \return Returns 0 on success, and non-zero on error.
   */
  int loadConfig( const char* file );

  //! Unload a service from the server
  /*!
   * Unload a service from the DTK server.
   *
   * \param name \e name is the name of the service to unload.  The
   * name comes to be when the service is loaded.
   *
   * \return Returns 0 on success, or non-zero on error.
   */
  int unloadService(const char *name);

  //! Request a reset of a service
  /*!
   * \param name Request the loaded DTK service named \e name to do a
   * to call it's dtkService::reset() method.
   *
   * \return Returns 0 if the service successfully resets, or non-zero
   * otherwise.
   */
  int resetService(const char *name);

  //! Check if a service is loaded
  /*!
   * \param name \e name is the name of the service to check for.
   *
   * Returns 0 if the service named \e name is loaded or non-zero
   * otherwise.
   */
  int checkService(const char *name);
 
  //! Tell the DTK server to shutdown
  /*!
   * Returns zero if the DTK server that this DTK client is connected
   * to successfully receives this command.
   *
   * \return Returns 0 on success, or non-zero on error.
   */
  int shutdownServer(void);

  //! Connect remote DTK shared memory
  /*!
   * Connect the DTK shared memory file \e shmFilename to DTK server
   * on the remote Internet address and port given by \e addressPort.
   *
   * \param shmFileName is the file name of the DTK shared memory file
   * on the local system.  If the full path file name is not given and
   * if the environment variable DTK_SHAREDMEM_DIR is set the
   * directory in DTK_SHAREDMEM_DIR is used to get the full path, else
   * if DTK_SHAREDMEM_DIR is not set the directory will be that which
   * is printed from the program `dtk-config --sharedMem-dir' on the
   * system where the DTK server is running.
   *
   * \param addressPort For example \e addressPort = "foo.com:1234".
   * Something like \e addressPort = "foo.com" would use the default
   * DTK server port.
   *
   * \param remoteName The remote DTK shared memory file name will be
   * the same as the local DTK shared memory file if \e remoteName is
   * NULL, else the remote shared memory file name will be \e
   * remoteName.
   *
   * \return Returns 0 on success, or non-zero on error.
   */
  int connectRemoteSharedMem(const char *shmFileName,
			     const char *addressPort,
			     const char *remoteName=NULL);

private:

  void clean_up(void);

  dtkTCPSocket *tcp;

  char buffer[DTKCLIENT_BUFF_SIZE+1];

  // server address and port.
  char  *connection;

};
