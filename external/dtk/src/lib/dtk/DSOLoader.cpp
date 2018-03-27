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
/* This file was written by Lance Arsenault.  Chris Logie contributed
 * a lot of the brains needed for this code.
 */

// There can be only one instance of the DSO Loader class because
// having more than one would not add any functionality and would make
// things very confusing.  With one Loader
// object I can make two instances of one service by keeping track of
// the objects that have been loaded.  Since there can be only one
// instance of the DSO Loader class I make it just a function to
// enforce that.

// The env variable LD_LIBRARY_PATH (and LD_LIBRARY*_PATH on IRIX) is
// the only way to vary the search path for dlopen().  To get around
// this limitation we use compute full paths.

// I have set it up so that you can call dtkDSO_load() within
// dtkDSO_load() and so on.  But you cannot call dtkDSO_unload() in
// the object that came from a call to dtkDSO_load().  That would be
// proccess suicide.

#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#ifdef DTK_ARCH_WIN32_VCPP
# include  <io.h>
# include <malloc.h>
# include <direct.h>
# include <winbase.h>
#else
# include <unistd.h>
# include <dlfcn.h>
# include <fcntl.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "utils.h"
#include "dtkMessage.h"
#include "dtkDSO.h"

//#define DLOPEN_MODE  (RTLD_LAZY | RTLD_GLOBAL)
//#define   DLOPEN_MODE  (RTLD_NOW | RTLD_LOCAL)
#ifdef DTK_ARCH_WIN32_VCPP
# define DLOPEN_MODE  (0)
# define SEPARATOR ';'
#else
# define DLOPEN_MODE  (RTLD_LAZY | RTLD_GLOBAL)
# define SEPARATOR ':'
#endif
//#define   DLOPEN_MODE  (RTLD_NOW | RTLD_GLOBAL)

#define LOADER_FUNCTION     "__dtkDSO_loader"
#define UNLOADER_FUNCTION   "__dtkDSO_unloader"

// I save a record of the DSO's that are loaded in a struct DSO_List
// so that if you load 
struct DSO_List
{
  void *func;         // pointer to function called that is loaded
  char *DSOFilename;  // File name of DSO. Ends in ".so" or ".dll" on Windoz.
#ifdef DTK_ARCH_WIN32_VCPP
  HINSTANCE DSO_handle; // returned by LoadLibrary()
#else
  void *DSO_handle;      // returned from dlopen()
#endif
  void *object;       // pointer to object that was created
  struct DSO_List *next;
  struct DSO_List *prev;
};


static struct DSO_List *list_top = NULL, *list_bottom = NULL;
static int count = 0;


#ifdef DTK_ARCH_WIN32_VCPP

#define LDOPEN_NAME  "LoadLibrary"
#define LDSYM_NAME   "GetProcAddress"

// I make windoz LoadLibrary() and family of functions look and
// act like UNIX's dlopen() and family of functions.

static char dlerrstr[64] = "0";
static int dlerrnum = 0;

inline char *dlerror(void)
{
	if(dlerrnum)
	{
		dlerrnum = 0;
		return dlerrstr;
	}
	else
	{
	    return NULL;
	}
}


inline HINSTANCE dlopen(const char *file, int foo=0)
{
     HINSTANCE handle = LoadLibrary(file);
	 if(handle)
	 {
		 SetLastError(0);
         dlerrnum = 0;
	 }
	 else // error
	 {
        dlerrnum = GetLastError();
		if(!dlerrnum) dlerrnum = 0x7fffffff; // make sure it's non-zero
		// This error string leaves something to be desired.
		sprintf(dlerrstr,"Windoz GetLastError()=%d",dlerrnum);
	 }
	return handle;
}


inline void *dlsym(HINSTANCE handle, const char *str)
{
	void *ptr = GetProcAddress(handle, str);
	if(ptr)
	{
	    SetLastError(0);
        dlerrnum = 0;
	}
	else // error
	{
        dlerrnum = GetLastError();
		if(!dlerrnum) dlerrnum = 0x7fffffff; // make sure it's non-zero
		// This error string leaves something to be desired.
		sprintf(dlerrstr,"Windoz GetLastError()=%d",dlerrnum);
	}
	return ptr;
}


