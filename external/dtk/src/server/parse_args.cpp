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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dtk.h>

#include "if_windows.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"

#include "parse_args.h"


// from usage.cpp
extern int Usage(void);


static int print_env(void)
{
  FILE *file = dtkMsg.file();

  fprintf(file,
	  "\n"
	  "   The DIVERSE Toolkit server `dtk-server'"
	  " uses several environment variables.\n"
	  " Here is a list of all environment variables"
	  " that effect DTK and the DTK server\n"
	  "\n");

  dtk_printEnv(file);
  return 1;
}

static int print_about(int returnVal)
{
  FILE *file = dtkMsg.file();
  fprintf(file,"\n  ---- The DIVERSE Toolkit server `%s' ---- \n\n"
	  "version: %s%s%s\n"
	  "release date: %s\n"
	  "The DIVERSE Toolkit home-page: %s%s%s\n"
	  "compiled on the date: %s\n"
	  "by user: %s on host: %s\n\n"
	  
	  "\n"
	  "The DIVERSE Toolkit comes with ABSOLUTELY NO WARRANTY.\n"
	  "This is free software, and you are welcome to\n"
	  "redistribute it under certain conditions.\n"
	  "This program is Licensed under the GNU Lesser General Public License.\n"
	  "See http://www.gnu.org/copyleft/lesser.html"
	  " for License details.\n\n",
	  "dtk-server",
	  dtkMsg.color.vil,DTK_VERSION,dtkMsg.color.end,
	  DTK_RELEASE_DATE,
	  dtkMsg.color.tur,DTK_HOMEPAGE,dtkMsg.color.end,
	  DTK_COMPILE_DATE,
	  DTK_WHO_COMPILED, DTK_COMPILE_HOST);
  return returnVal;
}


// I need to process theses options before I load any services. In
// this case of augument syntex getopts() and getlongopts() will not
// work.

// Tf you add to and/or edit this, parse_nonLoading_args(), PLEASE
// edit parse_loading_args() to make them consistant.
// parse_loading_args() must parse some of the same arguments again.

int parse_nonLoading_args(int argc, char **argv,
			  int *isDaemon, int *timeout)
{
  int isVerbose = 0;
  for(int i=1;i<argc;)
    {
      char *str = NULL;

      /*********** options that don't run the server ************/
      if(!strncmp(argv[i],"--about",3))
	return print_about(1);
      else if(!strncmp(argv[i],"--env",3))
	return print_env();
      else if(!strncmp(argv[i],"--help",3))
	return Usage();
      else if(!strncmp(argv[i],"--version",6))
	{
	  fprintf(dtkMsg.file(),"%s\n",DTK_VERSION);
	  return 1;
	}
      /********************************************************/
      else if(!strncmp(argv[i],"--verbose",3))
	{
	  i++;
	  isVerbose = 1;
	}
      else if((str = dtk_getOpt("-t","--timeout",argc,(const char **) argv,&i)))
	{
	  *timeout = atoi(str);
	}
      else if(!strncmp(argv[i],"--daemon",3))
	{
	  i++;
	  *isDaemon = 1;
	}
      else if(!strncmp(argv[i],"--config",3))
	{
      i++;
	}
      // Get all the combination short arguments like: `dtk-server -vdt20' .
      else if(strncmp(argv[i],"--",2) && argv[i][0] == '-')
	{
	  int j = 1;
	  for(;argv[i][j];j++)
	    {
	      switch(argv[i][j])
		{
		case 'a':
		  break;
		case 'A':
		  return print_about(1);
		  break;
		case 'c':
		  break;
		case 'd':
		  *isDaemon = 1;
		  break;
		case 'e':
		  return print_env();
		  break;
		case 'H':
		case 'h':
		  return Usage();
		  break;
		case 'n':
		  break;
		case 't':
		  {
		    if(argv[i][j+1])
		      *timeout = atoi(&(argv[i][j+1]));
		    else if(argv[++i])
		      {
			*timeout = atoi(argv[i]);
			j = 0;
		      }
		    else // missing timeout value in seconds
		      {
			return Usage();
		      }
		    // cause it to break from for() loop
		    for(;argv[i][j];j++);
		    j--;
		  }
		  break;
		case 'V':
		  fprintf(dtkMsg.file(),"%s\n",DTK_VERSION);
		  return 1;
		  break;
		case 'v':
		  isVerbose = 1;
		  break;
		default:
		  return Usage();
		  break;
		}
	    }
	  i++;
	}
      else // Ignore arguments that load services.
	i++;
    }

  if(isVerbose && !getenv("DTK_SPEW_LEVEL") && !getenv("DTK_SPEW"))
    {
      putenv(const_cast<char*>("DTK_SPEW=debug"));
      dtkMsg.reset();
    }

  // fix the timeout
  if(*timeout < MIN_STARTUP_TIMEOUT)
    *timeout = MIN_STARTUP_TIMEOUT;
  else if(*timeout > MAX_STARTUP_TIMEOUT)
    *timeout = MAX_STARTUP_TIMEOUT;

  return 0;
}


int parse_loading_args(int argc, char **argv,
		       ServiceManager *serviceManager)
{
  char *name = 0;
  char *s_arg = 0;
  char *config = 0;
  // Load objects from command line arguments.
  // getopt() and the like would not do what I wanted.
  for(int i=1;i<argc;)
  {
    /********* ignore arguments that are already processed ********/
    /**************************************************************/
    if(!strncmp(argv[i],"--verbose",3))
      i++;
    else if((dtk_getOpt("-t","--timeout",argc,(const char **) argv,&i)));
    else if(!strncmp(argv[i],"--daemon",3)) i++;
    // exclude all the combination short arguments like: `dtk-server -vdt20' .
    else if(strncmp(argv[i],"--",2) && (argv[i][0] == '-' &&
        (argv[i][1] == 'd' || argv[i][1] == 't' || argv[i][1] == 'v') ) )
      i++;
    /********************** load DSO *******************************/
    /***************************************************************/
      else if( ( config = dtk_getOpt( "-c", "--config", argc, (const char **) argv, &i ) ) )
      {
      if((name = dtk_getOpt("-n","--name",argc,(const char **) argv,&i)))
        s_arg = dtk_getOpt("-a","--argument",argc,(const char **) argv,&i);
      else if((s_arg =
        dtk_getOpt("-a","--argument",argc,(const char **) argv,&i)))
      name = dtk_getOpt("-n","--name",argc,(const char **) argv,&i);
        
        if( name || s_arg )
        {
          dtkMsg.add( DTKMSG_ERROR, "The service and config options are mutually exclusive\n"
              "either load a service or load a config file\n" );
          return Usage();
        }
        if( serviceManager->loadConfig( config, NULL ) )
          return -1;
      }
    else
    {
      char *service_file= (char *) argv[i++];
      if((name = dtk_getOpt("-n","--name",argc,(const char **) argv,&i)))
        s_arg = dtk_getOpt("-a","--argument",argc,(const char **) argv,&i);
      else if((s_arg =
        dtk_getOpt("-a","--argument",argc,(const char **) argv,&i)))
      name = dtk_getOpt("-n","--name",argc,(const char **) argv,&i);
      else if(serviceManager->add(service_file,name,s_arg))
        return -1; // error
    }
  }
  return 0;
}
