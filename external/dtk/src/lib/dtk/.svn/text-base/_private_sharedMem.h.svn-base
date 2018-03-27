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
//! bit masks used in struct dtkSharedMem_header::flag which is defined
//! in sharedMem.h

#define COUNTING      01
#define TIMING        02
#define QUEUING       04
#define NETWORK_TYPE 010 //! set=TCP  unset=UDP add more later
#define BLOCKING_RD  020 // There is a blocking reader reading


//! Bitmask flags that go with each queued event in shared memory.
#define DTK_BYTE_ORDER 01 //! on is net byte order, off is not.



//! Decleared in sharedMem.cpp for all objects the byte_order is the same.
extern unsigned int dtkSharedMem_byte_order;


#define CHUNK_SIZE      8

#define SIZE_PLUS(x)  ((u_int32_t) ((((u_int32_t) (x)) + ((u_int32_t) CHUNK_SIZE) \
                      - ((u_int32_t) 1))\
		      /((u_int32_t) CHUNK_SIZE))* ((u_int32_t) CHUNK_SIZE))


extern int dtk_recMkdir(const char *dir, mode_t mode);


#define FILTER_WAITING_TO_BE_LOADED ((dtkFilter *) -1)
#define FILTER_NOT_USED             ((dtkFilter *) -2)



//! lock_states 0
#define IS_NOTLOCKED   0 //! don't change from 0
#define IS_RLOCKED     1
#define IS_WLOCKED     2

#define COUNTING_PSIZE  SIZE_PLUS(sizeof(u_int64_t))
#define TIMING_PSIZE    SIZE_PLUS(sizeof(struct timeval))
#define QUEUING_PSIZE   SIZE_PLUS(sizeof(u_int64_t))



#ifdef DTK_ARCH_WIN32_VCPP

# define DIR_SEP  "\\"
# define DIR_CHAR  '\\'
# define PATH_SEP  ';'
# define IS_FULL_PATH(x)   ((x) && ((x)[0] == '\\' || \
	                       (strlen(x) >= 3 && (x)[1] == ':' && \
	                        (x)[2] == '\\')))
# define bzero(x,y)  memset((x), 0, (y))
# define S_ISREG(x)  ((x) & _S_IFREG)
# define S_ISDIR(x)  ((x) & _S_IFDIR)

#else

# define DIR_SEP  "/"
# define DIR_CHAR  '/'
# define PATH_SEP  ':'
# define IS_FULL_PATH(x)  ((x) && ((x)[0] == '/' && (x)[1]))
#endif


/**********************************************************************
 *********** used for filter_flags in private in dtkSharedMem *********
 **********************************************************************/

#define ALLOW_READ_FILTER            001
#define IS_READ_FILTER               002
#define IS_READ_FILTER_QUEUED        004

#define ALLOW_WRITE_FILTER           010
#define IS_WRITE_FILTER              020
#define IS_WRITE_FILTER_TIMESTAMPED  040


