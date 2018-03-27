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
#include <sys/types.h>
#include <string.h>

#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
#  include <sys/time.h>
#  include <unistd.h>
#endif


#include "if_windows.h"
#include "serviceClientTCP.h"
#include "serviceSlaveTCP.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "configParser.h"
#include "dtk-server.h"
#include "threadManager.h"
#include <fstream>
#include <vector>
#include <string>
#include <map>

#ifdef DTK_ARCH_WIN32_VCPP
#  define SEPARATOR ";"
#else
#  define SEPARATOR ":"
#endif

static char *get_service_name_from_DSOFile(const char *DSOFile)
{
  int j = 0;
  for(int i=0;DSOFile[i] != '\0';i++)
    if(DSOFile[i] == '/')
      j = i+1;

  char *name = dtk_strdup(&DSOFile[j]); // the '/''s are gone

  // strip off the ".so" suffix
  size_t len = strlen(name);
  if(len > 3 &&
     name[len-1] == 'o' &&
     name[len-2] == 's' &&
     name[len-3] == '.')
    name[len-3] = '\0';

  return name;
}


ServiceManager::ServiceManager(void)
{
  current = first = last = NULL;
  maxFDPlusOne = 0;
  servicePath = get_service_path();
  serviceConfigPath = get_service_config_path();
  calibrationPath = get_calibration_path();
  calibrationConfigPath = get_calibration_config_path();
}


ServiceManager::~ServiceManager(void)
{
  // unload all services in the reverse order that they were loaded.
  while(last)
    remove(last);

  if(servicePath)
    {
      free(servicePath);
      servicePath = NULL;
    }
  if(serviceConfigPath)
    {
      free(serviceConfigPath);
      serviceConfigPath = NULL;
    }
  if(calibrationPath)
    {
      free(calibrationPath);
      calibrationPath = NULL;
    }
  if(calibrationConfigPath)
    {
      free(calibrationConfigPath);
      calibrationConfigPath = NULL;
    }
}

