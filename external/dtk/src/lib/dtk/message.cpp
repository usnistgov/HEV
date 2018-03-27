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

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "dtkColor.h"
#include "dtkMessage.h"

#define _DTKMSG_DEFAULT_FILE  stdout

#define MAX_SHIFT  64

static int get_level_from_env(const char *env, int default_value)
{
  char *str = getenv(env);
  if(str)
    {
      if(!strncasecmp("debug",str,(size_t)1))
	return DTKMSG_DEBUG;
      if(!strncasecmp("info",str,(size_t)1))
        return DTKMSG_INFO;
      if(!strncasecmp("notice",str,(size_t)3))
	return DTKMSG_NOTICE;
      if(!strncasecmp("warn",str,(size_t)1))
        return DTKMSG_WARN;
      if(!strncasecmp("error",str,(size_t)1))
	return DTKMSG_ERROR;
      if(!strncasecmp("fatal",str,(size_t)1))
	return DTKMSG_FATAL;
      if(!strncasecmp("none",str,(size_t)1) ||
	 !strncasecmp("off",str,(size_t)1))
	return DTKMSG_NONE;

      int value = (int) strtol(str, (char **)NULL, 0);
      if(value > DTKMSG_NONE)
	return DTKMSG_NONE;
      if(value <= DTKMSG_NONE || value >= DTKMSG_DEBUG)
	return value;
      if(value < DTKMSG_DEBUG)
	return DTKMSG_DEBUG;
    }
  return default_value;
}

dtkMessage::~dtkMessage(void) {}

int dtkMessage::getSysErrno(void)
{
  return sysErrno;
}

// stuff to set for reset or initialize

void dtkMessage::shift(int add_count)
{
  shiftCount += add_count;
  if(shiftCount < 0)
    shiftCount = 0;
  else if(shiftCount > MAX_SHIFT)
    shiftCount = MAX_SHIFT;
}

void dtkMessage::resetShift(void)
{
  shiftCount = 0;
}

void dtkMessage::unshift(int remove_count)
{
  shiftCount =- remove_count;
  if(shiftCount < 0)
    shiftCount = 0;
  else if(shiftCount > MAX_SHIFT)
    shiftCount = MAX_SHIFT;
}

void dtkMessage::setPreMessage(const char *format, ...)
{
  if(!format) { setDefaultPreString(); return; }
  va_list ap;
  va_start(ap, format);
  vsnprintf (preMessage_str, PRESTR_SIZE-2, format, ap);
  va_end(ap);

  int len = strlen(preMessage_str);
  if(preMessage_str[len-1] != ' ')
    {
      preMessage_str[len] = ' ';
      preMessage_str[len+1] = '\0';
    }
  //printf("preMessage_str=%s\n",preMessage_str);
}


void dtkMessage::setDefaultPreString(void)
{
  setPreMessage("%s%s_%sD%sT%sK%s_%s ",
		color.und,
		color.yel,//_
		color.red,//D
		color.yel,//T
		color.red,//K
		color.yel,//_
		color.end);
}


dtkMessage::dtkMessage(int print_on_severity_in, FILE *file_in, int is_color)
{
  _off = 0;
  shiftCount = 0;
  reset(print_on_severity_in, file_in, is_color);
  setDefaultPreString();
}

void dtkMessage::reset(int print_on_severity_in, FILE *file_in, int is_color)
{
  if(file_in) _file = file_in;
  else _file = _DTKMSG_DEFAULT_FILE;

  char *env_str;

  if(!(env_str = getenv("DTK_SPEW_FILE")))
    if(!(env_str = getenv("DTK_MSG_FILE")))
      env_str = getenv("DTK_MESSAGE_FILE");

  if(env_str && (!strncasecmp(env_str, "stderr", (size_t) 4) ||
		 !strcmp(env_str, "2")))
    _file = stderr;
  else if(env_str && (!strncasecmp(env_str, "stdout", (size_t) 4) ||
		 !strcmp(env_str, "1")))
    _file = stdout;

  color.reset(_file, is_color);

  shiftCount = 0;
  sysErrno = errno = 0;   // reset system error number

  print_on_severity = get_level_from_env("DTK_SPEW", -23432);
  if(print_on_severity == -23432)
    print_on_severity =
      get_level_from_env("DTK_SPEW_LEVEL", print_on_severity_in);

  last_severity = -3216;
}

int dtkMessage::add(int severity, int isSysErr,
		    int returnVal, const char *format, ...)
{
  if(_off) return returnVal;

  if(errno && isSysErr)
    sysErrno = errno;
  last_severity = severity;
  if(severity < print_on_severity || !format || format[0]=='\0')
    return returnVal;
  va_list args;
  va_start(args, format);
  vadd(severity, isSysErr, format, args);
  va_end(args);
  return returnVal;  
}

void dtkMessage::add(int severity, const char *format, ...)
{
  if(_off) return;

  last_severity = severity;
  if(severity < print_on_severity || !format || format[0]=='\0')
    return;
  va_list args;
  va_start(args, format);
  vadd(severity, 0, format, args);
  va_end(args);
}

void dtkMessage::add(int severity, int isSysErr, const char *format, ...)
{
  if(_off) return;

  if(errno && isSysErr)
    sysErrno = errno;
  last_severity = severity;
  if(severity < print_on_severity || !format || format[0]=='\0')
    return;
  va_list args;
  va_start(args, format);
  vadd(severity, isSysErr, format, args);
  va_end(args);
}

void dtkMessage::vadd(int severity, int isSysErr,
		      const char *format, va_list args)
{
  switch(severity)
    {
    case DTKMSG_FATAL:
      fprintf(_file, "%s%s FATAL %s: ",preMessage_str,color.rred,color.end);
      break;
    case DTKMSG_ERROR:
      fprintf(_file, "%s%s ERROR %s: ",preMessage_str,color.rred,color.end);
      break;
    case DTKMSG_WARN:
      fprintf(_file, "%s%sWARNING%s: ",preMessage_str,color.rblu,color.end);
      break;
    case DTKMSG_NOTICE:
      fprintf(_file, "%s%s NOTICE%s: ",preMessage_str,color.grn,color.end);
      break;
    case DTKMSG_INFO:
      fprintf(_file, "%s%s  INFO %s: ",preMessage_str,color.vil,color.end);
      break;
    case DTKMSG_DEBUG:
      fprintf(_file, "%s%s DEBUG %s: ",preMessage_str,color.yel,color.end);
      break;
    }

  if(shiftCount)
    {
      for(int i=0;i<shiftCount;i++)
	fprintf(_file, " ");
    }

  if(errno && isSysErr && _file) // append system error info
    fprintf(_file, "%s(%ssys error number %d%s)%s %s%s\n",
	    color.red,color.tur, errno,
	    color.red,color.tur,
	    strerror(errno), color.end);

  if(_file)
    vfprintf (_file, format, args);
}

void dtkMessage::append(const char *format, ...)
{
  if(_off) return;

  if(last_severity < print_on_severity || !format || format[0]=='\0')
    return;
  va_list args;
  va_start(args, format);
  vfprintf(_file, format, args);
  va_end(args);
}

void dtkMessage::off(void)
{
  _off = 1;
}

void dtkMessage::on(void)
{
  _off = 0;
}

void dtkMessage::setFile(FILE *file)
{
  if(file) _file = file;
}

// Global object in DTK
dtkMessage dtkMsg;
