/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <dtk.h>
#include <dtk/dtkFilter_loader.h>

#define OUTPUT_FILE "forward"

struct ForwardList
{
  dtkSharedMem *shm;
  size_t *fromBytes, *fromOffset;
  size_t *toBytes, *toOffset;
  char *copyBuffer;
};


class forward : public dtkFilter
{
public:

  forward(size_t size, int argc, const char **argv);
  ~forward(void);

  int write(void *buf, size_t bytes, size_t offset,
	    struct timeval *timeStamp);

private:

  int Usage(void);
  int parse_args(size_t size, int argc, const char **argv);
  int get_byteList(const char *option, const char *arg, size_t **bytes, size_t **offset);
  int badOpt(const char *option, const char *arg, char *freeMe=NULL);

  ForwardList *forwardList;
  int listSize;
};


int forward::Usage(void)
{
  dtkMsg.add(DTKMSG_NOTICE,
	     "\n"
	     "  Usage: "OUTPUT_FILE" TO_FILE [-f|--from-bytes BYTELIST] [TO_FILE [-f|--from-bytes BYTELIST]] ...\n"
	     "\n"
	     "  Load a write filter that forwards data to others shared memory files.  FILE is the file name\n"
	     "of a shared memory file to forward data to.\n"
	     "\n"
	     "  -f|--from-bytes BYTELIST   write from the bytes listed in BYTELIST from the shared memory file FROM_FILE\n"
	     "                             BYTELIST is of the form N,M,.. or N-M where N and M are integers starting with zero.\n"
	     "                             For example the arguments `foo -f 0,3-5,14' would cause this filter to write\n"
	     "                             the first byte and 4th to the 6th byte and the 15th byte to the shared memory file \"foo\"\n"
	     " -t|--to-bytes BYTELIST      write to the bytes in BYTELIST in the order that the bytes are listed in BYTELIST\n"
	     "                             For example the arguments `-t 0,3-8' would cause this filter to write to the first byte\n"
	     "                             and than the 4th byte to the 9th byte of the TO_FILE.\n"
	     "                             The BYTELIST's in the --from-bytes and --to-bytes options must contain the same number\n"
	     "                             of bytes in each of the BYTELISTs.\n"
	     "\n");
  return -1;
}

int forward::badOpt(const char *option, const char *arg, char *freeMe)
{
  if(freeMe) dtk_free(freeMe);
  dtkMsg.add(DTKMSG_WARN, "shared memory write filter \""OUTPUT_FILE"\"\n"
	     "bad \"%s\" option \"%s\"\n", option,arg);
  return Usage();
}

int forward::get_byteList(const char *option, const char *arg, size_t **bytes, size_t **offset)
{
  char *s = dtk_strdup(arg);
  char *c = s;
  int count=0;
#define IS_DIGIT(x)  ((x) >= '0' || (x) <= '9')
  while(*c)
    {
      if(!IS_DIGIT(*c))
	return badOpt(option, arg, s);

      while(IS_DIGIT(*c)) c++;

      if(*c != '\0' && *c != ','  && *c != '-')
	return badOpt(option, arg, s);
      else if(*c == '-')
	{
	  c++;
	  if(!IS_DIGIT(*c))
	    return badOpt(option, arg, s);
	  while(IS_DIGIT(*c)) c++;
	}

      count++;
      if(*c) c++;
    }

  if(count)
    {
      *bytes = (size_t *) dtk_malloc((count+1)*sizeof(size_t));
      *offset = (size_t *) dtk_malloc((count+1)*sizeof(size_t));
    }
  else
    return badOpt(option, arg, s);

  int i = 0;
  // initialize
  for(i=0;i<count+1;i++)
    {
      (*bytes)[i] = 0;
      (*offset)[i] = 0;
    }

  i = 0;
  c = s;
  while(*c)
    {
      char *num = c;
      while(IS_DIGIT(*c)) c++;
      if(*c == ',' || *c == '\0')
	{
	  if(*c)
	    {
	      *c = '\0';
	      c++;
	    }
	  (*bytes)[i] = 1;
	  (*offset)[i] = strtoul(num, (char **) NULL, 10);
	  if((*offset)[i] == (unsigned long) -1)
	    return badOpt(option, arg, s);
	}
      else if(*c == '-')
	{
	  *c = '\0';
	  c++;
	  (*offset)[i] = strtoul(num, (char **) NULL, 10);
	  if((*offset)[i] == (unsigned long) -1)
	    return badOpt(option, arg, s);
	  num = c;
	  while(IS_DIGIT(*c)) c++;
	  if(*c)
	    {
	      *c = '\0';
	      c++;
	    }
	  (*bytes)[i] = strtoul(num, (char **) NULL, 10);
	  if((*bytes)[i] == (unsigned long) -1 ||
	     (*bytes)[i] == (size_t) 0 || (*offset)[i] > (*bytes)[i])
	    return badOpt(option, arg, s);
	  (*bytes)[i] -= (*offset)[i];
	}
      else
	return badOpt(option, arg, s);
      i++;
    }
  
  dtk_free(s);
  return 0;
}


