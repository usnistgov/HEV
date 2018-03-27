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
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <direct.h>
# include <malloc.h>
#else
# include <pthread.h>
# include <errno.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdarg.h>
#endif

#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "_private_sharedMem.h"

#ifdef DTK_ARCH_WIN32_VCPP
typedef int uid_t;
typedef int gid_t;
#endif


// return 1 on finding it.
// returns 0 if not found.

static int check_file(const char *file, mode_t mode,
		      uid_t uid, gid_t gid)
{
#if(0)
  dtkMsg.add(DTKMSG_DEBUG,
	     "dtk_getFileFromPath(): Trying \"%s\".\n",
	     file);
#endif
  struct stat Stat;

  if(stat(file, &Stat))
    {
#if(0)
      dtkMsg.add(DTKMSG_DEBUG, 1,
		 "dtk_getFileFromPath() failed: "
		 "stat(\"%s\",&Stat) failed.\n",
		 file);
#endif
      return 0;
    }
#ifndef DTK_ARCH_WIN32_VCPP
  else if(mode &&
	  /* is regular file */ Stat.st_mode & S_IFREG &&
	  ( // owner is you and owner has execute permission
	   (uid == Stat.st_uid && Stat.st_mode & mode & S_IRWXU) ||
	   // group is you and group has execute permission
	   (gid == Stat.st_uid && Stat.st_mode & mode & S_IRWXG) ||
	   //  you as other has execute permission
	   (Stat.st_mode & mode & S_IRWXO)
	   ))
    return 1;
  else if(mode)
    return 0;
#endif
  else
    return 1;
}


void *dtk_malloc(size_t size)
{
  void *ptr = malloc(size);
  if(!ptr)
    dtkMsg.add(DTKMSG_ERROR, 1,
	       "malloc(%d) failed.\n", size);
  return ptr;
}

void dtk_free(void *ptr)
{
  if(ptr)
    free(ptr);
  else
    dtkMsg.add(DTKMSG_ERROR,
	       "dtk_free(0x0) failed: you "
	       "can't free address 0x0.\n");
}

void *dtk_realloc(void *ptr, size_t size)
{
  void *p = realloc(ptr, size);
  if(!p)
    dtkMsg.add(DTKMSG_ERROR, 1,
	       "realloc(%p,%d) failed.\n",
	       ptr, size);
  return p;
}

char *dtk_strdup(const char *s)
{
  char *ptr = strdup(s);
  if(!ptr)
    dtkMsg.add(DTKMSG_ERROR, 1,
	       "strdup(str) failed: "
	       "str address=%p.\n",s);
  return ptr;
}


// Get an option argument at index *i;
// if found index the counter *i to the next string.

char *dtk_getOpt(const char *shorT, const char *lonG, 
		  int argc, const char **argv, int *i)
{
  // check for form long=VALUE
  size_t len = 0;
  if(*i < argc)
    {
      char *str = (char *) argv[*i];
      for(;*str && *str != '=';str++);
      if(*str == '=')
	{
	  len = (size_t) str - (size_t) argv[*i];
	  str++;
	}
      if(len && !strncmp(argv[*i],lonG,len)
	  &&  *str != '\0')
	{
	  (*i)++;
	  return str;
	}
    }

  // check for  -a VALUE   or --long VALUE
  if(((*i + 1) < argc) &&
     (!strcmp(argv[*i],lonG) || !strcmp(argv[*i],shorT)))
    {
      (*i)++;
      return (char *) argv[(*i)++];
    }
  
  // check for   -aVALUE
  if( (*i < argc) && !strncmp(argv[*i],shorT,len=strlen(shorT))
      && argv[*i][len] != '\0')
    return (char *) &(argv[(*i)++][len]);

  return (char *) NULL;
}

void *dtk_swapBytes(void *data, size_t size)
{
  unsigned char *end = &((unsigned char *) data)[size-1];
  unsigned char *begin = (unsigned char *) data;
  unsigned char c;
 
  while(((size_t) begin) < ((size_t) end))
    {
      c = *begin;
      *(begin++) = *end;
      *(end--) = c;
    }
  return data;
}