// return 1 on finding it.
// returns 0 if not found.
static int check_file_read(const char *file)
{
   if(_access(file, 00) == -1 || // file doesn't exist
	  _access(file, 04) == -1)  // can't read file
	{
		dtkMsg.add(DTKMSG_NOTICE, 1,
			"dtkDSO_load() failed: "
			"can't read file \"%s\".\n",
			file);
		return 0;
	}
	return 1;
}

#else /* #ifdef DTK_ARCH_WIN32_VCPP */


#define LDOPEN_NAME  "ldopen"
#define LDSYM_NAME   "ldsym"

// return 1 on finding it.
// returns 0 if not found.
static int check_file_exec(const char *file, uid_t uid, gid_t gid)
{
#if(1)
  dtkMsg.add(DTKMSG_DEBUG,
	     "dtkDSO_load(): Trying \"%s\".\n",
	     file);
#endif
  struct stat Stat;

  if(stat(file, &Stat))
    {
      if(EACCES == errno)
	dtkMsg.add(DTKMSG_NOTICE, 1,
		   "dtkDSO_load() failed: "
		   "stat(\"%s\",&Stat) failed: "
		   "looks like user with id %d does not"
		   " have access to this to file \"%s\".\n",
		   file, uid, file);
      return 0;
    }

  if(// owner is you and owner has execute permission
     (uid == Stat.st_uid && Stat.st_mode & S_IXUSR) ||
     // group is you and group has execute permission
     (gid == Stat.st_uid && Stat.st_mode & S_IXGRP) ||
     //  you as other has execute permission
     (Stat.st_mode & S_IXOTH)
     )
    return 1;
  return 0;
}
#endif /* #else #ifdef DTK_ARCH_WIN32_VCPP */



#ifdef DTK_ARCH_WIN32_VCPP

#define IS_FULL_PATH(x)   ((x)[0] == '\\' || (strlen(x) >= 2 && (x)[1] == ':'))
						   

static char *find_file(const char *path_in, const char *file_in)
{
  if(file_in == NULL) return NULL; // return NULL

  // Add the suffix ".dll" to file_in if needed.
  char *file = (char *) dtk_malloc(strlen(file_in) + 5);

  size_t S = strlen(file_in);
  if(S >= ((size_t) 4) &&
     file_in[S - 1] == 'l' &&
     file_in[S - 2] == 'l' &&
     file_in[S - 3] == 'd' &&
     file_in[S - 4] == '.')
    strcpy(file,file_in);
  else
    sprintf(file,"%s.dll",file_in);

  // If file is full path.
  if(IS_FULL_PATH(file))
    {
      if(check_file_read(file))
	    return file;
      else
	  {
	    free(file);
	    return NULL;
	  }
    }

  char *cwd = _getcwd(NULL,1);

  if(cwd == NULL)
    {
#if(1)
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkDSO_load() failed: "
		 "_getcwd() failed.\n");
#endif
      errno = 0;
      cwd = (char *) dtk_malloc(1);
      cwd[0] = '\0';
    }

  if(path_in && path_in[0] != '\0')
    {
      char *path = dtk_strdup(path_in);
      char *dir = path;
      char *next_dir = dir;
      
      // Get a big enough string to hold whatever the found file could be.
      char *str = (char *) dtk_malloc(strlen(path) + strlen(cwd) + 
				  strlen(file) + 3);

      //printf("file=%s line=%d path=\"%s\"\n",__FILE__,__LINE__, path);

      // search the path
      while(dir[0] != '\0')
	  {
	  // Sift for the next dir in the path.
	  for(;*next_dir != ';' && *next_dir != '\0';next_dir++);
	  
	  while(*next_dir == ';')
	    {
	      *next_dir = '\0';
	      next_dir++;
	    }

	  // Strip off extra \ from the end of dir
	  size_t len = strlen(dir);
	  while(len && dir[len-1] == '\\')
	  {
	    dir[len-1] = '\0';
		len = strlen(dir);
	  }

	  *str = '\0';
	  if(!strcmp(".",dir))     // current working dir
	    sprintf(str,"%s\\%s",cwd,file);
	  else if(IS_FULL_PATH(dir)) // full path
	    sprintf(str,"%s\\%s",dir,file);
	  else if(*dir)            // relative path
	    sprintf(str,"%s\\%s\\%s",cwd,dir,file);

	  if(*str && check_file_read(str))
	    {
	      free(path);
	      free(cwd);
	      free(file);
	      return str;
	    }
	  dir = next_dir;
	}
      free(str);
      free(path);
    }

  // Check in current working directory if path is not set.
  else // path == NULL
    {
      // Get the current working directory.
      char *str = (char *) dtk_malloc(strlen(cwd) + strlen(file) + 2);
 
      // look in the current working directory.
      sprintf(str,"%s\\%s",cwd,file);
      if(check_file_read(str))
	  {
	   free(cwd);
	   free(file);
	   return str;
	  }
      free(str);
    }

  free(cwd);
  free(file);
  return NULL;   
}


