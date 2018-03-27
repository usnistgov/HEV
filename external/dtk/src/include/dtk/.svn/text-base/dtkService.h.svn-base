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
/* This file can be included with DSO codes that get loaded 
 * by the DTK server, `dtk-server'.
 */

#ifndef __DTK__SERVER__
#define __DTK__SERVER__


#ifdef DTK_ARCH_WIN32_VCPP
# ifdef DTK_SERVER_DSO_EXPORTS /* if building DLL library */
#  define DTKSERVERAPI __declspec(dllexport)
# else /* if using DLL library of the DTK server */
#  define DTKSERVERAPI __declspec(dllimport)
# endif
#else /* #ifdef DTK_ARCH_WIN32_VCPP *//* UNIX */
# define DTKSERVERAPI
#endif/* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#ifdef MULTI_THREADED
# ifdef DTK_ARCH_WIN32_VCPP
# else
#  include <pthread.h>
# endif
#endif


#ifndef DTKDSO_LOAD_ERROR
// The DTKDSO_*LOAD_* constants here are almost the same as in
// dtkDSO_loader.h

/***************** Used to return from dtkDSO_loader() *****************/

// Special dtkDSO_loader() return values.

// tell the loader to unload the DSO and fault due to error.
#define DTKDSO_LOAD_ERROR    ((dtkService *) 0)

// tell the loader to unload the DSO and continue.
#define DTKDSO_LOAD_UNLOAD   ((dtkService *) -1)


/***************** Used to return from dtkDSO_unloader() *****************/

// tell the loader to continue and not take any action.
#define DTKDSO_UNLOAD_CONTINUE  0

// tell the loader/unloader that the program is hosed and the
// dtk-server will exit.
#define DTKDSO_UNLOAD_ERROR    -1 /* error */

/***********************************************************************/
#endif /* #ifndef DTKDSO_LOAD_ERROR */


class DTKSERVERAPI ServerMutex
{
 public:
  ServerMutex(void);
  virtual ~ServerMutex(void);
  inline void lock(void) {
#ifdef MULTI_THREADED
# ifdef DTK_ARCH_WIN32_VCPP
    WaitForSingleObject(mutex,INFINITE);
# else
    pthread_mutex_lock(&mutex);
# endif
#endif
  }
  inline void unlock(void) {
#ifdef MULTI_THREADED
# ifdef DTK_ARCH_WIN32_VCPP
	ReleaseMutex(mutex);
# else
    pthread_mutex_unlock(&mutex);
# endif
#endif
  }

 private:

#ifdef MULTI_THREADED
# ifdef DTK_ARCH_WIN32_VCPP
  HANDLE mutex;
# else
  pthread_mutex_t mutex;
# endif
#endif
};

// struct ServiceManager_list is used to hold the list of services
// loaded used by class ServiceManager (below).
struct ServiceManager_list
{
  dtkService *service;
  void *loaderFunc;
  char *name;
  char *argv;
  char *file;
  // set if deleteOnRemove(service) was called.
  int deleteMe;
//  dtkCalibration *calibrator;
//  void *calibratorLoaderFunc;
#ifdef DTK_ARCH_WIN32_VCPP
  std::vector<HANDLE> fd_vector;
#else
  std::vector<int> fd_vector;
#endif
  struct ServiceManager_list *next;
  struct ServiceManager_list *prev;
};

class dtkConfigService;
class configParser;

class DTKSERVERAPI serviceConfig
{
public:
  friend class configParser;

  inline std::string getServiceType() { return m_service_type; };
  inline std::string getName() { return m_name; };
  inline bool getTrackd() { return m_trackd; };
  inline std::vector<std::string> getTrackdType() { return m_trackd_type; };
  inline std::vector<std::string> getShmName() { return m_shm_name; };
  inline std::map<std::string,int> getShmSize() { return m_shm_size; };
  inline std::string getPort() { return m_port; };
  inline std::string getBaud() { return m_baud; };
  inline int getBaudFlag() { return m_baud_flag; };
  inline std::vector<std::vector<std::string> > getConfig() { return m_config; };
  inline dtkCalibration* getCalibrator() { return m_calibrator; };
  inline std::vector<std::string> getCalibratorParams() { return m_calibrator_params; };
  inline std::string getCalibratorName() { return m_calibrator_name; };
  inline std::vector<std::string> getCalibrationConfigPath()
      { return m_calibration_config_path; };
  inline bool hasOptions() { return has_options; };

private:
  std::string m_service_type;
  std::string m_name;
  bool m_trackd;
  std::vector<std::string> m_trackd_type;
  std::vector<std::string> m_shm_name;
  std::map<std::string,int> m_shm_size;
  std::string m_port;
  std::string m_baud;
  int m_baud_flag;
  std::vector<std::vector<std::string> > m_config;
  dtkCalibration* m_calibrator;
  std::vector<std::string> m_calibrator_params;
  std::string m_calibrator_name;
  std::vector<std::string> m_calibration_config_path;
  bool has_options;
};