// name must come from malloc().
int ServiceManager::
_add(dtkService *service, char *name,
     const char *arg, const char *file)
{
  struct ServiceManager_list *m = first;
  struct ServiceManager_list *l = new ServiceManager_list;
  if( !l )
    dtkMsg.add(DTKMSG_ERROR, 1, "Unable to allocate ServiceManager_list.\n" );

//  struct ServiceManager_list *l =
//    (struct ServiceManager_list *)
//    dtk_malloc(sizeof(struct ServiceManager_list));

  l->service = service;
  l->loaderFunc = NULL;
  l->name = name;
  l->file = NULL;
  l->argv = NULL;
  l->next = NULL;
  l->prev = NULL;
  l->deleteMe = 0;
//  l->calibrator = NULL;

  if(file && file[0])
    l->file = dtk_strdup(file);

  // check service file descriptor
#ifdef DTK_ARCH_WIN32_VCPP
  if(!(l->service) || ( !(l->service->fd) && !(l->service->fd_vector.size()) ) )
#else
  if(!(l->service) || ( l->service->fd < 0 && !(l->service->fd_vector.size()) ) || 
    l->service->fd > FD_SETSIZE)
#endif
  {
    dtkMsg.add(DTKMSG_WARN,
        "The service%s%s, named \"%s%s%s\"\n"
        "has a file descriptor (%s%d%s) "
        "that is out of range.\n"
        "  The range of valid file descriptors"
        " is from %d to %d for this OS.\n",
        file?", from: file ":"", file?file:"",
        dtkMsg.color.red, name, dtkMsg.color.end,
        dtkMsg.color.tur, l->service->fd,
        dtkMsg.color.end,
        0, FD_SETSIZE);
    goto addService_error;
  }

  // check file descriptor with loaded services
  for(;m;m=m->next)
  {
#ifdef DTK_ARCH_WIN32_VCPP
    if(m->service->fd == l->service->fd && l->service->fd != NULL)
#else
    if(m->service->fd == l->service->fd && l->service->fd != -1)
#endif
    {
      dtkMsg.add(DTKMSG_WARN,
           "The service%s%s, named \"%s%s%s\"\n"
           "has a file descriptor (%s%d%s) "
           "that which is already in use by the"
           " loaded service named \"%s\"\n",
           file?", from file: ":"", file?file:"",
           dtkMsg.color.red, name, dtkMsg.color.end,
           dtkMsg.color.tur, l->service->fd,
           dtkMsg.color.end,
           m->name);
      goto addService_error;
    }
    else
    {
      for( unsigned int i=0;i<l->service->fd_vector.size();i++ )
      {
        for( unsigned int j=0;j<m->service->fd_vector.size();j++ )
        {
          if( l->service->fd_vector[i] == m->service->fd_vector[j] )
          {
            dtkMsg.add(DTKMSG_WARN,
               "The service%s%s, named \"%s%s%s\"\n"
               "has a file descriptor (%s%d%s) "
               "that which is already in use by the"
               " loaded service named \"%s\"\n",
               file?", from file: ":"", file?file:"",
               dtkMsg.color.red, name, dtkMsg.color.end,
               dtkMsg.color.tur, l->service->fd,
               dtkMsg.color.end,
               m->name);
            goto addService_error;
          }
        }
      }
    }
  }

  // Fill in the rest of the list data
  if(arg && arg[0])
    l->argv = dtk_strdup(arg);

  if( l->service->fd_vector.size() )
    l->fd_vector = l->service->fd_vector;
#ifdef DTK_ARCH_WIN32_VCPP
  if( l->service->fd != NULL )
#else
  if( l->service->fd > -1 )
#endif
    l->fd_vector.push_back( l->service->fd );

  // Put this service at the end of the list.
  if(!first)
  {
    first = last = l;
  }
  else
  {
    last->next = l;
    l->prev = last;
    last = l;
  }

#ifndef DTK_ARCH_WIN32_VCPP
  for( unsigned int i=0;i<l->fd_vector.size();i++ )
  {
    if(maxFDPlusOne < l->fd_vector[i] + 1)
      maxFDPlusOne = l->fd_vector[i] + 1;
  }
#endif

  char msg[1024];
  sprintf( msg, "The service%s%s%s, named \"%s%s%s\",\n"
      " was loaded with file descriptor%s",
      file?", from file: \"":"", file?file:"", file?"\"":"",
      dtkMsg.color.red, name, dtkMsg.color.end,
      l->fd_vector.size()>1?"s ":" " );
  for( unsigned int i=0;i<l->fd_vector.size();i++ )
  {
    sprintf( msg, "%s%s%d%s", msg, i==0?dtkMsg.color.tur:"",
        l->fd_vector[i], i==l->fd_vector.size()-1?dtkMsg.color.end:", " );
  }
  dtkMsg.add(DTKMSG_INFO,
         "%s%s%s.\n", msg,
         (l->argv)?" and argument=":"",
         (l->argv)?(l->argv):"" );

  return 0; // success

addService_error:

  if(l)
  {
    if(l->file)
      free(l->file);
    if(l->name)
      free(l->name);
    if(l->argv)
      free(l->argv);
//    free(l);
    delete l;
    l = NULL;
  }

  return -1;
}

// This gets called by ThreadManager.cpp's thread_routine().
char *ServiceManager::getName(dtkService *s)
{
  struct ServiceManager_list *m;
  for(m=first;m;m=m->next)
  {
    if(s == m->service)
      return m->name;
  }
  return NULL;
}
  

int ServiceManager::
check_service_name(const char *name, int verbose)
{
  // check the service name
  if(!name || !(name[0]))
  {
    if(verbose)
      dtkMsg.add(DTKMSG_WARN,
          "The service name \"%s\" is invalid.\n",
          name);
    return -1; // error
  }

  struct ServiceManager_list *m;
  for(m=first;m;m=m->next)
    if(!strcmp(name, m->name))
    {
      if(verbose)
        dtkMsg.add(DTKMSG_WARN,
            "Already loaded is a service named %s%s%s.\n",
            dtkMsg.color.grn, name, dtkMsg.color.end);
      return -1; // error
    }

  return 0; // success
}

// This will generate a name for the service if name is NULL.
int ServiceManager::
add(dtkService *s, const char *name, const char *arg)
{
  mutex.lock();
  // get the service name
  static int nameNum = 0;
  if(!s)
  {
    mutex.unlock();
    return
        dtkMsg.add(DTKMSG_WARN, 0, -1, 
            "can't load a NULL dtkSservice object.\n");
  }

  if(name && name[0] && check_service_name(name))
  {
    mutex.unlock();
    return -1;
  }
  else if(name && name[0])
    name = dtk_strdup(name);
  else
  {
    char str[] = "service-------------------";
    sprintf(&(str[7]),"%d",nameNum++);
    while(check_service_name(str,0))
      sprintf(&(str[7]),"%d",nameNum++);

    name = dtk_strdup(str);
  }
  int i = _add(s, (char *) name, arg);

#ifdef MULTI_THREADED

  if(!i && threadManager.add(s))
  {
    mutex.unlock();
    remove(s);
    return i;
  }
#endif

  mutex.unlock();
  return i;
}