// Find a file that is the first that is in path.  You must free()
// after.  This will resolve "." as the current working directory.

// all mode bits are or'ed with the 
char *dtk_getFileFromPath(const char *path_in,
			  const char *file, mode_t mode)
{
  uid_t uid=0;
  gid_t gid=0;

#ifndef DTK_ARCH_WIN32_VCPP
  if(mode)
    {
      uid = geteuid();
      gid = getegid();
    }
#endif

  if(file == NULL || file[0] == '\0') return NULL;

  //If file is full path.
  if(IS_FULL_PATH(file))
    {
      if(check_file(file,mode,uid,gid))
	    return dtk_strdup(file);
      else
	    return NULL;
    }



#ifdef DTK_ARCH_IRIX
  char *cwd = getcwd(NULL, (size_t) -1);
#else /* Linux and other */
  char *cwd = getcwd(NULL, (size_t) 0);
#endif

  if(cwd == NULL)
    {
#if(0)
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtk_getFileFromPath() failed: "
		 "getcwd() failed.\n");
#endif
      errno = 0;
      cwd = (char *) dtk_malloc(1);
      if(cwd == NULL)
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk_getFileFromPath() failed: "
		     "malloc() failed.\n");
	  return NULL;
	}
      cwd[0] = '\0';
    }

  if(!strcmp(DIR_SEP,cwd))
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
	  for(;*next_dir != PATH_SEP && *next_dir != '\0';next_dir++);
	  
	  while(*next_dir == PATH_SEP)
	    {
	      *next_dir = '\0';
	      next_dir++;
	    }
	  if(!strcmp(".",dir))
	    sprintf(str,"%s%c%s",cwd,DIR_CHAR, file);
	  else if(*dir == DIR_CHAR)
	    {
	      while(dir[strlen(dir)-1] == DIR_CHAR)
		dir[strlen(dir)-1] = '\0';
	      sprintf(str,"%s%c%s",dir, DIR_CHAR, file);
	    }
#ifdef DTK_ARCH_WIN32_VCPP
          else if(dir[1] == ':' && dir[2] == DIR_CHAR)
	    {
	      while(dir[strlen(dir)-1] == DIR_CHAR)
		dir[strlen(dir)-1] = '\0';
	      sprintf(str,"%s%c%s",dir, DIR_CHAR, file);
	    }
#endif
	  else
	    {
	      while(dir[strlen(dir)-1] == DIR_CHAR)
		dir[strlen(dir)-1] = '\0';
	      sprintf(str,"%s%c%s%c%s",
			     cwd, DIR_CHAR, dir, DIR_CHAR, file);
	    }
	  if(check_file(str,mode,uid,gid))
	    {
	      free(path);
	      free(cwd);
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
      sprintf(str,"%s%c%s",cwd,DIR_CHAR,file);
      if(check_file(str,mode,uid,gid))
	{
	  free(cwd);
	  return str;
	}
      free(str);
    }

  free(cwd);
  return NULL;   
}

void tokenize( const std::string& str, std::vector<std::string>& tokens,
    const std::string& delimiters )
{
    size_t startpos = str.find_first_not_of( delimiters, 0 );
    size_t endpos = str.find_first_of( delimiters, startpos );
    while( startpos != std::string::npos )
    {
        tokens.push_back( str.substr( startpos, endpos - startpos) );
        startpos = str.find_first_not_of( delimiters, endpos );
        endpos = str.find_first_of( delimiters, startpos );
        if( startpos != std::string::npos && endpos == std::string::npos )
            endpos = str.size();
    }
}

int convertStringToNumber( int& value, const char* str )
{
    if( str == NULL )
    {
        dtkMsg.add( DTKMSG_ERROR, "The const char* parameter passed to convertStringToNumber\n"
                "is NULL.\n" );
        return -1;
    }
    if( !strcmp( str, "" ) )
    {
        dtkMsg.add( DTKMSG_ERROR, "The const char* parameter passed to convertStringToNumber\n"
                "is empty.\n" );
        return -1;
    }
    errno = 0;
    value = strtol( str, NULL, 0 );
    return errno;
}

