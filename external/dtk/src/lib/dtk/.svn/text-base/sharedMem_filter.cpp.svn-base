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
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <winbase.h>
# include <winsock2.h>
# include <malloc.h>
# include <process.h>
//# include "../../config/dtkConfigure.h"
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <sys/time.h>
# include <unistd.h>
# include <sys/un.h>
# include <netinet/in.h>
#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#ifdef DTK_ARCH_IRIX
   typedef uint64_t u_int64_t;
#endif

#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_IRIX || defined DTK_ARCH_CYGWIN
#  include <sys/socket.h>
#endif


#include "types.h"
#include "dtkBase.h"
#include "dtkFilter.h"
#include "filterList.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "dtkRWLock.h"
#include "dtkConditional.h"
#include "dtkSegAddrList.h"
#include "dtkSocket.h"
#include "dtkUDPSocket.h"
#include "dtkSocketList.h"
#include "dtkSharedMem.h"
#include "dtkDSO.h"

#include "_private_sharedMem.h"

// You may ask "Why'd ya do it that way." It would appear the
// everything is done the hardest way possible.  You must keep in mind
// that you cannot store pointers in shared memory, because all
// addresses will be dependent on where the shared memory is mapped to
// in a given process.  In DTK we do not force the shared memory to be
// mapped to a particular address.  This makes it nicer for the user,
// but not the DTK API coder.

/*****************************************************************************
 *                 filter entree in the shared memory file                   *
 *****************************************************************************

   Some entrees are padded to the nearest CHUNK_SIZE in length.  You
   can load any number of filters. More then one filter can be loaded
   from the same file.

   If the entrie was a C struct it would look something like:

entree items
------------
   {
1      struct FilterHeader,
2      PADDING
3     "name", // Uniquely defines a filter for this shared memory file.
              // If no name is given at load time it defaults to the DSO
              // file base name with out the ".so", else if there is a conflict
              // it generates a name simular to the previous default name.
4     "dso_file", // The full path name used to load the filter.  This will
                  // be "" if the filter is not loaded from a DSO file.
5     "arg0","arg1",...,"argN","" // arguments passed to DSO loader.
                                  // This is "" if there are none.
   }


*****************************************************************************/

struct FilterHeader
{
  int32_t offset_to_next; // Offset to the next FilterHeader from this
  // FilterHeader.  This is zero if this is the last one.  This can be
  // negative.
  dtkSharedMem *obj;// This is NULL unless the filter is used in just
                    // one dtkSharedMem object in one process
  pid_t pid; // The PID of the process with the filter loaded.
             // This may be DTK_ALL_PID to show it is requested to be
             // loaded by all processes and obj is NULL.
};

// Tell the dtkFilter_loader.h file that there is no filter to load in
// this file by defining __DTKFILTER_NOT_LOADED__
#define __DTKFILTER_NOT_LOADED__
#include "dtkFilter_loader.h"

static char *getFilterDSOPAth(void)
{
  char *s = getenv("DTK_FILTER_PATH");
  if(!s)
    {
      s = (char *) dtk_malloc(3 + strlen(dtkConfig.getString(dtkConfigure::FILTER_DSO_DIR)));
      sprintf(s, ".%c%s", PATH_SEP, dtkConfig.getString(dtkConfigure::FILTER_DSO_DIR));
    }
  else
    s = dtk_strdup(s);
  return s;
}

