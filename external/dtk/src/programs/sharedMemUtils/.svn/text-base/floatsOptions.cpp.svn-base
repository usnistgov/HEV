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
#include <stdio.h>
#include <dtk.h>

#ifndef HEADER_FILE
#  define HEADER_FILE "floatsOptions.h"
#endif
#include HEADER_FILE

int running = 1;

static char EndChar;

int Usage(void)
{
  printf("\n"
	 "  Usage: %s FILE_NAME [-c|--count] [-f|--fields \"N0 N1 N2 ...\"]\n"
	 "    [-n|--number NUM] [-r|--carrage-return] [-q|--queue] \n"
	 "    [-t|--time-stamp]| [-h|--help]\n"
	 "\n"
	 "   Read from DTK shared memory than print (ascii) data to standard out.  FILE_NAME is\n"
	 " is the name the DTK shared memory file to read from.\n"
	 "\n"
	 "   OPTIONS\n"
	 "\n"
	 " -c|--count            print a count of the number of reads\n"
	 " -f|--fields \"N0 N1 N2 ...\"  to print just fields \"N0 N1 N2 ...\"\n"
	 "                       fields are numbered from 0\n"
	 " -h|--help             print this help and than exit\n"
	 " -n|--number NUM       print just NUM points and then exit\n"
	 " -r|--carrage-return   print a carrage return instead of new line after each sample\n"
	 " -q|--queue            queue the shared memory and read from the queue\n"
	 " -t|--time-stamp       add a time stamp\n\n",
	 PROGRAM_NAME);
  return 1;
}


int parse_args(int argc, char **argv, char **name, int **field,
	       int *number, char *endchar, int *count,
	       int *queue, int *time)
{
  for(int i=1;i<argc;)
    {
      char *str;
      if(!strcmp(argv[i],"--carrage-return"))
        {
          *endchar = '\r';
          i++;
        }
      else if(!strcmp(argv[i],"--count"))
        {
          *count = 1;
          i++;
        }
      else if(!strcmp(argv[i],"--help"))
        return Usage();
      else if(!strcmp(argv[i],"--queue"))
        {
          *queue = 1;
          i++;
        }
      else if(!strcmp(argv[i],"--time-stamp"))
        {
          *time = 1;
          i++;
        }
      else if((str = dtk_getOpt("-n","--number",argc,(const char **)argv,&i)))
        *number=atoi(str);
      else if((str = dtk_getOpt("-f","--fields",argc,(const char **)argv,&i)))
	{
	  // count the number of fields
	  int count=0;
	  char *string=str;
	  for(;*string;)
	    {
	      while(*string == ' ' || *string == '\t' || *string == ',')
		string++;
	      if(*string == '\0')
		break;
	      if(*string != ' ' && *string != '\t' && *string != ',')
		count++;
	      while(*string != ' ' && *string != '\t' &&
		    *string != ',' && *string)
		string++;
	    }
	  *field = (int *) dtk_malloc(sizeof(int)*(count + 1));
	  if(*field == NULL)
	    return Usage();
	  string=str;
	  for(int *i=*field;*string;i++)
	    {
	      while(*string == ' ' || *string == '\t' || *string == ',')
		string++;
	      sscanf(string,"%d",i);
	      while(*string != ' ' && *string != '\t' &&
		    *string != ',' && *string)
		string++;
	    }
	  (*field)[count] = -1; // marks the end of the array.
	}
      else if(argv[i][0] == '-' && argv[i][1] != '-')
	{
	  // parse options grouped like: -qt = -q -t .
	  char *s = &(argv[i][1]);
	  for(;*s;s++)
	    {
	      switch (*s)
		{
		case 'c':
		  *count = 1;
		  break;
		case 'r':
		  *endchar = '\r';
		  break;
		case 'q':
		  *queue = 1;
		  break;
		case 't':
		  *time = 1;
		  break;
		case 'h':
		default:
		  return Usage();
		  break;
		}
	    }
	  i++;
	}
      else
        *name = argv[i++];
    }

  if(!(*name)) return Usage();

  dtkMsg.setPreMessage(PROGRAM_NAME);

  EndChar = *endchar;

  return 0;
}


void sig_catcher(int sig)
{
  if(EndChar == '\r')
    dtkMsg.setPreMessage("\n"PROGRAM_NAME);
  dtkMsg.add(DTKMSG_INFO, "caught signal %d ... exiting\n",sig);
  running = 0;
}
