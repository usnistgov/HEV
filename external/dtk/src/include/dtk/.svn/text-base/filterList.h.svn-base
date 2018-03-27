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

// This manages lists of the loaded dtkFilters for the dtkSharedMem
// class.  Do not Doxygen this.  This is only used internally to DTK.
// This has no failure modes other than memory allocation (malloc())
// which is not delt with.

struct Filter_list
{
  // If (filter==FILTER_NOT_LOADED) then the filter has not been loaded yet.
  // FILTER_NOT_LOADED is used in sharedMem_filter.cpp
  dtkFilter *filter;
  // loaderFunction is returned from dtkDSO_load()
  void *loaderFunction; 
  int doRead; 
  // bool to tell if there is a read filter method used
  int doWrite;
  // bool to tell if there is a write filter method used
  struct Filter_list *next;

  pid_t pid;
  
  char *name; // Unique to this filter in the list, it also
  // in the shared memory file. See sharedMem_filter.cpp.
  
  char *dso_file;
  int argc;
  char **argv;
};


class FilterList
{
 public:

  FilterList(void);

  virtual ~FilterList(void);

  void add(dtkFilter *fltr,
	   void *loaderFunc,
	   const char *name,
	   const char *dso_file,
	   int argc, const char **argv, pid_t pid);

  void remove(dtkFilter *fltr);
  void remove(const char *name);

  // get the first (first loaded) in the list.
  // Returns NULL is there are none in the list.
  inline dtkFilter *start(void)
    {
      if(first)
	{
	  current = first;
	  return first->filter;
	}
      return NULL;
    }

  // get the next filter in the list. Returns NULL at the end.
  inline dtkFilter *next(void)
    {
      if(current)
	{
	  current = current->next;
	  if(current)
	    return current->filter;
	}
      return NULL;
    }

  inline int checkRead(void)
    {
      if(current)
	return current->doRead;
      return 0;
    }

  // Frees and sets dso_file to NULL.
  void markNeedUnloading(const char *name=NULL);

  // returns true if the current in list needs unloading by having
  // current->dso_file set to NULL.
  inline int needUnloading(void)
    {
      return (current->dso_file)?0:1;
    }

  inline int checkWrite(void)
    {
      if(current)
	return current->doWrite;
      return 0;
    }

  inline void removeRead(void)
    {
      if(current)
	current->doRead = 1;
    }
 
  inline void removeWrite(void)
    {
      if(current)
	current->doWrite = 1;
    }

  // do not mess with the returned memory.
  void *getLoaderFunction(const char *name=NULL);

  dtkFilter *getFilter(const char *name);

  char *getName(dtkFilter *fltr=NULL);

  struct Filter_list *getFilterList(const char *name=NULL);

 private:

  void _free(struct Filter_list *l);

  struct Filter_list *first, *current;
};

