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
// Uses DTK_DSO_PATH and DTK_DSO_FILES
//
// env DTK_DSO_PATH=`dtk-config --root`/etc/dtkAugment_DSO dtk-augmentManager
//
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <dtk.h>

#define PROGRAM_NAME   "dtk-augmentManager"

int running = 1;

static int Usage(void)
{
  dtkConfigure config;
  dtkColor c(stdout);
  printf("\nUsage: "PROGRAM_NAME" [DSO_FILES ...] [-h|--help]\n"
	 "  Load DSO_FILES and run with the a dtkManager object.\n"
	 "\n"
	 "  "PROGRAM_NAME" can be used build programs from pieces of code\n"
	 "  that are in the form of loadable dynamic shared objects (DSOs) that\n"
	 "  typically have a C++ dtkAugment object as the primary interface, and\n"
	 "  a standard DTK form.  These DSOs can be mixed and matched to construct\n"
	 "  different programs at run-time.\n"
	 "\n"
	 "  ---- environment variables ---\n"
	 "    %sDTK_DSO_PATH%s is used for setting the search path to DSO files.\n"
	 "                 %sDTK_DSO_PATH%s: is prepended to the default path\n"
	 "                 which is %s.:%s%s.\n"
	 "   %sDTK_DSO_FILES%s is used to list DSOs to load in a colon\n"
	 "                 seperated list.  DSO files from the command\n"
	 "                 line arguments override %sDTK_DSO_FILES%s.\n\n",
	 c.tur,c.end, c.tur,c.end,
	 c.grn, config.getString(dtkConfigure::AUGMENT_DSO_DIR), c.end,
	 c.tur,c.end, c.tur,c.end);
  return 1;
}


static void catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO,"caught signal %d: exiting"
	     " after this frame.\n", sig);
  running = 0;
}


int main(int argc, char **argv)
{
  char *files = getenv("DTK_DSO_FILES");

  if(argc > 1)
    {
      int i=1;
      for(;i<argc;i++)
	if(!strcmp("-h", argv[i]) || !strcmp("--help", argv[i]))
	  return Usage();
    }

  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.grn,
                       PROGRAM_NAME, dtkMsg.color.end);
  dtkManager m;
  if(m.isInvalid()) return 1; // error

  { // Set the DSO search PATH.

    dtkConfigure c;
    char *env = getenv("DTK_DSO_PATH");
    if(env)
      {
	char *format = "%s:.:%s";
	char *path = (char *)
	  dtk_malloc(strlen(format) + strlen(env) +
		     strlen(c.getString(dtkConfigure::AUGMENT_DSO_DIR)));
	sprintf(path, format, env, c.getString(dtkConfigure::AUGMENT_DSO_DIR));
	m.path(path);
      }
    else
      {
	char *format = ".:%s";
	char *path = (char *)
	  dtk_malloc(strlen(format) +
		     strlen(c.getString(dtkConfigure::AUGMENT_DSO_DIR)));
	sprintf(path, format, c.getString(dtkConfigure::AUGMENT_DSO_DIR));
	m.path(path);
      }  
  }

  signal(SIGINT,catcher);
  signal(SIGTERM,catcher);
#ifndef DTK_ARCH_WIN32_VCPP
  signal(SIGQUIT,catcher);
#endif

  {
    int err = 0;
    if(files && argc < 2)
      {
	if(m.load((const char *) files))
	  err = 1; // error
      }
    else
      {
	if(m.load((const char **) &(argv[1])))
	  err = 1; // error
      }
    if(err)
      return dtkMsg.add(DTKMSG_ERROR, 0, 1,
			"error loading, exiting\n");
  }

  if(m.preConfig() || m.config() || m.postConfig()
     || m.sync())
    return 1;

  while(running && m.state & DTK_ISRUNNING && !(m.frame()))
    ;

  // m.removeAll() is needed when dtkAugments are declared and added using
  // dtkManager::add().
  // m.removeAll();
}