// return 2 if name is not set and file is loaded
// return 1 is there is a service named "name".
// return 0 if not.
int ServiceManager::
check(const char *file, const char *name)
{
  mutex.lock();
  struct ServiceManager_list *m;
  if(name && name[0])
  {
    for(m=first;m;m=m->next)
    {
      if(!strcmp(m->name,name))
      {
        mutex.unlock();
        return 1;
      }
    }
    mutex.unlock();
    return 0;
  }
  if(file && file[0])
  {
    name = get_service_name_from_DSOFile(file);
    for(m=first;m;m=m->next)
      if(!strcmp(m->name,name))
      {
        free((void *) name);
        mutex.unlock();
        return 2;
      }
      free((void *) name);
  }
  mutex.unlock();
  return 0;
}

// return 1 is there is a service named "name".
// return 0 if not.
int ServiceManager::
check(const char *name)
{
  mutex.lock();
  struct ServiceManager_list *m;
  if(name && name[0])
  {
    for(m=first;m;m=m->next)
      if(!strcmp(m->name,name))
      {
        mutex.unlock();
        return 1;
      }
  }
  mutex.unlock();
  return 0;
}

dtkService *ServiceManager::
get(const char *name)
{
  mutex.lock();
  struct ServiceManager_list *m;
  if(name && name[0])
  {
    for(m=first;m;m=m->next)
      if(!strcmp(m->name,name))
      {
        mutex.unlock();
        return m->service;
      }
  }
  mutex.unlock();
  return NULL;
}


dtkService *ServiceManager::
deleteOnRemove(dtkService *s)
{
  mutex.lock();
  struct ServiceManager_list *m;
  if(s)
  {
    for(m=first;m;m=m->next)
      if(s == m->service)
      {
        m->deleteMe = 1;
        mutex.unlock();
        return m->service;
      }
  }
  mutex.unlock();
  return NULL;
}

