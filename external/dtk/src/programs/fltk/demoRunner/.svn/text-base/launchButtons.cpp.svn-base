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
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Input.H>
#include <dtk.h>
#ifdef DTK_ARCH_CYGWIN
# include <dirent.h>
# define direct dirent
#else 
# include <sys/dir.h>
#endif

#define CD_CLICKED_COLOR FL_MAGENTA
#define EX_CLICKED_COLOR FL_MAGENTA
#define CD_COLOR         FL_GREEN
#define EX_COLOR         FL_CYAN

// from getcwd.cpp
extern char *Getcwd();

// from dtk-demoRunner.cpp
extern int update_launcherButtons;
extern int switchLaunchToKill;

// from mainWindow.cpp
extern Fl_Output *cwd_output;
extern Fl_Scroll *launch_scroll;
extern char *topDir;



class launchButton;

static launchButton *lastLaunchButton = NULL;
static char *cwd = NULL;

#define RUNNINGLABELSIZE ((size_t) 40)

#define LABELSIZE ((size_t) 35)
#define CHDIR_PRE "cd "
#define EXEC_PRE  "run "
#define LABELSIZE ((size_t) 35)
#define PROCFILESIZE  40

pid_t launch_pid = 0;
static char _launch_procfile[PROCFILESIZE+1];
char *launch_procfile = _launch_procfile;

static char _killLabelString[RUNNINGLABELSIZE+1];
char *killLabelString = _killLabelString;

static char _killButtonString[RUNNINGLABELSIZE+1];
char *killButtonString = _killButtonString;


// We keep a list of inodes of all files that have been acted on, so
// that we can color the buttons differently.
static ino_t *inodes = NULL;


static int checkINode(ino_t ino)
{
  if(!inodes)
    {
      inodes = (ino_t *) malloc(sizeof(ino_t));
      inodes[0] = (ino_t) 0;
      return 0;
    }

  ino_t *i = inodes;
  for(;(*i);i++)
    if(*i == ino)
      return 1;
  return 0;
}

static void addINode(const char *file)
{
  struct stat st;
  if(stat(file,&st) == 0)
    {
      int i = 0;
      for(;(inodes[i]);i++)
	{
	if(inodes[i] == st.st_ino)
	  {
	  break;
	  }
	}
      if(!(inodes[i]))
	{
	  // add to list
	  inodes = (ino_t *) realloc(inodes, sizeof(ino_t)*(i+2));
	  inodes[i] = st.st_ino;
	  inodes[i+1] = (ino_t) 0;
	}
    }
}


class launchButton : public Fl_Button
{
public:
  launchButton(int count, const char *file, const char *dir);
  ~launchButton(void);
  launchButton *prev;
  // fileName and dir are both set in listFile mode, otherwise both
  // are not set.
  char *fileName;
  char *dir;
  char labelStr[LABELSIZE+1];
private:
  char *getLaunchButtonName(const char *fileName, const char *dir);
};

static void cdCallback(Fl_Widget *who, void *data)
{
  char *cdir = Getcwd();
  if(!cdir) return;

  launchButton *l = (launchButton *) data;
  l->color(CD_CLICKED_COLOR);

  if(chdir(l->dir))
    {
      printf("dtk-demoRunner Error: chdir(\"%s\") failed:\n"
	     "system error number %d: \"%s\"\n"
	     "i.e.: Can't change the current working"
	     "directory to \"%s/%s\".\n",
	     l->dir, errno, strerror(errno),
	     strcmp("/",cdir)?cdir:"" , l->dir);
      return;
    }

  cdir = Getcwd();
  if(!cdir) return;

  addINode(cdir);

  // I must call buildLaunchButtons() after this returns
  update_launcherButtons = 1;

  cwd_output->value(cdir);
}