#else  /* #ifdef DTK_ARCH_WIN32_VCPP */
// Find a file that is the first executable that is in path.  You must
// free() after.  This will resolve "." as the current working
// directory.

static char *find_file(const char *path_in, const char *file_in)
{
  if(file_in == NULL) return NULL; // return NULL

  // Add the suffix ".so" to file_in if needed.
  char *file = (char *) dtk_malloc(strlen(file_in) + 4);

  size_t S = strlen(file_in);
  if(S >= ((size_t) 3) &&
     file_in[S - 1] == 'o' &&
     file_in[S - 2] == 's' &&
     file_in[S - 3] == '.')
    strcpy(file,file_in);
  else
    sprintf(file,"%s.so",file_in);  

  uid_t uid = geteuid();
  gid_t gid = getegid();

  // If file is full path.
  if(file[0] == '/')
    {
      if(check_file_exec(file, uid, gid))
	{
	  return file;
	}
      else
	{
	  free(file);
	  return NULL;
	}
    }

#ifdef DTK_ARCH_IRIX
  char *cwd = getcwd(NULL, (size_t) -1);
#else /* not IRIX OS */
  char *cwd = getcwd(NULL, (size_t) 0);
#endif

  if(cwd == NULL)
    {
#if(1)
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkDSO_load() failed: "
		 "getcwd() failed.\n");
#endif
      errno = 0;
      cwd = (char *) dtk_malloc(1);
      cwd[0] = '\0';
    }

  if(!strcmp("/",cwd))
    cwd[0] = '\0';

  if(path_in && path_in[0] != '\0')
    {
      char *path = dtk_strdup(path_in);
      char *dir = path;
      char *next_dir = dir;
      
      // Get a big enough string to hold whatever the found file could be.
      char *str = (char *) dtk_malloc(strlen(path) + strlen(cwd) + 
				  strlen(file) + 3);

      //printf("file=%s line=%d path=\"%s\"\n",__FILE__,__LINE__, path);

      // search the path
      while(dir[0] != '\0')
	{
	  // Sift for the next dir in the path.
	  for(;*next_dir != SEPARATOR && *next_dir != '\0';next_dir++);
	  
	  while(*next_dir == SEPARATOR)
	    {
	      *next_dir = '\0';
	      next_dir++;
	    }

	  // Strip off extra '/' from the end of dir
	  size_t len = strlen(dir);
	  while(len && dir[len-1] == '/')
	  {
	    dir[len-1] = '\0';
		len = strlen(dir);
	  }

	  *str = '\0';
	  if(!strcmp(".",dir))     // current working dir
	    sprintf(str,"%s/%s",cwd,file);
	  else if(*dir == '/')     // full path
	    sprintf(str,"%s/%s",dir,file);
	  else if(*dir)            // relative path
	    sprintf(str,"%s/%s/%s",cwd,dir,file);

	  if(*str && check_file_exec(str, uid, gid))
	    {
	      free(path);
	      free(cwd);
	      free(file);
	      return str;
	    }
	  dir = next_dir;
	}
      free(str);
      free(path);
    }

  // Check in current working directory if path is not set.
  else // path == NULL
    {
      // Get the current working directory.
      char *str = (char *) dtk_malloc(strlen(cwd) + strlen(file) + 2);

      // look in the current working directory.
      sprintf(str,"%s/%s",cwd,file);
      if(check_file_exec(str, uid, gid))
	{
	  free(cwd);
	  free(file);
	  return str;
	}
      free(str);
    }

  free(cwd);
  free(file);
  return NULL;   
}	  

