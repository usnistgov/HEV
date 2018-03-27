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
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <winbase.h>
# include <winsock2.h> // for struct timeval
# include <fcntl.h>
# include <sys/stat.h>
# include <io.h>
# include <direct.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <sys/time.h>
# include <fcntl.h>
# include <unistd.h>
# include <wctype.h>
# include <sys/mman.h>
# ifdef DTK_ARCH_CYGWIN
#  include <dirent.h>
#  define direct dirent
# else
#  include <sys/dir.h>
# endif
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

#include "_private_sharedMem.h"


static int _destroyShmFile(const char *filename_org, int flags,
			               const char *fileName)
{
#ifdef DTK_ARCH_WIN32_VCPP
  dtkSharedMem* shm = new dtkSharedMem( fileName );
  if( shm->isValid() )
  {
    delete shm;
    if(unlink(fileName))
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 "unlink(\"%s\") failed to remove file.\n",
		 filename_org, flags, fileName);
      return -1;
    }

    dtkMsg.add(DTKMSG_DEBUG, 1,
	     "dtkSharedMem_destroy(\"%s\",%d) "
	     "removed shared memory file \"%s%s%s\".\n",
	     filename_org, flags,
	     dtkMsg.color.tur, fileName, dtkMsg.color.end);
  }
  else
  {
    delete shm;
    return dtkMsg.add( DTKMSG_NOTICE, 1, -1,
        "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
        "Skipping file %s - not a valid dtkSharedMem segment\n",
        filename_org, flags, fileName ); 
  }
  return 0;
#endif

  int fd = open(fileName, O_RDWR);
  if(fd < 0)
    return dtkMsg.add(DTKMSG_WARN, 1, -1,
		      "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		      " open(\"%s\", O_RDWR) failed.\n",
		      filename_org, flags, fileName);

 struct dtkSharedMem_header *header;

#ifdef DTK_ARCH_WIN32_VCPP

  struct dtkSharedMem_header h;
  header = &h;
  if(sizeof(struct dtkSharedMem_header) !=
	  _read(fd, header, sizeof(struct dtkSharedMem_header)))
  {
     dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 " failed to read %d bytes from file \"%s\".\n",
		 filename_org, flags,
		 sizeof(struct dtkSharedMem_header),
		 fileName);
      close(fd);
      return -1;
  }

#else /* #ifdef DTK_ARCH_WIN32_VCPP */

  header = (struct dtkSharedMem_header *)
    mmap(NULL, sizeof(dtkSharedMem_header),
	 PROT_READ|PROT_WRITE,
	 MAP_SHARED, fd, 0);
  if(MAP_FAILED == (void *) header)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 " mmap() failed can't map file \"%s\".\n",
		 filename_org, flags, fileName);
      close(fd);
      return -1;
    }

#endif /* #else #ifdef DTK_ARCH_WIN32_VCPP */

  if(header->magic_number != DTKSHAREDMEM_HEADER)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 " bad magic number for file \"%s\".\n",
		 filename_org, flags, fileName);
#ifndef DTK_ARCH_WIN32_VCPP
      munmap(header, sizeof(dtkSharedMem_header));
#endif
      close(fd);
      return -1;
    }

#ifndef DTK_ARCH_WIN32_VCPP
  if(dtkRWLock_connect(&(header->rwlock)) ||
     dtkRWLock_destroy(&(header->rwlock)))
    dtkMsg.append("dtkSharedMem_destroy(\"%s\",%d) "
		  " for file \"%s\".\n",
		  filename_org, flags, fileName);

  // Destroy the dtkConditional that would be use for blocking read if
  // it exists.
  if(header->blocking_read_lock.magic_number == DTKCONDITIONAL_TYPE)
    {
      if(dtkConditional_connect(&(header->blocking_read_lock)))
	{
	  dtkMsg.append("dtkSharedMem_destroy(\"%s\",%d) "
			" for file \"%s\".\n",
			filename_org, flags, fileName);
	}
      else if(dtkConditional_destroy(&(header->blocking_read_lock)))
	dtkMsg.append("dtkSharedMem_destroy(\"%s\",%d) "
		      " for file \"%s\".\n",
		      filename_org, flags, fileName);
    }

  munmap(header, sizeof(dtkSharedMem_header));
#endif /* #ifndef DTK_ARCH_WIN32_VCPP */

  if(unlink(fileName))
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 "unlink(\"%s\") failed to remove file.\n",
		 filename_org, flags, fileName);
      close(fd);
      return -1;
    }

  dtkMsg.add(DTKMSG_DEBUG, 1,
	     "dtkSharedMem_destroy(\"%s\",%d) "
	     "removed shared memory file \"%s%s%s\".\n",
	     filename_org, flags,
	     dtkMsg.color.tur, fileName, dtkMsg.color.end);
  close(fd);
  return 0;
}