// This checks the list of filters DSOs that are in shared memory with
// the local list of filters in filterList. You need a W LOCK before
// calling this.
void dtkSharedMem::sync_filterList(void)
{
  // Find DSO filters in filterList that are not listed in shared
  // memory and mark it for removal.
  if(!filterList) filterList = new FilterList;

  dtkFilter *filter = filterList->start();
  while(filter)
    {
      struct FilterHeader *h = NULL;

      if(header->filter_list_offset)
	{
	  h = (struct FilterHeader *) (((char *) header) + header->filter_list_offset);

	  while(h)
	    {
	      char *Name = (((char *) h) + SIZE_PLUS(sizeof(struct FilterHeader)));
	      if(!strcmp(Name, filterList->getName()))
		break;

	      if(h->offset_to_next)
		h = (struct FilterHeader *) (((char *) h) + h->offset_to_next);
	      else
		h = NULL;
	    }
	}

      if(!h) // This filter was not found in the list.
	filterList->markNeedUnloading();
      filter = filterList->next();
    }

  // Find DSO filters listed in shared memory that are not in
  // filterList and add it to filterList.

  if(header->filter_list_offset)
    {
      pid_t pid = getpid();
      struct FilterHeader *h =
	(struct FilterHeader *) (((char *) header) + header->filter_list_offset);
      
      while(h)
	{
	  char *Name = ((char *) h) + SIZE_PLUS(sizeof(struct FilterHeader));
	  
	  // If not found in the filter list.
	  if(!(filterList->getFilter(Name)))
	    {
	      // Add it to the list so that it will be added after the
	      // rwlock is unlocked.
	      char *s = Name;
	      while(*s) s++; // to '\0' of "name"
	      s++; // to start of "dso_file"
	      char *dso_file = s;// to start of "dso_file"
	      int argc=0;
	      while(*s) s++; // to '\0' of "dso_file"
	      s++; // to start of "argv0"
	      char *argv0 = s; // to start of "argv0"
	      while(*s)
		{
		  while(*s) s++; // to '\0' of "argv[argc]"
		  s++; // to next "argvN"
		  argc++;
		}
	      char **argv = (char **) dtk_malloc((argc+1)*sizeof(char *));
	      s = argv0; // to start of "argv0"
	      int i;
	      for(i=0;*s;i++)
		{
		  argv[i] = s;
		  while(*s) s++; // to '\0' of "argv[argc]"
		  s++; // to next "argvN"
		}
	      argv[i] = NULL; // NULL terminate it.
	      
	      if(h->pid == DTK_ALL_PID ||
		 (h->pid == pid && (!(h->obj) || h->obj == this))
		 )
		filterList->add(FILTER_WAITING_TO_BE_LOADED,
				NULL, Name, dso_file, argc,
				(const char **) argv, h->pid);
	      else
		filterList->add(FILTER_NOT_USED,
				NULL, Name, dso_file, argc,
				(const char **) argv, h->pid);
	      dtk_free(argv);
	    }

	  if(h->offset_to_next)
	    h = (struct FilterHeader *) (((char *) h) + h->offset_to_next);
	  else
	    h = NULL;
	}
    }
      
  // set the local change counter to the value of the change counter
  // in shared memory.
  filter_change_count = header->filter_change_count;
}