int forward::parse_args(size_t size, int argc, const char **argv)
{
  int i;
  for(i=0;i<argc;i++)
    if(!strcmp(argv[i], "--help") || !strncmp(argv[i], "-h",2))
      return Usage();

  
  if(argc < 1) return Usage();

  // The first augment must be a TO_FILE shared memory file.

  for(i=0;i<argc;)
    {
      if(dtk_getOpt("-f","--from-bytes",argc,argv,&i) ||
	 dtk_getOpt("-t","--to-bytes",argc,argv,&i))
	return Usage();
      else
	{
	  listSize++;
	  i++;
	}

      if(i<argc && dtk_getOpt("-f","--from-bytes",argc,argv,&i) &&
	 i<argc && dtk_getOpt("-t","--to-bytes",argc,argv,&i));
      else if(i<argc && dtk_getOpt("-t","--to-bytes",argc,argv,&i) &&
	      i<argc && dtk_getOpt("-f","--from-bytes",argc,argv,&i));
    }

  if(listSize == 0) return Usage();

  forwardList = (struct ForwardList *)
    dtk_malloc(listSize*sizeof(struct ForwardList));
  for(i=0;i<listSize;i++)
    {
      forwardList[i].shm = NULL;
      forwardList[i].fromBytes = NULL;
      forwardList[i].fromOffset = NULL;
      forwardList[i].toBytes = NULL;
      forwardList[i].toOffset = NULL;
      forwardList[i].copyBuffer = NULL;
    }

  int j = -1;
  for(i=0;i<argc;)
    {
      char *str;
      if((str = dtk_getOpt("-f","--from-bytes",argc,argv,&i)))
        {
	  if(get_byteList("-t|--to-bytes", str, &(forwardList[j].fromBytes),
			  &(forwardList[j].fromOffset)))
	    return -1; // error				      
        }
      else if((str = dtk_getOpt("-t","--to-bytes",argc,argv,&i)))
	{
	  if(get_byteList("-t|--to-bytes", str, &(forwardList[j].toBytes),
			  &(forwardList[j].toOffset)))
	    return -1; // error	
	}
      else
	{
	  j++;
	  forwardList[j].shm = new dtkSharedMem(argv[i]);
	  if(forwardList[j].shm->isInvalid())
	    {
	      delete forwardList[j].shm;
	      forwardList[j].shm = NULL;
	      return Usage();
	    }
	  i++;
	}
    }

  // Check the list of bytes and offsets to see that the sizes are OK.
  for(i=0;i<listSize;i++)
    {
      size_t fromSize=0;
      if(forwardList[i].fromBytes)
	{
	 int k;
	 for(k=0;forwardList[i].fromBytes[k]; k++)
	   {
	     if(forwardList[i].fromBytes[k] + forwardList[i].fromOffset[k] > getSize())
	       {
		 dtkMsg.add(DTKMSG_WARN, "shared memory write filter \""OUTPUT_FILE"\"\n"
			    "shared memory is of size of only %d bytes.\n",
			    getSize());
		 return Usage();
	       }
	     fromSize += forwardList[i].fromBytes[k];
	   }
	 if(fromSize > forwardList[i].shm->getSize())
	   {
	     dtkMsg.add(DTKMSG_WARN, "shared memory write filter \""OUTPUT_FILE"\"\n"
			"shared memory \"%s\" is of size of only %d bytes.\n",
			forwardList[i].shm->getName(), forwardList[i].shm->getSize());
	     return Usage();
	   }
       }

      size_t toSize=0;
      if(forwardList[i].toBytes)
	{
	 int k;
	 for(k=0;forwardList[i].toBytes[k]; k++)
	   {
	     if(forwardList[i].toBytes[k] + forwardList[i].toOffset[k] > forwardList[i].shm->getSize())
	       {
		 dtkMsg.add(DTKMSG_WARN, "shared memory write filter \""OUTPUT_FILE"\"\n"
			    "shared memory is of size of only %d bytes.\n",
			    forwardList[i].shm->getSize());
		 return Usage();
	       }
	     toSize += forwardList[i].toBytes[k];
	   }
	 if(!forwardList[i].fromBytes && toSize > getSize())
	   {
	     dtkMsg.add(DTKMSG_WARN, "shared memory write filter \""OUTPUT_FILE"\"\n"
			"shared memory is of size of only %d bytes.\n",
			getSize());
	     return Usage();
	   }
	 else if(forwardList[i].fromBytes && fromSize != toSize)
	   {
	     dtkMsg.add(DTKMSG_WARN, "shared memory write filter \""OUTPUT_FILE"\"\n"
			"inconsistent \"-f|--from-bytes\" and \"-t|--from-bytes\" options.\n");
	     return Usage();
	   }
	}
      else if(forwardList[i].fromBytes) // && !forwardList[i].toBytes
	{
	  forwardList[i].toBytes = (size_t *) dtk_malloc(2*sizeof(size_t));
	  forwardList[i].toOffset = (size_t *) dtk_malloc(2*sizeof(size_t));
	  forwardList[i].toBytes[0] = fromSize;
	  forwardList[i].toBytes[1] = 0;
	  forwardList[i].toOffset[0] = 0;
	  forwardList[i].toOffset[1] = 0;		       
	}

      if(!forwardList[i].fromBytes && forwardList[i].toBytes)
	{
	  forwardList[i].fromBytes = (size_t *) dtk_malloc(2*sizeof(size_t));
	  forwardList[i].fromOffset = (size_t *) dtk_malloc(2*sizeof(size_t));
	  forwardList[i].fromBytes[0] = toSize;
	  forwardList[i].fromBytes[1] = 0;
	  forwardList[i].fromOffset[0] = 0;
	  forwardList[i].fromOffset[1] = 0;		       
	}
      else if(!forwardList[i].fromBytes && !forwardList[i].toBytes)
	{
	  if(getSize() != forwardList[i].shm->getSize())
	    {
	      dtkMsg.add(DTKMSG_WARN, "shared memory write filter \""OUTPUT_FILE"\"\n"
			 "shared memory is of size of %d bytes and "
			 "forwarding shared memory \"%s\" is of size %d.\n",
			 getSize(), forwardList[i].shm->getName(), forwardList[i].shm->getSize());
	      return Usage();
	   }
	  forwardList[i].fromBytes = (size_t *) dtk_malloc(2*sizeof(size_t));
	  forwardList[i].fromOffset = (size_t *) dtk_malloc(2*sizeof(size_t));
	  forwardList[i].fromBytes[0] = getSize();
	  forwardList[i].fromBytes[1] = 0;
	  forwardList[i].fromOffset[0] = 0;
	  forwardList[i].fromOffset[1] = 0;		       
	  forwardList[i].toBytes = (size_t *) dtk_malloc(2*sizeof(size_t));
	  forwardList[i].toOffset = (size_t *) dtk_malloc(2*sizeof(size_t));
	  forwardList[i].toBytes[0] = getSize();
	  forwardList[i].toBytes[1] = 0;
	  forwardList[i].toOffset[0] = 0;
	  forwardList[i].toOffset[1] = 0;
	}
    }

   for(i=0;i<listSize;i++)
     {
       size_t size = (forwardList[i].shm->getSize() > getSize())? forwardList[i].shm->getSize(): getSize();
       forwardList[i].copyBuffer = (char *) dtk_malloc(size);
     }

  return 0;
}


