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

// Eric Tester, etester@vt.edu, wrote the original version of this plays
// back the file created by playbackWrite
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


class playbackRead : public dtkNav
{

public:

  playbackRead (dtkManager *);
  ~playbackRead (void);
  int postConfig (void);
  int postFrame (void);
  int activate(void);

private:
  
  FILE* file;
  
  dtkManager *mgr;
  dtkDequeuer *dequeuer;
  dtkInXKeyboard *keyboard;
  
  KeyCode xkc_l;
  KeyCode xkc_r;
  
  dtkCoord reset_location ;
  dtkCoord  prevFileLoc ;
  dtkCoord  currFileLoc ;
  
  dtkTime t;
  long double currFileTime;
  long double prevFileTime;
  long double resettime;
};

playbackRead::playbackRead (dtkManager *m) :
  dtkNav (m,"playbackRead"), t(0.0,1)
{
  mgr = m;
  keyboard = NULL;
  dequeuer = NULL;

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

  setDescription("playbackRead Log file is: %s\n"
		 "  %splaybackRead commands:  %s\n"
                 "  \'%sL%s\'  Play Log- start/stop\n"
		 "  \'%sR%s\'  Reset- clear and reset\n",
		 logfile, dtkMsg.color.rvil, dtkMsg.color.end,
		 dtkMsg.color.grn, dtkMsg.color.end,
		 dtkMsg.color.red, dtkMsg.color.end);

  if( (file = fopen(logfile,"r")) == NULL)
    {
      dtkMsg.add(DTKMSG_ERROR,1,
		 "playbackRead::playbackRead() failed "
		 "to open logfile \"%s\".\n",
		 logfile);
      if(logfile_mem) free(logfile_mem);
      return;
    }

  if(logfile_mem) free(logfile_mem);

  validate();
}


playbackRead::~playbackRead (void) {
  
  if (dequeuer) {
    delete dequeuer;
    dequeuer = NULL;
  }
  
  if (file) {
    fclose (file);
    file = NULL;
  }
}

int playbackRead::postConfig (void) {

  dtkMsg.add(DTKMSG_NOTICE,getDescription());

  dequeuer = new dtkDequeuer(mgr->record());
  if(!dequeuer || dequeuer->isInvalid())
    return ERROR_;
  

  keyboard = static_cast<dtkInXKeyboard *>(mgr->get("xkeyboard", 
						    DTKINXKEYBOARD_TYPE));
  if(!keyboard) return ERROR_;
  
  //////////////// get X key codes from key syms ///////////////////////
  
  if(!(xkc_l = keyboard->getKeyCode(XK_l)) ||
     !(xkc_r = keyboard->getKeyCode(XK_r)) )
    return ERROR_;
  
  keyboard->queue();
  
  // initialize initial location and time;
  fscanf (file, "%Lg %f %f %f %f %f %f", &currFileTime,
	  &(reset_location.x), &(reset_location.y),
	  &(reset_location.z), &(reset_location.h),
	  &(reset_location.p), &(reset_location.r));
  
  // initialize previous time and location		
    
  // changed jtk 7/04
  location = prevFileLoc = currFileLoc = reset_location ;
  //location = currFileLoc ;

  prevFileLoc = reset_location ;
  
  resettime = prevFileTime = currFileTime;
  
  t.reset(currFileTime,1);
  
  return dtkNav::postConfig();
}


int playbackRead::postFrame (void) {

  static int playbackflag = 0;
  dtkRecord_event* e;
  
  while((e = dequeuer->getNextEvent(keyboard))) {
    
    KeyCode key;
      
    // if key is pressed
    if((keyboard->read(&key,e)))
      {
	// start playback
	if(key == xkc_l)
	  {
	    
	    if(!playbackflag)
	      {
		dtkMsg.add(DTKMSG_NOTICE,"%s Starting playback...\n",
			   getName());
		t.reset (prevFileTime,1);
		
	      }
	    else
	      {
		dtkMsg.add(DTKMSG_NOTICE,"%s Stopping playback...\n",
			   getName());
		prevFileTime = t.get();

		// added jtk 7/04
		prevFileLoc = location ;

	      }
	    
	    playbackflag = !playbackflag;
	    
	    // reset the playback
	  }
	else if (key == xkc_r)
	  {
	    dtkMsg.add(DTKMSG_NOTICE,"%s Stopping and "
		       "reseting playback...\n",
		       getName());
	    fflush(stdout);
	    
	    rewind(file);
	    t.reset (resettime,1);
	    prevFileTime = currFileTime = resettime;
	    
	    location = reset_location;
	    
	    playbackflag = 0;
	    fflush(stdout);
	    
	    // prints the help menu
	  }
      }
  }
  
  if(playbackflag)
    {
      long double time_now = t.get();
      
      while(time_now >= currFileTime)
	{
	  prevFileLoc   = currFileLoc;
	  
	  prevFileTime = currFileTime;
	  
	  // reads in from file,  if there is nothing there
	  // prints end of file
	  if (7!=(fscanf (file, "%Lg%f%f%f%f%f%f", &currFileTime,
			  &(currFileLoc.x), &(currFileLoc.y),
			  &(currFileLoc.z), &(currFileLoc.h),
			  &(currFileLoc.p), &(currFileLoc.r))))
	    {
	      dtkMsg.add(DTKMSG_NOTICE, "end of playback file\n");
	      playbackflag = 0;
	      return CONTINUE;
	    }
	}

      // linear interpolates data
      location.x = prevFileLoc.x+(time_now-prevFileTime)*
	(currFileLoc.x-prevFileLoc.x)/(currFileTime-prevFileTime);
      location.y = prevFileLoc.y+(time_now-prevFileTime)*
	(currFileLoc.y-prevFileLoc.y)/(currFileTime-prevFileTime);
      location.z = prevFileLoc.z+(time_now-prevFileTime)*
	(currFileLoc.z-prevFileLoc.z)/(currFileTime-prevFileTime);
      location.h = prevFileLoc.h+(time_now-prevFileTime)*
	(currFileLoc.h-prevFileLoc.h)/(currFileTime-prevFileTime);
      location.p = prevFileLoc.p+(time_now-prevFileTime)*
	(currFileLoc.p-prevFileLoc.p)/(currFileTime-prevFileTime);
      location.r = prevFileLoc.r+(time_now-prevFileTime)*
	(currFileLoc.r-prevFileLoc.r)/(currFileTime-prevFileTime);

    }
  return dtkNav::postFrame();
}

int playbackRead::activate(void)
{
  t.reset(0.0,1);
  // Need to add flushing the queue to dtkNav::activate().
  while(dequeuer->getNextEvent(keyboard));
  return dtkNav::activate();
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
  return new playbackRead(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


