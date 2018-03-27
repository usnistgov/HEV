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
/****** utils.h ******/

#include <algorithm>
#include <string>
#include <vector>

/*! \file
 *
 * \brief small utility functions
 *
 * These are some utility functions that don't do alot.
 */

// The DTK utility class.  It is mostly used internal to the DTK Library.

// Very light error spewing libc wrappers.  There do not exit() or
// anything stupid like that.  There spew to the object dtkMsg at
// DTK_SPEW=error
extern DTKAPI void *dtk_malloc(size_t size);
extern DTKAPI void dtk_free(void *ptr);
extern DTKAPI void *dtk_realloc(void *ptr, size_t size); 
extern DTKAPI char *dtk_strdup(const char *s);


/*! \fn void *dtk_swapBytes(void *data, size_t size);
 * \brief swap the bytes
 *
 * \param data pointer to the data whos data will be byte swapped.
 *
 * \param size length of data to be byte swapped.
 *
 * \return \e data the first argument.
 */
extern DTKAPI void *dtk_swapBytes(void *data, size_t size);

// Get an option argument at index *i;
// if found index the counter *i to the next string.

extern DTKAPI
char *dtk_getOpt(const char *shorT, const char *lonG, 
			     int argc, const char **argv, int *i);

/*! \fn void tokenize( const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " " );
 * \brief tokenize a string
 * \param str reference to a string to be parsed
 * \param tokens reference to a vector of string to store the tokens
 * \param delimiters a string containg the individual characters which delimit the tokens
 */
extern DTKAPI
void tokenize( const std::string& str, std::vector<std::string>& tokens,
      const std::string& delimiters = " " );


/*! \fn int dtk_catchSignal(const int *_signals, int *flag);
 * \brief unset flag when signal is caught
 *
 * dtk_catchSignal() sets up a signal catcher that will catch the list
 * of signals in \e signals.  When any of the signals are caught it
 * will set \e flag to zero and then return.  It's nothing more
 * than a way to write less code.  When working with shared memory
 * read/write locks are needed and if signals interupt a read/write
 * lock while it is being changed it can put the read/write lock in an
 * unusable state.
 *
 * When a signal is caught this will spew on level DTKSPEW_NOTICE.
 * See dtkMessage.  Calling this more than once will just add more
 * to the list.
 *
 * \param _signals a zero terminated list a signals.  See `kill -l' or
 * man signal(2).
 *
 * \param flag is an int that will get set to zero if any of the
 * signals in \e _signals gets caught.  The integer that \e flag
 * points to needs to persist.
 *
 * \return 0 on success, and -1 on error.
 *
 * \include dtk/tests/blockingRead/blockingQRead.cpp
 */

extern DTKAPI int dtk_catchSignal(const int *_signals, int *flag);

/*! \fn int dtk_catchSignal(const int sig, int *flag);
 * \brief unset flag when signal is caught
 *
 * dtk_catchSignal() sets up a signal catcher that will catch the
 * signal \e sig.  When the signal sig is caught it will set \e
 * flag to zero and then return.  It's nothing more than a way to
 * write less code.  When working with shared memory read/write locks
 * are needed and if signals interupt a read/write lock while it is
 * being changed it can put the read/write lock in an unusable state.
 *
 * When a signal is caught this will spew on level DTKSPEW_NOTICE.
 * See dtkMessage.  Calling this more than once will just add more
 * to the list.
 *
 * \param sig a signal.  See `kill -l' or man signal(2).
 *
 * \param flag is an int that will get set to zero if signal \e sig
 * gets caught.  The integer that \e flag points to needs to
 * persist.
 *
 * \return 0 on success, and -1 on error.
 *
 *  \include dtk/tests/blockingRead/blockingRead.cpp
 */

extern DTKAPI int dtk_catchSignal(int sig, int *flag);



// returns pointer to static memory.
extern DTKAPI char *dtk_getCommonAddressPort(const char *addressPort);

extern DTKAPI void *dtk_swapBytes(void *data, size_t size);


// file=NULL defaults to stdout
#ifdef DTK_ARCH_WIN32_VCPP
extern DTKAPI int dtk_printEnv(FILE *file=NULL, int is_color=DTKCOLOR_OFF);
#else
extern DTKAPI int dtk_printEnv(FILE *file=NULL, int is_color=DTKCOLOR_AUTO);
#endif


// dtk_splice() builds a data structure like
// { "str0", "str1", "str2", ... }
// The data (...) must be NULL terminated.
// returns length of data written on success.
// returns 0 on failure.

extern DTKAPI size_t dtk_splice(size_t length, char *buff, ...);



// Runs a thread to call in parallel with the current thread.  It's
// handy in case you are calling something that will block (hang) the
// current thread, and you can't afford to block it.
extern DTKAPI int dtk_call(void (*func)(void *), void *data_to_func);


// If path is NULL or "" it will look in the current working
// directory for the file, else it will not.
// dtk_getFileFromPath() uses malloc() to get the returned string.
// Use free() when your done with it.
// dtk_getFileFromPath() returns NULL on error or if no file is found.
// dtk_getFileFromPath() adds to dtkErr on error.
// Returns a string that is a full path to the file.
extern DTKAPI char *dtk_getFileFromPath(const char *path_in,
				 const char *file, mode_t mode=0);


// dtk_tempname() makes temp file name.  dtk_tempname() uses stat() to
// make sure that the file does not exist.  Use it with
// dtk_tempname("/tmp/ProgramName") for example.  It uses
// gettimeofday() to get a number since gettimeofday() gets a
// different number every time it's called.  Use free() on the
// returned pointer.
extern DTKAPI char *dtk_tempname(const char *pre);

// Utility function to convert string to integer
extern DTKAPI int convertStringToNumber( int& value, const char* str );
// Utility function to convert string to integer
extern DTKAPI int convertStringToNumber( long& value, const char* str );
// Utility function to convert string to integer
extern DTKAPI int convertStringToNumber( float& value, const char* str );
// Utility function to convert string to integer
extern DTKAPI int convertStringToNumber( double& value, const char* str );
// Utility function to make string lower case
extern DTKAPI std::string& tolower( std::string& str );


#if(0)
/* IRIX 6.5 has a nasty bug in tmpnam() and tempnam()
 * It appears that these functions overwrite their string
 * arrays or something.  Using them screws up my code.
 * For example:   use IRIX CC to compile
 */

#include <stdio.h>
#include <errno.h>
int main(void)
{
  errno = 0;
  if(tmpnam(NULL) == NULL) return 1;
  printf("errno=%d\n",errno);
  return 0;
}

/*
 * When I compile and run the above program
 * it prints to stdout: errno=2
 * assuming printf() is not screwed up then
 * tmpnam() is screwed up.
 */
#endif
/* So I say roll your own. */