int convertStringToNumber( long& value, const char* str )
{
    if( str == NULL )
    {
        dtkMsg.add( DTKMSG_ERROR, "The const char* parameter passed to convertStringToNumber\n"
                "is NULL.\n" );
        return -1;
    }
    if( !strcmp( str, "" ) )
    {
        dtkMsg.add( DTKMSG_ERROR, "The const char* parameter passed to convertStringToNumber\n"
                "is empty.\n" );
        return -1;
    }
    errno = 0;
    value = strtol( str, NULL, 0 );
    return errno;
}

int convertStringToNumber( float& value, const char* str )
{
    if( str == NULL )
    {
        dtkMsg.add( DTKMSG_ERROR, "The const char* parameter passed to convertStringToNumber\n"
                "is NULL.\n" );
        return -1;
    }
    if( !strcmp( str, "" ) )
    {
        dtkMsg.add( DTKMSG_ERROR, "The const char* parameter passed to convertStringToNumber\n"
                "is empty.\n" );
        return -1;
    }
    errno = 0;
    value = strtod( str, NULL );
    return errno;
}

int convertStringToNumber( double& value, const char* str )
{
    if( str == NULL )
    {
        dtkMsg.add( DTKMSG_ERROR, "The const char* parameter passed to convertStringToNumber\n"
                "is NULL.\n" );
        return -1;
    }
    if( !strcmp( str, "" ) )
    {
        dtkMsg.add( DTKMSG_ERROR, "The const char* parameter passed to convertStringToNumber\n"
                "is empty.\n" );
        return -1;
    }
    errno = 0;
    value = strtod( str, NULL );
    return errno;
}

std::string& tolower( std::string& str )
{
    std::transform( str.begin(), str.end(), str.begin(), (int(*)(int))::tolower );
    return str;
}


static int *_signalFlag = NULL;

static void _signalCatcher(int sig)
{
  dtkMsg.add(DTKMSG_NOTICE, "Caught signal %d: setting "
	     "signal flag to zero.\n", sig);
  *_signalFlag = 0;
}

int dtk_catchSignal(const int *signals, int *flag)
{
  if(!flag) return -1;
  
  for(int i=0;signals[i];i++)
    // ignoring failure modes here.
    signal(signals[i], _signalCatcher);

  _signalFlag = flag;

  if(!(*_signalFlag))
    *_signalFlag = 1;

  return 0; // success
}

int dtk_catchSignal(int sig, int *flag)
{
  if(!flag) return -1;
  
  signal(sig, _signalCatcher);

  _signalFlag = flag;

  if(!(*_signalFlag))
    *_signalFlag = 1;

  return 0; // success
}

#ifndef DTK_ARCH_WIN32_VCPP

// dtk_call()  from another thread
// call the func(data) in `seconds' seconds.

struct func_data
{
  void (*func)(void *);
  void *data;
};


static void *go(void *x)
{
  struct func_data *s = (struct func_data *) x;
  s->func(s->data);
  free(s);
  return NULL;
}


// From a seperate thread call func() in time in seconds.

int dtk_call(void (*func)(void *), void *data_to_func)
{
  if(func == NULL)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtk_call() failed:"
                 " invalid argument.\n");
      return -1;
    }

  struct func_data *s = (struct func_data *)
    malloc(sizeof(struct func_data));
  if(s == NULL)
    {
      dtkMsg.add(DTKMSG_FATAL,1,
		 "dtk_call() failed:"
                 " malloc() failed.\n");
      return -1;
    }

  s->func = func;
  s->data = data_to_func;

  pthread_t p;
  if(pthread_create(&p, NULL, go, (void *) s))
    {
      free(s);
      dtkMsg.add(DTKMSG_WARN,1,
		 "dtk_call() failed:"
                 " pthread_create() failed.\n");
      return -1;
    }
  return 0;
}



#endif /* #ifndef DTK_ARCH_WIN32_VCPP */
