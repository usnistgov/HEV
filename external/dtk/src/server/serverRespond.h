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
// Since we do not want to make a new dtkSharedMem object for each
// request we need to keep a list of all the dtkSharedMem objects used.
// This assumes that the DTK server is a single thread.

#define RESPONSE_BUF_SIZE   ((size_t) 2048)

struct sharedMemList
{
  dtkSharedMem *sharedMem;
  struct sharedMemList *next;
};


class DTKSERVERAPI ServerRespond
{

 public:

  ServerRespond(void);
  ~ServerRespond(void);

  // writing to the a shared memory file from the network.
  char *writeSharedMem(size_t *size_out,char request_type,
		     void *data, size_t size,
		     const char *name, int byte_order,
		     const char *fromClientAddr);

  char *loadService(size_t *size_out,
		    const char *file, const char *name, const char *arg);

  char *loadConfig(size_t *size_out, const char *config_file, const char* path,
      const char* cwd, const char* svc_path, const char* cal_path,
      const char* cal_conf_path);

  char *unloadService(size_t *size_out,
		      const char *name);
  char *resetService(size_t *size_out,
		     const char *name);
  char *checkService(size_t *size_out,
		     const char *name);

  // (0) from client to server
  char *connectServer(size_t *size_out, const char *serverAddressPort,
		      const char *fromClientAddr);

  // (1) from server to server
  char *connectServerFail(size_t *size_out, const char *addressPort,
			  const char *fromClientAddr);

  // (2) from server to server back to client reply.
  char *connectServer(size_t *size_out, const char *addressPort,
		      const char *fromClientAddr, const char *errorStr);


  // Remotename refers to name on other system.
  // (0) From client to server
  char *connectSharedMem(size_t *size_out,
		       const char *name,
		       const char *addressPort,
		       const char *remoteName,
		       const char *fromClientAddrPort);
  // (1) From server to server
  char *connectSharedMem(size_t *size_out,
		       size_t seg_size,
		       const char *name,
		       const char *addressPort,
		       const char *remoteName,
		       const char *fromClientAddrPort);
  
  // (2) From server to server with reply to client.
  char *connectSharedMem(size_t *size_out,
		       const char *name,
		       const char *addressPort,
		       const char *remoteName,
		       const char *fromClientAddrPort,
		       const char *errorStr);

  char *sharedMemWriteList(size_t *size_out, const char *segName);

  // This is the one thread safe method in this class.
  int removeAddressFromWroteLists(const char *addr);
  
 private:

  // get sharedMem from a local list, so we don't make
  // many copies of dtkSharedMems.
  dtkSharedMem *getSharedMem(size_t size, const char *name);
  dtkSharedMem *getSharedMem(const char *name);

  struct sharedMemList *shmList;

  char *fail(size_t *size, const char *s=NULL, ...);
  char *success(size_t *size, const char *s=NULL, ...);

  // just a buffer for the string response
  char response[RESPONSE_BUF_SIZE+2]; 
  char *response1;   // points to reponse[1]

  ServerMutex removeAddressFromWroteListsMutex;
};