static void execCallback(Fl_Widget *who, void *data)
{
  launchButton *l = (launchButton *) data;

  char *Cwd = Getcwd();
  if(!Cwd) return; // error

  // go to directory where it needs to run
  if(l->dir && chdir(l->dir))
    {
      printf("dtk-demoRunner Error: chdir(\"%s\") failed:\n"
	     "system error number %d: \"%s\"\n"
	     "i.e.: Can't change the current working"
	     "directory to \"%s\".\n",
	     l->dir, errno, strerror(errno),
	     l->dir);
      return;
    }

#ifdef DTK_ARCH_IRIX
  // IRIX does not have vfork()
  pid_t pid = fork();
# else // Linux and other
  // vfork() is much better than fork() in this case.
  pid_t pid = vfork();
#endif

  // if not child and in different directory get back to original
  // directory
  if(l->dir  &&  pid != (pid_t) 0  &&  chdir(Cwd))
    {
      printf("dtk-demoRunner Error: chdir(\"%s\") failed:\n"
	     "system error number %d: \"%s\"\n"
	     "i.e.: Can't change the current working"
	     "directory to \"%s\".\n",
	     l->dir, errno, strerror(errno),
	     l->dir);
    }

  if(pid == (pid_t)-1) // error
    {
      printf("dtk-demoRunner Error: *fork() failed:\n"
	     "system error number %d: \"%s\"\n"
	     "i.e.: Can't run the program \"%s\".\n",
	     errno, strerror(errno), l->fileName);
      return;
    }
  if(pid != (pid_t) 0) // I'm the mommy
    {
      launch_pid = pid;
#ifdef DTK_ARCH_LINUX 
      snprintf(launch_procfile, PROCFILESIZE,"/proc/%d", pid);
#else // all other OS
      snprintf(launch_procfile, PROCFILESIZE,"/proc/%10.10d", pid);
#endif
      snprintf(killLabelString, RUNNINGLABELSIZE,
	       "Running: %s", &(l->labelStr[strlen(EXEC_PRE)]));
      snprintf(killButtonString, RUNNINGLABELSIZE,
	       "Kill: %s", &(l->labelStr[strlen(EXEC_PRE)]));
      addINode(l->fileName);
      l->color(EX_CLICKED_COLOR);
      switchLaunchToKill = 1;
      return;
    }
  else // I'm the child
    {
      // run through a sh for free command line parsing
      const char *argv[4] =
      {
	"sh", "-c", l->fileName, NULL
      };
      struct stat st;
      // make f exclude the white space
      char *f = strdup(l->fileName);
      char *g = f;
      for(;*f;f++)
	if(*f == ' ' || *f == '\t')
	  {
	    *f = '\0';
	    break;
	  }
      f = g;
      //printf("f=%s\n",f);
      if(f[0] != '/' ||
	 (stat(f, &st) == 0 &&
	  !S_ISDIR(st.st_mode) && (0111 & st.st_mode)))
	{
	  execvp("/bin/sh", (char *const *) argv);
	  printf("dtk-demoRunner Error: execv() failed:\n"
		 "system error number %d: \"%s\"\n"
		 "i.e.: Can't run the program: /bin/sh -c \"%s\"\n",
		 errno, strerror(errno), l->fileName);
	  exit(1);
	}
      printf("dtk-demoRunner Error: Can't run the program: \"%s\"\n",
	     l->fileName);
      exit(1);
    }
}

char *launchButton::getLaunchButtonName(const char *fileName_in,
					const char *dir_in)
{
  if(fileName_in)
    {
      char FileName[LABELSIZE+1];
      if(dir_in)
	{
	  snprintf(FileName,LABELSIZE,"%s/%s",dir_in,fileName_in);
	}
      else
	{
	  strncpy(FileName, fileName_in, LABELSIZE);
	  FileName[LABELSIZE] = '\0';
	}

      if(strlen(EXEC_PRE) + strlen(FileName) + 1 > LABELSIZE)
	{
	  int len = LABELSIZE - strlen(EXEC_PRE) - 4;
	  snprintf(labelStr, LABELSIZE, "%s%*.*s...",
		   EXEC_PRE, len, len, FileName);
	}
      else
	snprintf(labelStr, LABELSIZE, "%s%s", EXEC_PRE, FileName);
    }
  else // just "cd" case
    {
      if(strlen(CHDIR_PRE) + strlen(dir_in) + 1 > LABELSIZE)
	{
	  size_t len = LABELSIZE - strlen(CHDIR_PRE) - 4;
	  snprintf(labelStr, LABELSIZE, "%s...%s", CHDIR_PRE,
		   &dir_in[strlen(dir_in) - len]);
	}
      else
	snprintf(labelStr, LABELSIZE, "%s%s", CHDIR_PRE, dir_in);
    }

  return labelStr;
}