// free() after.
static char *Getcwd()
{
#define STEP     ((size_t) 64)
#define MAXSIZE  ((size_t) 100)*STEP

  size_t size = STEP;
  char *str = (char *) dtk_malloc(size);

  for(;size < MAXSIZE ;)
    {
      if(getcwd(str,size-1))
        {
          return str;
        }
      size += STEP;
      str = (char *) dtk_realloc(str, size);
    }

  free(str);
  dtkMsg.add(DTKMSG_WARN, 1, -1,
	     "dtkSharedMem_destroy() failed, getcwd() failed.\n");

  return NULL;
}

//#ifndef DTK_ARCH_WIN32_VCPP
// return 0 on success and non-zero on failure.  This recurses too.
static int _destroyShmDir(const char *filename, int flags,
			  const char *dirname)
{
  char *cwd = Getcwd();
  if(!cwd) return 1;

#ifdef DTK_ARCH_WIN32_VCPP
  struct _finddata_t file_data;
  intptr_t file_search = _findfirst( dirname, &file_data );
  if( file_search == -1 )
#else //DTK_ARCH_WIN32_VCPP
  DIR *dir = opendir(dirname);

  if(!dir)
#endif //DTK_ARCH_WIN32_VCPP
    {
      if(chdir(cwd))
	dtkMsg.add(DTKMSG_WARN, 1,
		   "dtkSharedMem_destroy(\"%s\",%d) failed, "
		   "chdir(\"%s\") failed.\n",
		   filename, flags, cwd);
      free(cwd);
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 " opendir(\"%s\") failed.\n",
		 filename, flags, dirname);

#ifdef DTK_ARCH_WIN32_VCPP
      _findclose( file_search );
#endif //DTK_ARCH_WIN32_VCPP

      return -1;
    }
#ifdef DTK_ARCH_WIN32_VCPP
  _findclose( file_search );
#endif //DTK_ARCH_WIN32_VCPP

  if(chdir(dirname))
    {
      free(cwd);
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed, "
		 "chdir(\"%s\") failed.\n",
		 filename, flags, dirname);
      return -1;
    }

  int return_val = 0;

#ifdef DTK_ARCH_WIN32_VCPP
  char* dir_contents = new char[strlen(dirname)+3];
  sprintf( dir_contents, "%s\\*", dirname );
  file_search = _findfirst( dir_contents, &file_data );
  while( !_findnext( file_search, &file_data ) )
  {
    if(strcmp(".", file_data.name) && strcmp("..", file_data.name))
    {
      if( file_data.attrib & _A_SUBDIR )
      {
        char* new_dirname = new char[strlen(dirname)+strlen(file_data.name)+1];
        sprintf( new_dirname, "%s\\%s", dirname, file_data.name );
        return_val += _destroyShmDir( filename, flags, new_dirname );
      }
      else if( file_data.attrib & _A_NORMAL )
      {
        char* new_filename = new char[strlen(dirname)+strlen(file_data.name)+1];
        sprintf( new_filename, "%s\\%s", dirname, file_data.name );
        return_val += _destroyShmFile( file_data.name, flags, new_filename );
      }
      else if( file_data.attrib & _A_ARCH )
      {
        char* new_filename = new char[strlen(dirname)+strlen(file_data.name)+1];
        sprintf( new_filename, "%s\\%s", dirname, file_data.name );
        return_val += _destroyShmFile( file_data.name, flags, new_filename );
      }
      else // Found a file that I can't remove.
      {
        dtkMsg.add(DTKMSG_NOTICE, 1, 1,
           "dtkSharedMem_destroy(\"%s\",%d) failed,\n"
           " will not remove file \"%s\" from directory "
           "\"%s\".\n",
           filename, flags, file_data.name, dirname);
        return_val++;
      }
    }
  }
  _findclose( file_search );
#else //DTK_ARCH_WIN32_VCPP
  struct direct *ent;
  while((ent = readdir(dir)))
    {
      struct stat s;
      if(strcmp(".", ent->d_name) && strcmp("..", ent->d_name))
	{
	  if(!stat(ent->d_name, &s))
	    {
	      if(S_ISDIR(s.st_mode)) // is a directory.
		return_val += _destroyShmDir(filename, flags, ent->d_name);
	      else if(S_ISREG(s.st_mode)) // is a regular file
		return_val += _destroyShmFile(filename, flags, ent->d_name);
	      else // Found a file that I can't remove.
		{
		  dtkMsg.add(DTKMSG_NOTICE, 1, 1,
			     "dtkSharedMem_destroy(\"%s\",%d) failed,\n"
			     " will not remove file \"%s\" from directory "
			     "\"%s\".\n",
			     filename, flags, ent->d_name, &s, dirname);
		  return_val++;
		}
	    }
	  else // stat() failed
	    {
	      dtkMsg.add(DTKMSG_NOTICE, 1, 1,
			 "dtkSharedMem_destroy(\"%s\",%d) failed,\n"
			 " stat(\"%s\", %p) failed file in"
			 " directory \"%s\".\n",
			 filename, flags, ent->d_name, &s, dirname);
	      return_val++; // Found a file that I can't remove.
	    }
	}
    }
