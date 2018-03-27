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
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Pack.H>
#include <dtk.h>

// from dtk-launcher.C
extern int update_killButtons;

// from mainWindow.C
extern Fl_Scroll *kill_scroll;

class killButton;

static killButton *firstKillButton = NULL;
static killButton *lastKillButton = NULL;

// LABELSIZE must be the same as LABELSIZE in launchButtons.C
#define LABELSIZE  ((size_t) 35)
#define PROCFILESIZE ((size_t) 30)
#define KILL_PRE   "kill "

class killButton : public Fl_Button
{
public:
  killButton(const char *arg0, const char *args, pid_t pid, int stripPathFlag);
  ~killButton(void);
  killButton *prev;
  killButton *next;
  pid_t pid;
  char procFile[PROCFILESIZE+1];
private:
  char labelStr[LABELSIZE+1];
  char *getKillButtonName(const char *fileName, const char *args_in, pid_t pid, int stripPathFlag);
};

static void execCallback(Fl_Widget *who, void *data)
{
  //printf("calling kill(%d,SIGTERM)\n",
  // ((killButton *) data)->pid);
  kill(((killButton *) data)->pid, SIGTERM);
}

void killAll(Fl_Widget *who, void *data)
{
  killButton *b = lastKillButton;
  if(b)
    {
      kill(b->pid, SIGTERM);
      b = b->prev;
    }
  for(;b;b = b->prev)
    {
      usleep((unsigned long) 100000);
      kill(b->pid, SIGTERM);
    }
}

char *killButton::getKillButtonName(const char *fileName,
				    const char *args_in, pid_t pid,
				    int stripPathFlag)
{
  char args[LABELSIZE+1];
  snprintf(args,LABELSIZE, "%s", args_in);

  char *argv0 = (char *)fileName;

  if(!stripPathFlag)
    {
      // take off the stuff before last '/'
      for(; *argv0 && *argv0 != ' ';argv0++);
      // make argv0 point to just after the first '/'
      for(;*argv0 != '/' && argv0 != (char *) fileName; argv0--);
      if(*argv0 == '/')
	argv0++;
    }

  char pidString[LABELSIZE+1];
  snprintf(pidString, LABELSIZE, "%d", pid);
  size_t len = strlen(KILL_PRE) + strlen(argv0) +
    strlen(args) + strlen(pidString) + 3;

  if(len > LABELSIZE)
    {
      int l = LABELSIZE - strlen(KILL_PRE) - strlen(pidString) - 5;
      // split up the string reduction into two parts.
      int arg0Len = (l > (int) strlen(argv0)) ? (int) strlen(argv0) : l ;
      l -= arg0Len;
      int argLen =  (l > 0) ? l : 0 ;
      snprintf(labelStr, LABELSIZE, "%s%*.*s%s%*.*s... %s",
	       KILL_PRE,
	       arg0Len, arg0Len, argv0,
	       (argLen > 0) ? " " : "",
	       argLen, argLen, args, 
	       pidString);
    }
  else // len <= LABELSIZE
    {
      int l = LABELSIZE - len + 1;
      char s[LABELSIZE+1];
      // fill s with ' ' spaces
      int i=0;
      for(; i<l;i++) s[i] = ' ';
      s[i] = '\0';
      snprintf(labelStr, LABELSIZE, "%s%s %s%s%s", KILL_PRE,
	       argv0, args, s, pidString);
    }

  return labelStr;
}

killButton::killButton(const char *fileName, const char *args_in,
		       pid_t pid_in, int stripPathFlag) : 
  Fl_Button (12, 382, 323, 25, 
	     getKillButtonName(fileName,args_in,pid_in, stripPathFlag))
{
#ifdef DTK_ARCH_LINUX
  snprintf(procFile, PROCFILESIZE,"/proc/%d", pid_in);
#else // IRIX
  snprintf(procFile, PROCFILESIZE,"/proc/%10.10d", pid_in);
  //printf("procFile=%s\n",procFile);
#endif
  // printf("procFile=%s\n",procFile);
  labelfont(FL_COURIER_BOLD);
  pid = pid_in;
  align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  callback(execCallback, this);
  // make this be the next and last in the list
  if(!lastKillButton)
    {
      lastKillButton = firstKillButton = this;
      prev = NULL;
    }
  else
    {
      prev = lastKillButton;
      lastKillButton->next = this;
      lastKillButton = this;
    }
  next = NULL;

}

killButton::~killButton(void)
{
  //printf("line=%d file=%s removing %s\n",__LINE__,__FILE__, fileName);
  if(prev) prev->next = next;
  else firstKillButton = next;

  if(next) next->prev = prev;
  else lastKillButton = prev;
}


int updateKillButtons(const char *prepath, const char *path,
		      const char *args, pid_t add_pid)
{
  //printf("line=%d file=%s\n",__LINE__,__FILE__);
  update_killButtons = 0;
  //printf("line=%d file=%s\n",__LINE__,__FILE__);


  // remove all the killButton widgets
  killButton *b = firstKillButton;
  for(;b;b = b->next)
    kill_scroll->remove(b);

  // This redraw() will help space the buttons correctly if they are
  // above the kill_scroll widget.  That's why I have to remove all
  // the buttons every time this is called.
  kill_scroll->redraw();

  if(add_pid)
    {
      if(prepath)
	{
	  // take off the dirname part of path and than put in its
	  // place prepath/.
	  char labelPath[LABELSIZE+1];
	  char *f = (char *) path;
	  for(;*f && *f != ' ';f++);
	  for(;*f != '/' && f != path;f--);
	  if(*f == '/') f++;
	  snprintf(labelPath,LABELSIZE,"%s/%s",prepath,f);
	  new killButton(labelPath, args, add_pid, 1); // don't strip path
	}
      else
	new killButton(path, args, add_pid, 0); // do strip path
    }

  b = firstKillButton;
  for(;b;)
    {
      killButton *c = b->next;
      // If I can open the proc file the process is still in
      // existence.  If not is't not or messed up, so delete
      // killButton object.
      int fd = open(b->procFile, O_RDONLY);
      if(fd == -1)
	delete b;
      else
	close(fd);

      b = c;
    }

  // repack the kill_scroll widget
  short y = kill_scroll->y() + 3;
  b = firstKillButton;
  int i = 0;
  for(;b;b = b->next)
    {
      b->position(3, y + i*25);
      i++;
      kill_scroll->add(b);
    }

  kill_scroll->redraw();
  return 0; // success
}