launchButton::launchButton(int count, const char *fileName_in,
			   const char *dir_in) :
  Fl_Button (13, 33 + 25*count, 323, 25,
	     getLaunchButtonName(fileName_in, dir_in))
{
  fileName = NULL;
  dir = NULL;

  labelfont(FL_COURIER_BOLD);
  align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  if(!fileName_in)
    {
      callback(cdCallback, this);
      dir = strdup(dir_in);
    }
  else // fileName != NULL
    {
      callback(execCallback, this);
      
      if(dir_in) // listFile mode
	{
	  // since in this case this program will do "cd $dir ; $fileName"
	  // than I'll make fileName begin with "./".
	  char *f = (char *) fileName_in;
	  for(; *f && *f != ' '; f++);
	  for(; f != fileName_in && *f != '/'; f--);
	  if(*f == '/') f++;
	  fileName = (char *) malloc(strlen(f) +3 );
	  sprintf(fileName,"./%s",f);
	  dir = strdup(dir_in);
	}
      else
	{
	  fileName = strdup(fileName_in);
	}
    }
  prev = lastLaunchButton;
  lastLaunchButton = this;
  //printf("line=%d file=%s fileName=%s dir=%s\n",__LINE__,__FILE__,fileName,dir);
}

launchButton::~launchButton(void)
{
  //printf("line=%d file=%s removing %s\n",__LINE__,__FILE__, fileName);
  if(fileName)
    {
      free(fileName);
      fileName = NULL;
    }
  if(dir)
    {
      free(dir);
      dir = NULL;
    }
  lastLaunchButton = prev;
}

// rebuild the launch button widgets using the current directory. This
// is called every time the directory changes.  buildLaunchButtons()
// will not get called if in listFile mode (listFile != NULL)
int buildLaunchButtons(void)
{
  //printf("line=%d file=%s\n",__LINE__,__FILE__);
  update_launcherButtons = 0;

  // remove all the launchButton widgets
  while(lastLaunchButton)
    {
      Fl_Widget *child = lastLaunchButton;
      launch_scroll->remove(child);
      delete child;
    }
  // This redraw() will help space the buttons correctly if they are
  // above the launch_scroll widget.  That's why I have to remove all
  // the button every time this is called.
  launch_scroll->redraw();

  cwd = Getcwd();
  if(!cwd) return 1;
  DIR *dir = opendir(cwd);
  if(!dir)
    {
      printf("dtk-demoRunner Error: opendir(\"%s\") failed:\n"
	     "system error number %d: \"%s\"\n"
	     "i.e.: Can't open the current working directory.\n",
	     cwd, errno, strerror(errno));
      cwd = NULL;
      return 1;
    }
  // file types just for this stuff
#define EXFILE    010
#define DIREC     0100
#define CLICKED   01
#define NONE      00
#define EXORDIR   (EXFILE | DIREC)
  struct FileList
  {
    char *file;
    struct FileList *next;
    int type; //(file or directory)
  } *fileList = NULL;

  {
    struct direct *ent;
    while((ent = readdir(dir)))
      {
	struct stat st;
	if(stat(ent->d_name,&st) == 0 && strcmp(".", ent->d_name) &&
	   strcmp("..", ent->d_name))
	  {
	    int type = NONE;
	    if(S_ISDIR(st.st_mode))
	      type = DIREC; // directory type
	    else if(!S_ISDIR(st.st_mode) && (0111 & st.st_mode))
	      type = EXFILE; // executable file

	    // has this file been clicked on before
	    if(checkINode(st.st_ino))
	      type |= CLICKED;

	    if(type != NONE)
	      {
		// make entry
		struct FileList *entry = (struct FileList *)
		  malloc(sizeof(struct FileList));
		entry->file = strdup(ent->d_name);
		entry->next = NULL;
		entry->type = type;

		// add entry to list in order
		if(!fileList) fileList = entry;// first one
		else
		  {
		    struct FileList *l = fileList;
		    struct FileList *m = fileList;
		    while(m)
		      {
			if((EXORDIR & type) < (EXORDIR & m->type)) break;
			if((EXORDIR & type) == (EXORDIR & m->type) &&
			   strcmp(m->file,ent->d_name) > 0) break;
			l = m;
			m = m->next;
		      }
		    if(l == m) // first one
		      {
			fileList = entry;
			entry->next = l;
		      }
		    else
		      {
			entry->next = m;
			l->next = entry;
		      }
		  }
	      }
	  }
      }
    closedir(dir);
  }

  int buttonCount = 0;
  struct FileList *l = fileList;
  class launchButton *lb;

  if(strcmp(cwd,topDir))
    {
      lb = new launchButton(buttonCount++, NULL, "..");
      lb->color(CD_COLOR);
      launch_scroll->add(lb);
    }
  while(l)
    {
      if(l->type & EXFILE) // executable file
	{
	  lb = new launchButton(buttonCount++,l->file, NULL);
	  if(l->type &  CLICKED)
	    lb->color(EX_CLICKED_COLOR);
	  else
	    lb->color(EX_COLOR);
	  launch_scroll->add(lb);
	}
      else // directory type
	{
	  lb = new launchButton(buttonCount++, NULL, l->file);
	  if(l->type & CLICKED)
	    lb->color(CD_CLICKED_COLOR);
	  else
	    lb->color(CD_COLOR);
	  launch_scroll->add(lb);
	}
      free(l->file);
      struct FileList *m = l;
      l = l->next;
      free(m);
    }

  launch_scroll->redraw();

  return 0; // success
}