#endif  /* #else #ifdef DTK_ARCH_WIN32_VCPP */

// Remove one item from the list remove the objects handle if it's the
// only object with that handle.

static void free_up_a_list(struct DSO_List *l)
{
  if(l)
    {
      // save a copy of the DSO_handle that may need removing.
#ifdef DTK_ARCH_WIN32_VCPP
      HINSTANCE DSO_handle_to_remove = l->DSO_handle;
#else
      void *DSO_handle_to_remove = l->DSO_handle;
#endif

      l->func = NULL;
      if(l->DSOFilename)
	{
	  free(l->DSOFilename);
	  l->DSOFilename = NULL;
	}
      if(l->DSO_handle)
	l->DSO_handle = NULL;

      /** tie up the linked list **/
      if(l == list_bottom)
	list_bottom = l->prev;
      if(l == list_top)
	list_top = l->next;
      if(l->next && l->prev)
	{
	  (l->next)->prev = (l->prev);
	  (l->prev)->next = (l->next);
	}
      else if(l->next)
	(l->next)->prev = NULL;
      else if(l->prev)
	(l->prev)->next = NULL;
      free(l);

      // Now the struct DSO_List has been changed.

      // Do not call dlclose on a DSO handle that is in use.
      // Check the list to see if it is in use with another object,
      // if it is do not close the handle. If it is not in use remove it.
      struct DSO_List *list = list_top;
      for(;list;list = list->next)
	if(list->DSO_handle == DSO_handle_to_remove)
	  break;

      // if there is no list or I made it to the bottom of the list
      // without finding a match, then close the DSO_handle_to_remove.
       if((list_top == NULL ||  list == NULL) && DSO_handle_to_remove)
#ifdef DTK_ARCH_WIN32_VCPP
		 FreeLibrary(DSO_handle_to_remove);
#else
         dlclose(DSO_handle_to_remove);
#endif
    }
}

// add to the bottom of the linked list
static void add_to_list(struct DSO_List *l)
{
  l->func =NULL;
  l->DSOFilename = NULL;
  l->DSO_handle = NULL;
  l->object = NULL;
  l->next = NULL;
  if(list_bottom)
    {
      list_bottom->next = l;
      l->prev = list_bottom;
      list_bottom = l;
    }
  else
    {
      list_bottom = l;
      list_top = l;
      l->prev = NULL;
    }
}


static int get_func_from_DSO(struct DSO_List *l)
{
  char *err = dlerror();
  l->DSO_handle = dlopen(l->DSOFilename,DLOPEN_MODE);
  err = dlerror();
#if(0)
  dtkMsg.add(DTKMSG_DEBUG,
	     "dtkDSO_load(): called dlopen(\"%s\",%d) \n",
	     l->DSOFilename, (int) DLOPEN_MODE);
#endif

  if(err || (l->DSO_handle) == NULL)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkDSO_load(): %s(\"%s\",...) failed:\n%s\n",
		 LDOPEN_NAME, l->DSOFilename, err);
      free_up_a_list(l);
      return -1; // error
    }

  dlerror(); // clear error string if any in dlerror()
  l->func = dlsym(l->DSO_handle, LOADER_FUNCTION);
  err = dlerror();
  if(err)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkDSO_load(): %s(%p,\"%s\") failed:\n%s\n",
		 LDSYM_NAME, l->DSO_handle, LOADER_FUNCTION, err);
      free_up_a_list(l);
      return -1; // error
    }

  if(l->func == NULL)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkDSO_load(): function call "
		 "to DTK function loader \"%s\" failed:\n"
		 "The function was found in \"%s\" but "
		 "failed when it was called\n",
		 LOADER_FUNCTION, l->DSOFilename);
      free_up_a_list(l);
      return -1; // error
    }
  return 0; // success
}


static struct DSO_List *LoadDSO(const char *path_in, const char *filename_in)
{
  char *path = (char *) path_in; // could be NULL, no problem.
  char *filename = find_file(path,filename_in);

#ifdef DTK_ARCH_WIN32_VCPP