int dtkSharedMem::loadFilter(const char *dso_file, int argc,
			     const char **argv, pid_t pid_in,
			     const char *fltr_name_in)
{
  if(isInvalid()) return -1;
  if(!filterList) filterList = new FilterList;

  char *fltr_name = (char *) fltr_name_in;

  pid_t pid = pid_in;
  if(pid_in == DTK_CURRENT_PID || pid_in == DTK_CURRENT_OBJECT)
    pid = getpid();

  // W LOCK
  if(dtkRWLock_wlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::loadFilter(\"%s\",...) "
			"failed.\n",dso_file);
    }

  // sync filterList to the filter list in the shared memory file
  if(filter_change_count != header->filter_change_count)
    sync_filterList();

  char *mem = NULL;
  // Get a unique name.
  {
    if(!fltr_name) fltr_name = (char *) dso_file;
    else if(filterList->getFilter(fltr_name))
      {
	dtkRWLock_wunlock(rwlock);
	dtkMsg.add(DTKMSG_WARN,
		   "dtkSharedMem::loadFilter(\"%s\", %d, %p, %d, "
		   "\"%s\") failed:\nthe filter named "
		   "\"%s\" is already listed in "
		   "shared memory file \"%s\".\n",
		   dso_file, argc, argv, pid, fltr_name_in, fltr_name, name);
	return -1;
      }

    int i=2;
    if(filterList->getFilter(fltr_name))
      {
	mem = (char *) dtk_malloc(strlen(fltr_name)+6);
	for(;filterList->getFilter(fltr_name) && i<9900;i++)
	  sprintf(mem, "%s_%d", fltr_name, i);
	fltr_name = mem;
      }
  }

  // add this filter to the list in the shared memory file
  {
    size_t size =  strlen(fltr_name) + 1 + strlen(dso_file) + 1;
    int i=0;
    for(;i<argc;i++)
      size += strlen(argv[i]) + 1; // "argv[i]"
    size += 1; // "" terminator

    size = SIZE_PLUS(sizeof(struct FilterHeader)) + SIZE_PLUS(size);

    char *shm = _alloc(size);
    if(!shm)
      {
	dtkRWLock_wunlock(rwlock);
	dtkMsg.add(DTKMSG_ERROR,
		   "dtkSharedMem::loadFilter(\"%s\",...) failed.\n",
		   dso_file);
	return -1;
      }

    struct FilterHeader *h = (struct FilterHeader *) shm;
    h->offset_to_next = 0;
    if(pid_in == DTK_CURRENT_OBJECT)
      h->obj = this;
    else
      h->obj = NULL;
    h->pid = pid;

    shm += SIZE_PLUS(sizeof(struct FilterHeader));
    strcpy(shm, fltr_name);
    shm += strlen(fltr_name) + 1;
    strcpy(shm, dso_file);
    shm += strlen(dso_file) + 1;
    for(i=0;i<argc;i++)
      {
	strcpy(shm, argv[i]);
	shm += strlen(argv[i]) + 1;
      }
    strcpy(shm, "");

    if(header->filter_list_offset)
      {
	// Add this to the end of the list in shared memory.
	struct FilterHeader *fh = (struct FilterHeader *)
	  (((char *) header) + header->filter_list_offset);

	while(fh->offset_to_next)
	  fh = (struct FilterHeader *) (((char *)fh) + fh->offset_to_next);

	fh->offset_to_next = ((size_t) h) - ((size_t) fh);
      }
    else
      header->filter_list_offset = (u_int32_t)(size_t)(((char *) h) -
						((size_t) header));
  }

  // increment the change count. So now all other dtkSharedMem objects
  // in any process will see that they need to update their DSO
  // filters.
  filter_change_count = ++(header->filter_change_count);

  // W UNLOCK
  dtkRWLock_wunlock(rwlock);

  // Add this filter to the filterList.
  filterList->add(FILTER_WAITING_TO_BE_LOADED,
		  NULL, fltr_name, dso_file, argc, argv, pid);

  // Free the memory if it was used to get the name of the filter.
  if(mem) dtk_free(mem);

  // Load or Unload all filters in filterList.
  if(unloadAndLoad_filterList())
    {
      dtkMsg.append("dtkSharedMem::loadFilter(\"%s\", %d, %p, %d, "
		    "\"%s\") failed to update filters for "
		    "shared memory file \"%s\".\n",
		    dso_file, argc, argv, pid, fltr_name_in, name);
      return -1;
    }

  return 0;
}


int dtkSharedMem::cleanUpFilters(void)
{
  if(filterList)
    {
      dtkFilter *filter = filterList->start();
      while(filter)
	{
	  if(filter != FILTER_WAITING_TO_BE_LOADED &&
	     filter != FILTER_NOT_USED)
	    filterList->markNeedUnloading();
	  else if (filter == FILTER_WAITING_TO_BE_LOADED)
	    {
	      struct Filter_list *fl = filterList->getFilterList();
	      fl->filter = FILTER_NOT_USED;
	    }
	  filter = filterList->next();
	}
      return unloadAndLoad_filterList();
    }
  return 0;
}