forward::forward(size_t size, int argc, const char **argv) : dtkFilter(size)
{
  listSize = 0;
  forwardList = NULL;

  if(parse_args(size, argc, argv)) return; // error

  // debug spew.
  dtkMsg.add(DTKMSG_DEBUG, "Loaded shared memory write filter \"%s\" which will write to:\n"
	     "shared memory file(s):", OUTPUT_FILE);
  int i;
  for(i=0;i<listSize;i++)
    {
      dtkMsg.append("\n\"%s\": FROM:", forwardList[i].shm->getName());
      int k;
      for(k=0;forwardList[i].fromBytes[k]; k++)
	dtkMsg.append(" %d bytes with %d offset",
		      forwardList[i].fromBytes[k],
		      forwardList[i].fromOffset[k]);
      dtkMsg.append(" TO:");
      for(k=0;forwardList[i].toBytes[k]; k++)
	dtkMsg.append(" %d bytes with %d offset",
		      forwardList[i].toBytes[k],
		      forwardList[i].toOffset[k]);
    }
  dtkMsg.append(".\n");

  validate();
}


forward::~forward(void)
{
  if(forwardList)
    {
      int i;
      for(i=0;i<listSize;i++)
	{
	  if(forwardList[i].shm)
	    delete forwardList[i].shm;
	  if(forwardList[i].fromBytes)
	    dtk_free(forwardList[i].fromBytes);
	  if(forwardList[i].fromOffset)
	    dtk_free(forwardList[i].fromOffset);
	  if(forwardList[i].toBytes)
	    dtk_free(forwardList[i].toBytes);
	  if(forwardList[i].toOffset)
	    dtk_free(forwardList[i].toOffset);
	  if(forwardList[i].copyBuffer)
	    dtk_free(forwardList[i].copyBuffer);
	}
      dtk_free(forwardList);
      forwardList = NULL;
    }
}