#endif //DTK_ARCH_WIN32_VCPP

  if(chdir(cwd))
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed, "
		 "chdir(\"%s\") failed.\n",
		 filename, flags, cwd);
      free(cwd);
      return -1;
    }

#ifdef DTK_ARCH_WIN32_VCPP
  if(!RemoveDirectory(dirname))
#else
  if(rmdir(dirname))
#endif
    {
      dtkMsg.add(DTKMSG_NOTICE, 1, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed, "
		 "rmdir() failed to remove directory \"%s\".\n",
		 filename, flags, dirname);
      return_val++; // Found a file that I can't remove.
    }

  if(return_val)
    dtkMsg.append(" had trouble removing stuff from directory \"%s\".\n",
		  dirname);
  else
    dtkMsg.add(DTKMSG_DEBUG, "dtkSharedMem_destroy(\"%s\",%d) removed all\n"
	       " DTK shared memory files from directory \"%s%s%s\".\n",
		 filename, flags,
	       dtkMsg.color.tur, dirname, dtkMsg.color.end);

  free(cwd);
  return - return_val;
}
//#endif /* #ifndef DTK_ARCH_WIN32_VCPP */

int dtkSharedMem_destroy(const char *filename, int flags)
{
  char *fullPath = NULL;

  {
    char *dtk_sharedMem_dir = getenv("DTK_SHAREDMEM_DIR");
    if(!dtk_sharedMem_dir)
      dtk_sharedMem_dir = const_cast<char*>(DTK_DEFAULT_SHAREDMEM_DIR);

    if(!filename || !(filename[0]))
      fullPath = dtk_strdup(dtk_sharedMem_dir);
    else if(filename[0] != '/')
      { // get the full path file name, fullPath
	fullPath = (char *)
	  dtk_malloc(strlen(dtk_sharedMem_dir) +
		     strlen("/") + strlen(filename) + 1);
	sprintf(fullPath, "%s/%s", dtk_sharedMem_dir, filename);
      }
    else // full path file name, fullPath was passed in filename
      fullPath = dtk_strdup(filename);
  }

  struct stat s;

  if(stat(fullPath, &s)) // fullPath is not accessable.
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 " stat(\"%s\",%p) failed, can't access "
		 "file \"%s\".\n",
		 filename, flags, fullPath, &s, fullPath);
      free(fullPath);
      return -1;
    }

#ifdef DTK_ARCH_WIN32_VCPP
# define S_ISREG(x)  ((x) & _S_IFREG)
# define S_ISDIR(x)  ((x) & _S_IFDIR)
#endif

  // File too small.
  if(S_ISREG(s.st_mode) &&
     s.st_size < (off_t) SIZE_PLUS(sizeof(struct dtkSharedMem_header)))
    {
     
      dtkMsg.add(DTKMSG_WARN, 0,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 " file \"%s\" is of size %d (<%d)bytes which "
		 "is too small.\n",
		 filename, flags, fullPath, s.st_size,
		 SIZE_PLUS(sizeof(struct dtkSharedMem_header)));
       free(fullPath);
       return -1;
    }

  // File is not a regular file or directory.
  if(!(S_ISREG(s.st_mode) || S_ISDIR(s.st_mode)))
    {
      dtkMsg.add(DTKMSG_WARN, 0, -1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 " file \"%s\" is not a regular file or a directory.\n",
		 filename, flags, fullPath);
      free(fullPath);
      return -1;
    }

  // File is a directory and this is not recursive.
  if(S_ISDIR(s.st_mode)
//#ifndef DTK_ARCH_WIN32_VCPP
	  && !(flags & DTK_RECURSE)
//#endif
	  )
    {
      dtkMsg.add(DTKMSG_WARN, 0, -1,
		 "dtkSharedMem_destroy(\"%s\",%d) failed:\n"
		 " file \"%s\" is a directory.\n"
//#ifdef DTK_ARCH_WIN32_VCPP
//		 "removing directories is not supported on windows.\n"
//#endif
		 ,
		 filename, flags, fullPath);
      free(fullPath);
      return -1;
    }

//#ifndef DTK_ARCH_WIN32_VCPP
  // File is a regular file
  if(!S_ISDIR(s.st_mode))
    {
//#endif
      int ret = 
	_destroyShmFile(filename, flags, fullPath);
      free(fullPath);
      return ret;
//#ifndef DTK_ARCH_WIN32_VCPP
    }

  // It's a directory and this is recursive.
  int return_val = _destroyShmDir(filename, flags, fullPath);

  free(fullPath);

  return return_val;
//#endif /* #ifndef DTK_ARCH_WIN32_VCPP */
}