int dtkSharedMem::printFilters(FILE *file)
{
  if(isInvalid()) return -1;
  if(!filterList) filterList = new FilterList;

  if(!file) file = stdout;

  // W LOCK
  if(dtkRWLock_wlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::printFilters() failed.\n");
    }
  // sync filterList to the filter list in the shared memory file
  if(filter_change_count != header->filter_change_count)
    sync_filterList();

  // W UNLOCK
  dtkRWLock_wunlock(rwlock);

  dtkFilter *filter = filterList->start();

  if(filter)
    fprintf(file, "The r/w filters for shared memory file \"%s%s%s\" are:\n"
	    "  %sNAME%s   %sDSO file%s   %sPID%s  %saugments%s\n"
	    "  ----  ---------  -----\n",
	    dtkMsg.color.tur, name, dtkMsg.color.end,
	    dtkMsg.color.yel, dtkMsg.color.end,
	    dtkMsg.color.vil, dtkMsg.color.end,
	    dtkMsg.color.grn, dtkMsg.color.end,
	    dtkMsg.color.yel, dtkMsg.color.end
	    );
  else
    fprintf(file, "There are no r/w filters for shared memory file \"%s%s%s\".\n",
	    dtkMsg.color.tur, name, dtkMsg.color.end);

  while(filter)
    {
      struct Filter_list *l = filterList->getFilterList();

      char pid[20];
      if(l->pid == DTK_ALL_PID)
	sprintf(pid,"ALL PIDs");
      else
	sprintf(pid,"%u", l->pid);

      fprintf(file,"  %s%s%s  %s%s%s  %s%s%s  ",
	      dtkMsg.color.yel, l->name, dtkMsg.color.end,
	      dtkMsg.color.vil, l->dso_file, dtkMsg.color.end,
	      dtkMsg.color.grn, pid, dtkMsg.color.end
	      );
      int i=0;
      for(;i<l->argc;i++)
	fprintf(file, "'%s%s%s' ", dtkMsg.color.yel, (l->argv)[i], dtkMsg.color.end);
      fprintf(file, "\n");
      filter = filterList->next();
    }

  return 0;
}

// Unload DSO filters that list as being needed to be unloaded in
// filterList and then load DSO filters that list as being needed to
// be loaded in filterList.
int dtkSharedMem::unloadAndLoad_filterList(void)
{
  int returnVal = 0;

  // Unload marked filters.
  dtkFilter *filter = filterList->start();
  while(filter)
    {
      if(filterList->needUnloading())
	{
	  void *loaderFunction = filterList->getLoaderFunction();
	  // If the loaderFunction is NULL then it is not loaded.
	  if(loaderFunction && _unloadFilter(loaderFunction, filter))
	    {
	      dtkMsg.append("dtkSharedMem::*Filter(): unloading "
			    "filter named \"%s\" failed.\n",
			    filterList->getName());
	      returnVal += -1; // error
	    }
	  dtkFilter *oldFilter = filter;
	  filter = filterList->next();
	  filterList->remove(oldFilter);
	}
      else
	filter = filterList->next();
    }

  // Load filters.
  filter = filterList->start();
  while(filter)
    {
      if(filter == FILTER_WAITING_TO_BE_LOADED)
	{
	  struct Filter_list *l = filterList->getFilterList();
	  int i = _loadFilter(l->dso_file, l->argc, (const char **) l->argv,
			      &(l->filter),
			      (dtkFilter *(**)(size_t, int, const char **))
			      &(l->loaderFunction));
	  if(i) // Loaded a not so usable filter.  We keep it listed
	    // in filterList so that we stay in sync with
	    // the DSO filter list in shared memory.
	    {
	      dtkMsg.append("failed to load filter named \"%s\".\n",
			    filterList->getName());
	      l->filter = FILTER_NOT_USED;
	      filterList->removeRead();
	      filterList->removeWrite();
	      returnVal += i;
	      return returnVal;
	    }
	}
      filter = filterList->next();
    }

    return returnVal;
}

