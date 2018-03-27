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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

#include "config_widget.h"

#define DIR         0
#define NAME        1
#define CONFIG      2
#define HELP        3
#define CONFIG_PATH 4
#define HELP_PATH   5

// From mainWindow.C
extern char *cwd;

void help_callback(Fl_Widget *who, void *data)
{
  Config_Widget *c = (Config_Widget *) data;

  c->help_pid = fork();

  if(c->help_pid != 0)// parent process
    {
      if(c->help_pid > 0)
	{
	  c->helpButton->deactivate();
	  //c->helpButton->color(FL_DARK3);
	}
      return;
    }
  else // child process
    {
      usleep(100000);
      if(chdir(c->str[DIR]))
	{
	  dtkMsg.add(DTKMSG_WARN,1,
		     "chdir(\"%s\") failed.\n",
		     c->str[DIR]);
	  exit(1);
	}
      const char *argv[4] =
      {
	"sh", "-c", c->str[HELP], NULL
      };
      execvp("/bin/sh", (char *const *) argv);
      dtkMsg.add(DTKMSG_WARN,1,
		 "execvp( \"/bin/sh\","
		 "\"sh\", \"-c\",\"%s\") failed.\n",
		 argv[2]);
      exit(1);
    }
}

void config_callback(Fl_Widget *who, void *data)
{
  Config_Widget *c = (Config_Widget *) data;

  c->config_pid = fork();

  if(c->config_pid != 0)// parent process
    {
      if(c->config_pid > 0)
	{
	  c->configButton->deactivate();
	  c->configButton->color(FL_DARK3);
	}
      return;
    }
  else // child
    {
      usleep(100000);
      if(chdir(c->str[DIR]))
	{
	  dtkMsg.add(DTKMSG_WARN,1,
		     "chdir(\"%s\") failed.\n",
		     c->str[DIR]);
	  exit(1);
	}
      const char *argv[4] =
      {
	"sh", "-c", c->str[CONFIG], NULL
      };
      execvp("/bin/sh", (char *const *) argv);
      dtkMsg.add(DTKMSG_WARN,1,
		 "execvp( \"/bin/sh\","
		 "\"sh\", \"-c\",\"%s\") failed.\n",
		 argv[2]);
      exit(1);
    }
}

Config_Widget::~Config_Widget(void)
{
  for(int i=0;i<NUM_STR;i++)
    if(str[i])
      {
	free(str[i]);
	str[i] = NULL;
      }
}

#define HEIGHT  32

Config_Widget::
Config_Widget(int num, int installFlag, 
	      char *directory, char *name,
	      char *configScript, char *helpScript) :
  Fl_Group(2, yPos = 32 + num*HEIGHT, 344, HEIGHT)
{
  org_installFlag = installFlag;
  count = num;
  help_pid = 0;
  config_pid = 0;

  str[NAME] = strdup(name);
  str[DIR] = strdup(directory);
  str[CONFIG] = strdup(configScript);
  str[HELP] = strdup(helpScript);

  {
    char *format = const_cast<char*>("%s/%s");
    str[CONFIG_PATH] = (char *)
      malloc(strlen(format) + 
	     strlen(directory) +
	     strlen(configScript));
    sprintf(str[CONFIG_PATH],format,directory,configScript);
  }
  {
    char *format = const_cast<char*>("%s/%s");
    str[HELP_PATH] = (char *)
      malloc(strlen(format) + 
	     strlen(directory) +
	     strlen(helpScript));
    sprintf(str[HELP_PATH],format,directory,helpScript);
  }
  {
    Fl_Button* o = new Fl_Button(2, yPos, 154, HEIGHT, str[NAME]);
    o->box(FL_EMBOSSED_BOX);
    //o->labelfont(5);
    o->labelsize(12);
    o->align(FL_ALIGN_WRAP|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    add(o);
  }
  {
    Fl_Check_Button* o = install_Button = new
      Fl_Check_Button(156, yPos, 80, HEIGHT, "Install");
    o->color(50);
    o->box(FL_EMBOSSED_FRAME);
    o->down_box(FL_DIAMOND_DOWN_BOX);
    o->value(installFlag);
    o->selection_color(FL_GREEN);
    add(o);
  }
  {
    Fl_Button* o = configButton = new
      Fl_Button(236, yPos, 65, HEIGHT, "Configure");
    o->labelsize(12);
    o->callback(config_callback, this);
    add(o);
  }
  {
    Fl_Button* o = helpButton = new
      Fl_Button(301, yPos, 46, HEIGHT, "Help");
    o->labelsize(12);
    o->callback(help_callback, this);
    add(o);
  }
}

int Config_Widget::getCount(void) const
{
  return count;
}

void Config_Widget::print(FILE *file) const
{
  fprintf(file,"%d ", install_Button->value());
  for(int i=0;i<4;i++)
    {
      char *s = str[i];
      for(;*s;s++)
	if(*s == '\t' || *s == ' ')
	  break;
      if(*s) // if there is white space put in quotes
	fprintf(file," \"%s\"",str[i]);
      else
	fprintf(file," %s",str[i]);
    }
  fprintf(file,"\n");
}