int ServiceManager::
add(const char *file, const char *name, const char *arg,
    bool is_configured, serviceConfig* config, const char* svc_path,
      const char* config_path, const char* cwd, const char* cal_path,
      const char* cal_conf_path )
{
  mutex.lock();
  // get the service name
  if(name && name[0])
    name = dtk_strdup(name);
  else
    name = get_service_name_from_DSOFile(file);

  if(check_service_name(name))
  {
    free((void *) name);
    mutex.unlock();
    return -2;
  }

  std::string all_path;
  std::vector<std::string> paths;
  if( svc_path )
    paths.push_back( svc_path );
  if( cwd )
    paths.push_back( cwd );
  paths.push_back( servicePath );
  for( unsigned int i=0;i<paths.size();i++ )
  {
    all_path += paths[i];
    if( i < paths.size() - 1 )
      all_path += SEPARATOR;
  }

  // Load DSO
  dtkService *(*loaderFunc)(const char *ptr) =
      (dtkService *(*)(const char *))
      dtkDSO_load(all_path.c_str(), file, arg);

  if(!loaderFunc)
  {
    dtkMsg.add(DTKMSG_WARN,
        "ServiceManager::add() failed to load DSO file"
        " \"%s\".\n", file);
    free((void *) name);
    mutex.unlock();
    return -1;
  }

  // loaderFunc may call a ServiceManager method.
  mutex.unlock();
  dtkService *service = loaderFunc(arg);
  mutex.lock();

  if(service == DTKDSO_LOAD_ERROR)
  {
    dtkMsg.add(DTKMSG_WARN,
        "ServiceManager::add() failed: DSO file \"%s\"\n"
        " entry function returned an error code.\n", file);
    mutex.unlock();
    _unload(service, (void *) loaderFunc);
    free((void *) name);
    return -1; // error
  }

  if(service == DTKDSO_LOAD_UNLOAD)
  {
    dtkMsg.add(DTKMSG_DEBUG,
        "ServiceManager::add(): DSO file \"%s\"\n"
        " entry function returned telling to unload DSO.\n",
        file);
    free((void *) name);
    mutex.unlock();
    int i = _unload(service, (void *) loaderFunc);
    return i;
  }

  if( !is_configured && dynamic_cast<dtkConfigService*> ( service ) )
  {
    if( loadConfig( dynamic_cast<dtkConfigService*> ( service ), file, name, arg ) )
    {
      free((void *) name);
      mutex.unlock();
      _unload(service, (void *) loaderFunc);
      return -1;
    }
  }

  if( config && dynamic_cast<dtkConfigService*> ( service ) )
  {
    dtkConfigService* config_svc = dynamic_cast<dtkConfigService*> ( service );
    if( configureService( config_svc, config ) )
    {
      free((void *) name);
      mutex.unlock();
      _unload(config_svc, (void *) loaderFunc);
      return -1;
    }

    paths.clear();
    if( cal_conf_path )
      paths.push_back( cal_conf_path );
    if( cwd )
      paths.push_back( cwd );
    if( config_path )
      paths.push_back( config_path );
    if( cal_path )
      paths.push_back( cal_path );
    if( svc_path )
      paths.push_back( svc_path );
    paths.push_back( calibrationConfigPath );
	if( std::string( calibrationConfigPath ) != std::string( serviceConfigPath ) )
      paths.push_back( serviceConfigPath );
	if( std::string( calibrationConfigPath ) != std::string( calibrationPath ) )
      paths.push_back( calibrationPath );
	if( std::string( calibrationConfigPath ) != std::string( servicePath ) )
      paths.push_back( servicePath );
    for( unsigned int i=0;i<paths.size();i++ )
    {
      all_path += paths[i];
      if( i < paths.size() - 1 )
        all_path += SEPARATOR;
    }
    config_svc->m_calibration_config_path = all_path;

    std::string calibrator_name = config_svc->getCalibratorName();
    if( calibrator_name.size() )
    {
      paths.clear();
      all_path.clear();
      if( cal_path )
        paths.push_back( cal_path );
      if( cwd )
        paths.push_back( cwd );
      if( svc_path )
        paths.push_back( svc_path );
      paths.push_back( calibrationPath );
      paths.push_back( servicePath );
      for( unsigned int i=0;i<paths.size();i++ )
      {
        all_path += paths[i];
        if( i < paths.size() - 1 )
          all_path += SEPARATOR;
      }
      // Load DSO
      dtkCalibration *(*calibratorLoaderFunc)(const char *ptr) =
          (dtkCalibration *(*)(const char *))
          dtkDSO_load(all_path.c_str(), calibrator_name.c_str(), "");
      if(!calibratorLoaderFunc)
      {
        dtkMsg.add(DTKMSG_WARN,
            "ServiceManager::add() failed to load dtkCalibration DSO file"
            " \"%s\".\n", calibrator_name.c_str());
        free((void *) name);
        _unload(service, (void *) loaderFunc);
        mutex.unlock();
        return -1;
      }
      config_svc->m_calibratorLoaderFunc = (void*)calibratorLoaderFunc;

      // loaderFunc may call a ServiceManager method.
      mutex.unlock();
      config_svc->m_calibrator = calibratorLoaderFunc(arg);
      mutex.lock();

      if( !config_svc->getCalibrator() )
      {
        dtkMsg.add(DTKMSG_WARN,
            "ServiceManager::add() failed: dtkCalibration DSO file \"%s\"\n"
            " entry function returned an error code.\n", file);
        mutex.unlock();
        _unload(service, (void *) loaderFunc);
        free((void *) name);
        return -1; // error
      }
    }
  }

  int service_init = service->init();
  if( service_init == DTKSERVICE_UNLOAD )
  {
    dtkMsg.add(DTKMSG_DEBUG,
        "ServiceManager::add(): DSO file \"%s\"\n"
        " init() function returned telling to unload DSO.\n",
        file);
    free((void *) name);
    mutex.unlock();
    int i = _unload(service, (void *) loaderFunc);
    return i;
  }
  else if( service_init == DTKSERVICE_ERROR )
  {
    dtkMsg.add(DTKMSG_DEBUG,
        "ServiceManager::add(): DSO file \"%s\"\n"
        " init() function returned an error code.\n",
        file);
    free((void *) name);
    mutex.unlock();
    _unload(service, (void *) loaderFunc);
    return -1;
  }

  if(_add(service, (char *) name, arg, file))
    {
      dtkMsg.add(DTKMSG_WARN,
         "ServiceManager::add() failed to load DSO file"
         " \"%s\".\n",file);
      _unload(service, (void *) loaderFunc);
      free((void *) name);
      mutex.unlock();
      return -1;
    }

  last->loaderFunc = (void *) loaderFunc;

#ifdef MULTI_THREADED
  /******* add the service thread ******/
  if(threadManager.add(service))
    {
      mutex.unlock();
      remove(service);
      return -1;
    }
#endif

  mutex.unlock();
  return 0; // success
}