int dtkSharedMem::loadFilter(const char *dso_file, pid_t pid,
			     const char *fltr_name)
{
  return loadFilter(dso_file, 0, NULL, pid, fltr_name);
}

int dtkSharedMem::loadFilter(const char *dso_file, int argc,
			     const char **argv, const char *fltr_name)
{
  return loadFilter(dso_file, argc, argv, DTK_ALL_PID, fltr_name);
}


int dtkSharedMem::_loadFilter(const char *dso_file, int argc,
			      const char **argv, dtkFilter **filter,
			      dtkFilter *(**loaderFunction)(size_t, int, const char **))
{
  if(isInvalid()) return -1;

  if(!filterDSOPath)
    filterDSOPath = getFilterDSOPAth();

    dtkFilter *(*loaderFunc)(size_t, int, const char **) =
      (dtkFilter * (*)(size_t, int, const char **))
      dtkDSO_load(filterDSOPath, dso_file);

  if(!loaderFunc)
    {
      dtkMsg.append("dtkSharedMem::loadFilter() failed to load DSO file"
		    " \"%s\".\n", dso_file);
      return -1;
    }

  dtkFilter *fltr = loaderFunc(user_size, argc, argv);

  if(fltr == DTKFILTER_LOAD_ERROR)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkSharedMem::loadFilter() failed: DSO file \"%s\"\n"
		 " entry function returned an error code.\n", dso_file);
      _unloadFilter((void *) loaderFunc, fltr);
      return -1; // error
    }

  if(fltr->isInvalid())
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkSharedMem::loadFilter() failed: DSO file \"%s\"\n"
		 " entry function returned an invalid filter object.\n", dso_file);
      _unloadFilter((void *) loaderFunc, fltr);
      return -1; // error
    }

  if(fltr == DTKFILTER_LOAD_UNLOAD)
    {
      dtkMsg.add(DTKMSG_INFO,
		 "dtkSharedMem::loadFilter(): DSO file \"%s\"\n"
		 " entry function returned telling to unload DSO.\n",
		 dso_file);
      if(_unloadFilter((void *) loaderFunc, fltr))
	return -1;
    }

  if(_addFilter(fltr, (void *) loaderFunc))
    {
      dtkMsg.append("dtkSharedMem::loadFilter() "
		    "failed: DSO file \"%s\"\n", dso_file);
      _unloadFilter((void *) loaderFunc, fltr);
      return -1; // error
    }

  *loaderFunction = loaderFunc;
  *filter = fltr;

  return 0;
}

// Adds filter to this object.
int dtkSharedMem::_addFilter(dtkFilter *fltr, void *loaderFunction)
{
  unsigned int _flags = 0;

  if(fltr->getRequiredSize() && fltr->getRequiredSize() != user_size)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkSharedMem::loadFilter() failed: The required size for this filter "
		 "is %d\n and this shared memory \"%s\" is of size %d.\n",
		 fltr->getRequiredSize(), getName(), user_size);
      return -1; // error
    }
  if(fltr->needTimeStamp())
    {
      _flags |= IS_WRITE_FILTER_TIMESTAMPED;
      if(timeStamp()) return -1; // error
    }

  if(fltr->needQueued())
    {
      if(queue() || flush()<0)
	return -1; // error
      _flags |= IS_READ_FILTER_QUEUED;
    }

  if(fltr->needWriteCount())
    {
      if(writeCount())
	return -1; // error
      _flags |= IS_READ_FILTER_QUEUED;
    }

  // Assume that it is both a read and a write filter until they tells
  // me otherwise by trying to use them.
  filter_flags |= _flags | IS_READ_FILTER | IS_WRITE_FILTER;

  return 0;
}

