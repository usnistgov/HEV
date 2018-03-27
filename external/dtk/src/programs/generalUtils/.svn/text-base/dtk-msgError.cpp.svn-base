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
#include <dtk.h>
#include <string.h>
#include <stdio.h>

#ifndef PROGRAM_NAME
#  define PROGRAM_NAME "dtk-msgError"
#  define LEVEL  DTKMSG_ERROR
#endif


int main(int argc, char **argv)
{
  if(argc < 2 || !strcmp("-h",argv[1]) || !strcmp("--help",argv[1]))
    {
      printf("\n"
	     "Usage: %s [-p|--prepend PRE] string0 string1 ... | [-h|--help]\n\n"
	     "  Print using a dtkMessage object.\n\n"
	     "  OPTIONS\n"
	     "  -h|--help     Print this help and than exit\n"
	     "  -p|--pre PRE  Prepend PRE to the begining of the message\n"
	     "\n",
	     PROGRAM_NAME);
      return 1;
    }

  int i;
  char *pre = NULL;
  for(i=1;i<argc;i++)
    if((pre = dtk_getOpt("-p", "--prepend", argc,
			 (const char **) argv, &i)))
      break;

  if(pre) dtkMsg.setPreMessage(pre);


  for(i=1;i<argc;)
    if(dtk_getOpt("-p", "--prepend", argc,
		  (const char **) argv, &i));
    else
      {
	dtkMsg.add(LEVEL,"%s ", argv[i]);
	break;
      }

  for(i++;i<argc;)
    if(dtk_getOpt("-p", "--prepend", argc,
		  (const char **) argv, &i));
    else
      dtkMsg.append("%s ", argv[i++]);
  dtkMsg.append("\n");

  return 0;
}