  if(filename == NULL)
    {
      if(path && strlen(filename_in) > 4 &&
	 !strcmp(&filename_in[strlen(filename_in)-4],".dll"))
	dtkMsg.add(DTKMSG_WARN, 
		   "dtkDSO_load(): DLL file \"%s\"\n"
		   "not found in path \"%s\".\n",
		   filename_in, path);
      else if(path)
	dtkMsg.add(DTKMSG_WARN,
		   "dtkDSO_load(): DLL file \"%s.dll\"\n"
		   "not found in path \"%s\".\n",
		   filename_in, path);
      else if(strlen(filename_in) > 4 &&
	      !strcmp(&filename_in[strlen(filename_in)-4],".dll"))
	dtkMsg.add(DTKMSG_WARN,
		   "dtkDSO_load(): can't find "
		   "DLL file \"%s\".\n",
		   filename_in);
      else
	dtkMsg.add(DTKMSG_WARN,
		   "dtkDSO_load(): can't find "
		   "DLL file \"%s.dll\".\n",
		   filename_in);
      return NULL;
    }

#else /* #ifdef DTK_ARCH_WIN32_VCPP */

  if(filename == NULL)
    {
      if(path && strlen(filename_in) > 3 &&
	 !strcmp(&filename_in[strlen(filename_in)-3],".so"))
	dtkMsg.add(DTKMSG_WARN, 
		   "dtkDSO_load(): executable DSO file \"%s\"\n"
		   "not found in path \"%s\".\n",
		   filename_in, path);
      else if(path)
	dtkMsg.add(DTKMSG_WARN,
		   "dtkDSO_load(): executable DSO file \"%s.so\"\n"
		   "not found in path \"%s\".\n",
		   filename_in, path);
      else if(strlen(filename_in) > 3 &&
	      !strcmp(&filename_in[strlen(filename_in)-3],".so"))
	dtkMsg.add(DTKMSG_WARN,
		   "dtkDSO_load(): can't find "
		   "executable DSO file \"%s\".\n",
		   filename_in);
      else
	dtkMsg.add(DTKMSG_WARN,
		   "dtkDSO_load(): can't find "
		   "executable DSO file \"%s.so\".\n",
		   filename_in);
      return NULL;
    }
#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#if(0)
  dtkMsg.add(DTKMSG_DEBUG,
	     "dtkDSOLoad() loading: %s\n",filename);
#endif

  /************** ADD to the DSO list **************/

  struct DSO_List *l = (struct DSO_List *) malloc(sizeof(struct DSO_List));
  if(!l)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkDSO_load(): Can't "
		 "load DSO %s malloc() failed:\n",
		 filename);
      free(filename);
      return NULL;
    }

  add_to_list(l);

  l->DSOFilename = filename;

  /****** open DSO and get function **************/

  if(get_func_from_DSO(l))
    return NULL;

  count++;

  dtkMsg.add(DTKMSG_DEBUG,
	     "dtkDSO_load() loaded DSO file:\n %s%s%s.\n",
	     dtkMsg.color.grn, l->DSOFilename, dtkMsg.color.end);

  return l;
}

// For debuging.
void dtkDSO_print(FILE *file)
{
  if (!file) file = stdout;
  fprintf(file,"%s*object_address*%s  %s******FILE******%s\n",
	  dtkMsg.color.tur, dtkMsg.color.end,dtkMsg.color.red,
	  dtkMsg.color.end);
  if(count < 1)
    fprintf(file,"There are no dtk DSOs loaded.\n");
  else
    {
      int i=0;
      for(struct DSO_List *list = list_top; list != NULL;
	  list = list->next, i++)
	fprintf(file,"%s%p%s %s%s%s\n",
		dtkMsg.color.tur,
		list->object,
		dtkMsg.color.end,dtkMsg.color.red,list->DSOFilename, 
		dtkMsg.color.end);
    }
}


void *dtkDSO_load(const char *dso_path, const char *filename, const char *arg)
{
  struct DSO_List *l = LoadDSO(dso_path, filename);
  void * (*func)(const char *) = NULL;
  if(l)
    func = (void * (*)(const char *)) (l->func);

  if(func)
    {
      // We need a local copy of the place in a the loaders linked
      // list, in case dtkDSO_load() is called from l->func().
      struct DSO_List *current_bottom = list_bottom;
      // list_bottom can change when func() is called so
      // we use a local copy of it.
      return (current_bottom->object  = ((void *) func(arg)));
    }

  return NULL;
}