int dtkSharedMem::_unloadFilter(void *loaderFunction, dtkFilter *fltr)
{
  int (*unloaderFunc)(dtkFilter *) = 
    (int (*)(dtkFilter *)) dtkDSO_getUnloader(loaderFunction);

  if(!unloaderFunc)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkSharedMem::*loadFilter(): unloading failed.\n");
      // dtkDSO_getUnloader() will spew more detail.
      dtkDSO_unload(loaderFunction);
      return -1; // error
    }

  if(unloaderFunc(fltr) == DTKFILTER_UNLOAD_ERROR)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkManager::*loadFilter(): unloader function failed.\n");
      dtkDSO_unload(loaderFunction);
      return -1; // error
    }

  // dtkDSO_unload() returns -1 on error or 0 on success
  if(dtkDSO_unload(loaderFunction))
    return
      dtkMsg.add(DTKMSG_WARN, 0, -1,
		 "dtkSharedMem::*loadFilter(): failed for shared memory file \"%s\".\n",
		 name);

  dtkMsg.add(DTKMSG_DEBUG, "dtkSharedMem::*() unloaded filter for "
	     "shared memory file \"%s\".\n",
	     name);

  return 0;
}

int dtkSharedMem::unloadFilter(const char *fltr_name)
{
  if(isInvalid()) return -1;
  if(!filterList) filterList = new FilterList;

  // W LOCK
  if(dtkRWLock_wlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::unloadFilter(\"%s\") failed.\n",
			fltr_name);
    }

  // remove this from the filter list in the shared memory file.
  if(header->filter_list_offset)
    {
      struct FilterHeader *fh = (struct FilterHeader *)
	(((char *) header) + header->filter_list_offset);
      struct FilterHeader *prev_fh = NULL;
      
      while(fh)
	{
	  char *Name = ((char *) fh) + SIZE_PLUS(sizeof(struct FilterHeader));
	  if(!strcmp(Name,fltr_name))
	    {
	      break;
	    }
	  prev_fh = fh;
	  if(fh->offset_to_next)
	    fh = (struct FilterHeader *) (((char *) fh) + fh->offset_to_next);
	  else
	    fh = NULL;
	}
      if(fh) // remove this filter header
	{
	  if(prev_fh)
	    {
	      if(fh->offset_to_next)
		prev_fh->offset_to_next = (int32_t)(((size_t) fh)+fh->offset_to_next) -
		  ((size_t) prev_fh);
	      else
		prev_fh->offset_to_next = 0;
	    }
	  else // this is the first in the list
	    {
	      if(fh->offset_to_next)
		header->filter_list_offset = (u_int32_t)(((size_t) fh)+fh->offset_to_next) -
		  ((size_t) header);
	      else
		header->filter_list_offset = 0;
	    }

	  _free(fh);
	}
    }
  
  // sync to the filter list in the shared memory file
  sync_filterList();

  // increment the change count. So now all other dtkSharedMem objects
  // in any process will see that they need to update their DSO
  // filters.
  filter_change_count = ++(header->filter_change_count);

  // W UNLOCK
  dtkRWLock_wunlock(rwlock);

  if(unloadAndLoad_filterList())
    {
      dtkMsg.append("dtkSharedMem::unloadFilter(\"%s\") "
		    "failed for shared memory file \"%s\".\n",
		    fltr_name, name);
      return -1;
    }

  return 0;
}

void dtkSharedMem::stopReadFilters(void)
{
  filter_flags &= ~ALLOW_READ_FILTER;
}

void dtkSharedMem::stopWriteFilters(void)
{
  filter_flags &= ~ALLOW_WRITE_FILTER;
}

void dtkSharedMem::stopFilters(void)
{
  filter_flags &= ~ALLOW_WRITE_FILTER;
  filter_flags &= ~ALLOW_READ_FILTER;
}