// Does not remove it from the service list.  Just calls the C++
// unloader function dtkDSO_unloader(dtkService *s) and unloads the
// DSO file.
int ServiceManager::_unload(dtkService *s, void *p)
{
  int (*unloaderFunc)(dtkService *s) = 
    (int (*)(dtkService *)) dtkDSO_getUnloader(p);

  if(!unloaderFunc)
  {
    dtkMsg.add(DTKMSG_WARN,
        "dtkManager::load(): unloading "
        "DSO file failed to get "
        "unloader function.\n");
    dtkDSO_unload(p);
      return -1; // error
  }

  if(unloaderFunc(s) == DTKDSO_UNLOAD_ERROR)
  {
    dtkMsg.add(DTKMSG_WARN,
        "dtkManager::load(): unloading DSO file "
        "failed: unloader function failed.\n");
    dtkDSO_unload(p);
    return -1; // error
  }

  // dtkDSO_unload() returns -1 on error or 0 on success
  return dtkDSO_unload(p); 
}

int ServiceManager::remove(const char *name)
{
  mutex.lock();
  struct ServiceManager_list *m;
  for(m=first;m;m=m->next)
    if(!strcmp(name, m->name))
    {
      int i = remove(m);
      mutex.unlock();
      return i;
    }
  mutex.unlock();
  return -1;
}


int ServiceManager::remove(dtkService *s)
{
  mutex.lock();
  struct ServiceManager_list *m;
  for(m=first;m;m=m->next)
    if(s == m->service)
    {
      int i = remove(m);
      mutex.unlock();
      return i;
    }
  mutex.unlock();
  return -1;
}

int ServiceManager::removeAll(void)
{
  mutex.lock();

  int ret = 0;
  // unload all services in the reverse order that they were loaded.
  while(last)
    ret += remove(last);

  mutex.unlock();
  return ret;
}

int ServiceManager::remove(struct ServiceManager_list *l)
{
#ifdef MULTI_THREADED
  threadManager.remove(l->service);
#endif

  int returnVal = 0;

  // If removing the current service than make the current service by
  // the next service.
  if(l == current)
    current = current->next;

  // fix the list
  if(l->prev)
    l->prev->next = l->next;
  else
    first = l->next;

  if(l->next)
    l->next->prev = l->prev;
  else
    last = l->prev;

  if(l->service)
  {
    if(l->loaderFunc)
    {
      if(_unload(l->service, l->loaderFunc))
        returnVal = -1;
    }
    else if(l->deleteMe)
    {
      // If this is one of the built in server connected TCP 
      // dtkServices, than delete it here.
      delete l->service;
    }
  }

#ifndef DTK_ARCH_WIN32_VCPP
  for( unsigned int i=0;i<l->fd_vector.size();i++ )
  {
    if(maxFDPlusOne == l->fd_vector[i] + 1)
    {
      maxFDPlusOne = 0;
      struct ServiceManager_list *m = last;
      for(;m;m=m->prev)
        for( unsigned int j=0;j<m->fd_vector.size();j++ )
          if(maxFDPlusOne < m->fd_vector[j] + 1)
            maxFDPlusOne = m->fd_vector[j] + 1;
    }
  }
#endif

  dtkMsg.add(DTKMSG_INFO,
      "The service%s%s, named \"%s%s%s\" was "
      "unloaded.\n",
      (l->file)?", from file: ":"", (l->file)?(l->file):"",
      dtkMsg.color.red, l->name, dtkMsg.color.end);

  if(l->name)
    free(l->name);
  if(l->file)
    free(l->file);
  if(l->argv)
    free(l->argv);

//  free(l);

  return returnVal;
}

int ServiceManager::configureService( dtkConfigService* svc, serviceConfig* config )
{
  if( !svc )
  {
    dtkMsg.add( DTKMSG_ERROR, "Unable to configure the service -"
        "the dtkConfigService is NULL\n" );
    return -1;
  }
  else if( !config )
  {
    dtkMsg.add( DTKMSG_ERROR, "Unable to configure the service -"
        "the serviceConfig is NULL\n" );
    return -1;
  }

  svc->m_service_type = config->getServiceType();
  svc->m_name = config->getName();
  svc->m_trackd = config->getTrackd();
  svc->m_trackd_type = config->getTrackdType();
  svc->m_shm_name = config->getShmName();
  svc->m_shm_size = config->getShmSize();
  svc->m_port = config->getPort();
  svc->m_baud = config->getBaud();
  svc->m_baud_flag = config->getBaudFlag();
  svc->m_config = config->getConfig();
  svc->m_calibrator_params = config->getCalibratorParams();
  svc->m_calibrator_name = config->getCalibratorName();

  return 0;
}