// this will only get called once
int buildLaunchButtons_withListFile(const  char *listFile)
{
  cwd = Getcwd();
  if(!cwd) return 1;

  FILE *file = fopen(listFile, "r");
  if(!file)
    {
      printf("dtk-launcher Error: failed to open file: \"%s\":\n"
	     "fopen(\"%s\",\"r\") failed:\n"
	     "system error number %d: \"%s\"\n",
	     listFile, listFile,
	     errno, strerror(errno));
      return 1; // failure
    }
#define BUFSIZE (1024*3)
  char buf[BUFSIZE+1];
  int buttonCount = 0;

  while(fgets(buf, BUFSIZE, file))
    {
      // strip off '\n' and '#' and after
      for(char *s=buf; *s; s++)
	if(*s == '\n' || *s == '#')
	  {
	    *s = '\0';
	    break;
	  }
      // printf("read \"%s\"\n",buf);
      // fileName is the base fileName
      char *fileName = buf;
      
      if(fileName[0] != '\0')
	{
	  for(;*fileName && *fileName != ' ';fileName++);
	  for( ;fileName != buf; fileName-- )
	    if( *fileName  ==  '/' )
	      {
		fileName++;
		break;
	      }
	  if( *fileName  ==  '/' )
	    fileName++;
	  fileName = strdup(fileName);
	  
	  char *dir = buf;
	  for(;*dir && *dir != ' ';dir++);
	  for( ;dir != buf; dir--)
	    if(*dir  ==  '/')
	      {
		*dir = '\0';
		break;
	      }
	  if(dir == buf)
	    {
	      if(buf[0] == '/')
		{
		  buf[1] = '\0';
		  dir = buf;
		}
	      else
		dir = NULL;
	    }
	  else
	    dir = buf;
	  
	  launchButton *lb = new launchButton(buttonCount, fileName, dir);
	  lb->color(EX_COLOR);
	  launch_scroll->add(lb);
	  buttonCount++;

	  if(fileName)
	    free(fileName);
	}
    }

  fclose(file);

  // make sure that the inodes list is initialized
  if(!inodes)
    checkINode(0);

  launch_scroll->redraw();

  return 0; // success
}