void *dtkDSO_load(const char *dso_path, const char *filename)
{
  struct DSO_List *l = LoadDSO(dso_path, filename);
  void * (*func)(void) = NULL;

  if(l)
    func = (void * (*)(void)) (l->func);

  if(func)
    {
      // We need a local copy of the place in a the loaders linked
      // list, in case dtkDSO_load() is called from l->func().
      struct DSO_List *current_bottom = list_bottom;
      // list_bottom can change when func() is called so
      // we use a local copy of it.
      return (current_bottom->object  = ((void *) func()));
    }

  return NULL;
}

void *dtkDSO_load(const char *dso_path, const char *filename, void *arg)
{
  struct DSO_List *l = LoadDSO(dso_path, filename);
  void * (*func)(void *) = NULL;

  if(l)
    func = (void * (*)(void *)) (l->func);

  if(func)
    {
      // We need a local copy of the place in a the loaders linked
      // list, in case dtkDSO_load() is called from l->func().
      struct DSO_List *current_bottom = list_bottom;
      // list_bottom can change when func() is called so
      // we use a local copy of it.
      return (current_bottom->object  = ((void *) func(arg)));
    }

  return NULL;
}

void *dtkDSO_load(const char *dso_path, const char *filename,
		   void *arg0, void *arg1)
{
  struct DSO_List *l = LoadDSO(dso_path, filename);
  void * (*func)(void *, void *) = NULL;

  if(l)
    func = (void * (*)(void *, void *)) (l->func);

  if(func)
    {
      // We need a local copy of the place in a the loaders linked
      // list, in case dtkDSO_load() is called from l->func().
      struct DSO_List *current_bottom = list_bottom;
      // list_bottom can change when func() is called so
      // we use a local copy of it.
      return (current_bottom->object  = ((void *) func(arg0, arg1)));
    }

  return NULL;
}

// returns NULL on error or a pointer from a call to
// __dtkDSO_unloader() on success.  This calls __dtkDSO_unloader() and
// returns the return value from that call.  This is so we can get a
// pointer to a C++ function.  C functions do not necessarily let C++
// object pointers be passed as augments without braking them.
void *dtkDSO_getUnloader(void *object)
{
  for(struct DSO_List *l = list_top; l; l = l->next)
    if(l->object == object)
      {
	char *err = dlerror(); // clear error string if any in dlerror()
	void *(*func)(void) = (void * (*)(void))
	  dlsym(l->DSO_handle, UNLOADER_FUNCTION);
	err = dlerror();

	if(err || !func)
	  {
	    dtkMsg.add(DTKMSG_WARN, "dtkDSO_getUnloader(): dlsym(%p,\""
		       UNLOADER_FUNCTION
		       "\") failed: can't find unloader function "
		       UNLOADER_FUNCTION
		       "() in DSO file:\n%s\n",
		       l->DSO_handle, err);
	    return NULL; // error
	  }

	return func();
      }

  dtkMsg.add(DTKMSG_WARN,
	     "dtkDSO_getUnloader(): Can't find pointer %p.\n",
	     object);

  return NULL;
}

// returns 0 on success or -1 on error.
int dtkDSO_unload(void *object)
{
  for(struct DSO_List *l = list_top; l; l = l->next)
    if(l->object == object)
      {
	dtkMsg.add(DTKMSG_DEBUG,
		   "dtkDSO_unload() unloading DSO file:\n %s%s%s.\n",
		   dtkMsg.color.grn, l->DSOFilename, dtkMsg.color.end);

	free_up_a_list(l);
	count--;
	return 0;
      }

  dtkMsg.add(DTKMSG_WARN,
	     "dtkDSO_unload(): Can't find pointer %p to remove.\n",
	     object);
  return -1; // error
}

char *dtkDSO_getDSOFilePath(void *object)
{
  struct DSO_List *l = list_top;
  for(;l;l=l->next)
    {
      if(l->object == object)
	return l->DSOFilename;
    }
  return NULL;
}

