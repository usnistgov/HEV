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
// Eric Tester, etester@vt.edu, wrote the original version of this writes
// the navigated path to a file for later playback
//
// the file name is specified by the environment variable
// DTK_NAV_PLAYBACK_FILE, if set, otherwise the file used is
// "/tmp/playback-$LOGNAME.log"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dtk.h>
#include <dtkX11.h>
#include <dtk/dtkDSO_loader.h>

#define HELPMENU 

class playbackWrite : public dtkAugment
{

public:

  playbackWrite (dtkManager *);
  ~playbackWrite (void);
  
  int preFrame  (void);
  int postFrame (void);

 private:

  dtkManager *mgr;
  dtkNavList *navList;
  dtkDequeuer *dequeuer;
  
  dtkInXKeyboard *keyboard;
  KeyCode xkc_l;
  KeyCode xkc_r;

  dtkTime t;
  FILE *file;
  dtkCoord prevCoord;
};


playbackWrite::playbackWrite (dtkManager *m):
  dtkAugment ("playbackWrite"), t(0.0,1)
{
  mgr = m;
  keyboard = NULL;
  dequeuer = NULL;
  navList = NULL;

  char *logfile = getenv("DTK_NAV_PLAYBACK_FILE");
  char *logfile_mem = NULL;
  if(!logfile)
    {
      char *format = const_cast<char*>("/tmp/playback-%s");
      char *logname = getenv("LOGNAME");
      if(!logname) logname = const_cast<char*>("nonknown_user");
      logfile = logfile_mem = (char *) dtk_malloc(strlen(format) +
						  strlen(logname) + 1);
      sprintf(logfile, format, logname);
    }

  setDescription("playbackWrite Log file is: %s\n"
		 "  %splaybackWrite commands:  %s\n"
                 "  \'%sL%s\'  Logging- start/stop\n"
		 "  \'%sR%s\'  Reset- clear and reset\n",
		 logfile, dtkMsg.color.rvil, dtkMsg.color.end,
		 dtkMsg.color.grn, dtkMsg.color.end,
		 dtkMsg.color.red, dtkMsg.color.end);

  if( (file = fopen(logfile,"w")) == NULL)
    {
      dtkMsg.add(DTKMSG_ERROR,1,
		 "playbackWrite::playbackWrite() failed "
		 "to open logfile \"%s\".\n",
		 logfile);
      if(logfile_mem) free(logfile_mem);
      return;
    }

  if(logfile_mem) free(logfile_mem);

  validate();
}


playbackWrite::~playbackWrite (void) {
  
  if (dequeuer) {
    delete dequeuer;
    dequeuer = NULL;
  }
  
  if (file) {
    fclose (file);
    file = NULL;
  }
}

int playbackWrite::preFrame (void)
{
  dtkMsg.add(DTKMSG_NOTICE,getDescription());

  dequeuer = new dtkDequeuer(mgr->record());
  if(!dequeuer || dequeuer->isInvalid()) return ERROR_;
  
  navList = (dtkNavList *) mgr->check("dtkNavList", DTKNAVLIST_TYPE) ;
  if(!navList)
    {
      navList = new dtkNavList;
      if(mgr->add(navList))
	{
	  delete navList;
	  navList = NULL;
	  return ERROR_;
	}
    }

  keyboard = static_cast<dtkInXKeyboard *>(mgr->get("xkeyboard", 
						    DTKINXKEYBOARD_TYPE));
  if(!keyboard) return ERROR_;
  
  //////////////// get X key codes from key syms ///////////////////////

  if(!(xkc_l = keyboard->getKeyCode(XK_l)) ||
     !(xkc_r = keyboard->getKeyCode(XK_r)) )
    return ERROR_;

  keyboard->queue();

  
  ///// check time reset for delayed time thingy /////
  t.reset(0.0,1);

  return REMOVE_CALLBACK;
}


int playbackWrite::postFrame (void) {

  static int recordFlag = 0;
  static long double prevtime = 0;
  dtkRecord_event* e;
  dtkNav* currentNav = navList->current();

  while((e = dequeuer->getNextEvent(keyboard)))
    {
      if (currentNav)
	{
	  KeyCode key;
	  if((keyboard->read(&key,e)))
	    {
	      
	      if (key == xkc_l)
		{
		  if (!recordFlag) dtkMsg.add(DTKMSG_NOTICE,"%s "
					      "Begining recording...\n",
					      getName());
		  else dtkMsg.add(DTKMSG_NOTICE,"%s Stopped recording...\n",
				  getName());
		  recordFlag = !recordFlag;
		  t.reset(prevtime,1);
		}
	      else if (key == xkc_r)
		{
		  dtkMsg.add(DTKMSG_NOTICE,"%s Clearing current "
			     "saved playback...\n",
			     getName());
		  rewind(file);
		  recordFlag = 0;
		} 
	    }
	}
    }
  
  if (currentNav)
    {
      if(recordFlag)
	{
	  static int rflag = 1;	     // flag to record info
	  static int doubleflag = 0; // flag to test for duplicates
	  dtkCoord location;
	  
	  // get info from nav coord.
	  location = currentNav->location ;
	  
	  // checks if previous coord is the same as current coord
	  if (prevCoord.equals(&location)) 
	    {
	      // for duplicates
	      if (!doubleflag)  
		doubleflag = 1;
	      
	      rflag = 0;
	    }
	  else if (doubleflag)  
	    doubleflag = 0;
	  
	  // if true print value and to file
	  if (rflag)
	    {   
	      fprintf(file, "%.25Lg %.7g %.7g %.7g %.7g %.7g %.7g\n", 
		      prevtime= t.get(),
		      location.x, location.y, location.z, 
		      location.h, location.p, location.r);
	    
	      prevCoord = location;
	    }

	  rflag = 1;
	}
    }

  return CONTINUE;
}


/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment *dtkDSO_loader(dtkManager *manager, void *p)
{
  return new playbackWrite(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