// Class to manage a list of dtkService objects for the DTK server.
// This ServiceManager class is thread safe.

class DTKSERVERAPI ServiceManager
{
 public:

  ServiceManager(void);
  virtual ~ServiceManager(void);

  // return 0 on success
  // return 1 on failure
  int add(dtkService *s, const char *name=NULL, const char *arg=NULL);

  // return 0 on success
  // return -2 on service is already loaded and return -1 on other error.
  int add(const char *filename, const char *name=NULL, const char *arg=NULL,
      bool is_configured =false, serviceConfig* config = NULL, const char* svc_path = NULL,
      const char* config_path = NULL, const char* cwd = NULL, const char* cal_path = NULL,
      const char* cal_conf_path = NULL );

  // return 0 on success
  // return -2 on any service is already loaded
  // return -1 on other error
  // This function is to specifically load configuration files
  int loadConfig( const char* config, const char* config_path, const char* cwd = NULL,
      const char* svc_path = NULL, const char* cal_path = NULL,
      const char* cal_conf_path = NULL );

  // Set it so that the delete s is call in remove()
  dtkService *deleteOnRemove(dtkService *s);

  // return 0 on success
  int remove(const char *name);
  int remove(dtkService *s);
  int removeAll(void);


  // return 2 if name=NULL and file is loaded
  // return 1 is there is a service named "name".
  // return 0 if not.
  int check(const char *file, const char *name);

  // return 1 is there is a service named "name".
  // return 0 if not.
  int check(const char *name);

  // returns NULL on failure.
  // returns a pointer to the dtkService.
  dtkService *get(const char *name);

  // returns NULL on failure.  Don't mess with the memory returned.
  char *getName(dtkService *s);

#ifdef DTK_ARCH_WIN32_VCPP
#else
  int loadFileDescriptors(fd_set *fds);
  dtkService *getNextService(fd_set *fds);
#endif

  // This is the largest loaded file descriptor.  Please don't write
  // to maxFDPlusOne.
  int maxFDPlusOne;

  //dtkService *getNext(dtkService *s=NULL);

 private:

  ServerMutex mutex;

  int _unload(dtkService *s, void *p);

  int _add(dtkService *s, char *name,
	   const char *arg, const char *file=NULL);

  int remove(struct ServiceManager_list *l);

  int check_service_name(const char *name, int verbose=1);

  int loadConfig( dtkConfigService* service, const char* file,
      const char* name, const char* config );

  int configureService( dtkConfigService* svc, serviceConfig* config );

  struct ServiceManager_list *first, *last;

  struct ServiceManager_list *current;

  char *servicePath;
  char *serviceConfigPath;
  char *calibrationPath;
  char *calibrationConfigPath;
};


extern DTKSERVERAPI ServiceManager serviceManager;

// Don't write to this memory.
extern DTKSERVERAPI char *serverPort;


#ifndef __NOT_LOADED_DTKSERVICE__


static dtkService *dtkDSO_loader(const char *ptr=NULL);
static int dtkDSO_unloader(dtkService *s);

extern "C"
{

#ifdef DTK_ARCH_WIN32_VCPP
  extern __declspec(dllexport) void *__dtkDSO_loader(void);
  extern __declspec(dllexport) void *__dtkDSO_unloader(void);
#endif

  void *__dtkDSO_loader(void)
  {
    return (void *) dtkDSO_loader;
  }
  void *__dtkDSO_unloader(void)
  {
    return (void *) dtkDSO_unloader;
  }
}


#endif /* #ifndef __NOT_LOADED_DTKSERVICE__ */

#endif /* #ifndef __DTK__SERVER__ */