int ServiceManager::loadConfig( const char* config, const char* config_path, const char* cwd,
    const char* svc_path, const char* cal_path, const char* cal_conf_path )
{
  configParser parser;
  std::string all_path;
  if( config_path )
  {
    if( all_path.size() )
      all_path += SEPARATOR;
    all_path += config_path;
  }
  if( cwd )
  {
    if( all_path.size() )
      all_path += SEPARATOR;
    all_path += cwd;
  }
  if( svc_path )
  {
    if( all_path.size() )
      all_path += SEPARATOR;
    all_path += svc_path;
  }
  if( all_path.size() )
    all_path += SEPARATOR;
  all_path += serviceConfigPath;
  if( std::string( serviceConfigPath ) != std::string( servicePath ) )
  {
    all_path += SEPARATOR;
    all_path += servicePath;
  }

  if( parser.parse( config, all_path.c_str() ) )
    return -1;

//  char* save_servicePath = servicePath;
//  servicePath = (char *) dtk_malloc(strlen(path) + strlen(".:") + 1);
//  sprintf(servicePath,".:%s", path);

  std::map<std::string, serviceConfig> configuration = parser.getConfiguration();
  std::map<std::string, serviceConfig>::iterator itr = configuration.begin();
  for( ;itr!=configuration.end();itr++ )
  {
    if( add( itr->second.getServiceType().c_str(),
        itr->second.getName().c_str(), "", true, &(itr->second ), svc_path, config_path,
        cwd, cal_path, cal_conf_path ) )
    {
      dtkMsg.add( DTKMSG_ERROR, "ServiceManager::loadConfig - failed to add\n"
          "%s service from config file\n", itr->second.getServiceType().c_str() );
      return -1;;
    }
  }
//  free( servicePath );
//  servicePath = save_servicePath;
  return 0;
}

int ServiceManager::
loadConfig( dtkConfigService* service, const char* file, const char* name, const char* config )
{
  configParser parser;
  if( parser.parse( config, servicePath ) )
    return -1;

  std::map<std::string, serviceConfig> configuration = parser.getConfiguration();

  if( configuration[name].getName() != std::string( name ) )
  {
    dtkMsg.add( DTKMSG_ERROR, "Could not find a service named %s\n"
        "within the configuration file.\n", name );
    return -1;
  }
  else
    configureService( service, &configuration[name] );

  std::map<std::string, serviceConfig>::iterator itr = configuration.begin();
  for( ;itr!=configuration.end();itr++ )
  {
    if( add( itr->second.getServiceType().c_str(),
        itr->second.getName().c_str(), "", true, &(itr->second ) ) )
    {
      dtkMsg.add( DTKMSG_ERROR, "ServiceManager::loadConfig - failed to add\n"
          "%s service from config file\n", itr->second.getServiceType().c_str() );
      return -1;
    }
  }

  return 0;
}



#ifdef DTK_ARCH_WIN32_VCPP

#else /* #ifdef DTK_ARCH_WIN32_VCPP */

int ServiceManager::loadFileDescriptors(fd_set *fds)
{
  if(!first)
  {
    return 1;
  }

  FD_ZERO(fds);
  for(current=first;current;current=current->next)
  {
    for( unsigned int i=0;i<current->fd_vector.size();i++ )
      FD_SET(current->fd_vector[i], fds);
  }

  current = first;

  return 0;
}


dtkService *ServiceManager::getNextService(fd_set *fds)
{
  bool is_set = false;
  for(;current;current=current->next)
  {
    is_set = false;
    dtkService *s = current->service;
    s->fd_set_vector.clear();
    for( unsigned int i=0;i<current->fd_vector.size();i++ )
    {
      if(FD_ISSET(current->fd_vector[i], fds))
      {
        is_set = true;
        s->fd_set_vector.push_back( current->fd_vector[i] );
      }
    }
    if( is_set )
    {
      current=current->next;
      return s;
    }
  }
  current = NULL;

  return NULL;
}

#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