int forward::write(void *buf, size_t bytes, size_t offset,
		    struct timeval *timeStamp)
{
  //printf("file=%s line=%d\n",__FILE__,__LINE__);
  if(bytes != getSize() && offset != 0) return dtkFilter::REMOVE;
  int i;
  for(i=0;i<listSize;i++)
    {
      int j;
      char *buffer = (char *) buf;
      if(forwardList[i].shm->wlock()) return dtkFilter::ERROR_;
      for(j=0;forwardList[i].fromBytes[j];j++)
	{
	  if(forwardList[i].shm->write(&(buffer[forwardList[i].fromOffset[j]]),
				       forwardList[i].fromBytes[j],
				       forwardList[i].toOffset[j]))
	    {
	      if(forwardList[i].shm->wunlock())
		return dtkFilter::ERROR_;
	    }
	}
      if(forwardList[i].shm->wunlock()) return dtkFilter::ERROR_;
    }
 
  return dtkFilter::CONTINUE; // success
}


/************** All dtkFilter DSOs define these two functions ****************
 *
 * If you wish to build your filter DSO from more than one file and
 * you wish to not define dtkFilter_loader and/or dtkFilter_unloader
 * in another source file then you may then define
 * __DTKFILTER_NOT_LOADED__ before including the
 * dtk/dtkFliter_loader.h in that source file.
 *
 *****************************************************************************/


static dtkFilter *dtkFilter_loader(size_t size, int argc, const char **argv)
{
  return new forward(size, argc, argv);
}

static int dtkFilter_unloader(dtkFilter *f)
{
  delete f;
  return DTKFILTER_UNLOAD_CONTINUE;
}
