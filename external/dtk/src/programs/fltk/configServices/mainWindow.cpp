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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Group.H>
#include <dtk.h>

#include "config_widget.h"

static Config_Widget **config_Widget = NULL;
static int Config_count = 0;

char *cwd = NULL;

static char *fileName = NULL;


#define BUFFSIZE  1024

// from dtk-configService.C
extern int running;
extern int exitStatus;

static Fl_Scroll *launch_scroll;


void activate_launch_button(pid_t pid)
{
  Config_Widget **c = config_Widget;
  for(;*c;c++)
    {
      if(pid == (*c)->help_pid)
	{
	  (*c)->helpButton->activate();
	  (*c)->help_pid = 0;
	  return;
	}
      if(pid == (*c)->config_pid)
	{
	  (*c)->configButton->activate();
	  (*c)->config_pid = 0;
	  return;
	}
    }
}

static int config_has_changed(void)
{
  Config_Widget **c = config_Widget;
  for(;*c;c++)
    if((*c)->install_Button->value() != (*c)->org_installFlag)
      return 1;
  return 0;
 
}

static void save(void) 
{
  FILE *file = fopen(fileName, "w");
  if(!file)
    {
      dtkMsg.add(DTKMSG_ERROR,1,
		 "failed to open file \"%s\".\n",
		 fileName);
      return; // error
    }
  fprintf(file,
	  "# The format of this file is 5 entries per line\n"
	  "# Entries that include space or tab characters\n"
	  "# must be in double quotes (\")\n"
	  "# The 5 entries are:\n"
	  "# 1:  0 or 1  -- whether to install the service (1) or not (0)\n"
	  "# 2:  service source directory\n"
	  "# 3:  descriptive service name\n"
	  "# 4:  service configure program\n"
	  "# 5:  service help program\n"
	  "\n");

  Config_Widget **c = config_Widget;
  for(;*c;c++)
    (*c)->print(file);
  fclose(file);
  exitStatus = 0;
}

static void quitCallback(Fl_Widget *who, void *data)
{
  if(!config_has_changed())
    {
      running = 0;
      return;
    }
  int i = fl_choice("Save changes before exiting",
		    "Cancel", "No", "Yes");
  if(i == 0) // Cancel
    ;
  else if(i == 1) // No
    running = 0;
  else // Yes
    {
      save();
      running = 0;
    }
}

static void saveAndQuitCallback(Fl_Widget *who, void *data) 
{
  save();
  running = 0;
}

static void helpCallback(Fl_Widget *who, void *data) 
{
  fl_message("This GUI is just a front end to configure"
	     " and install DTK services.  The Help and Configure"
	     " buttons just launch programs.  This running program"
	     " only writes to the file \"%s\".",
	     fileName);
}

int make_mainWindow(char *fileName_in, int argc, char **argv)
{
  cwd = getcwd(NULL, 0);
  Fl_Window* w = new Fl_Window(366, 318, "Configure DTK Services");
  {
    Fl_Button* o = new Fl_Button(8, 288, 80, 25, "Quit");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(quitCallback);
  }
  {
    Fl_Button* o = new Fl_Button(97, 288, 80, 25, "Help");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(helpCallback);
  }
  {
    Fl_Button* o = new Fl_Button(187, 288, 169, 25, "Save and Quit");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(saveAndQuitCallback);
  }
  {
    launch_scroll = new Fl_Scroll(0, 30, 366, 253);
    launch_scroll->box(FL_DOWN_BOX);
    launch_scroll->labeltype(FL_ENGRAVED_LABEL);
    launch_scroll->labelsize(18);
    launch_scroll->label("Configure and Install which DTK Services?");
    launch_scroll->type(Fl_Scroll::VERTICAL_ALWAYS);
    launch_scroll->end();
    Fl_Group::current()->resizable(launch_scroll);

    fileName = fileName_in;
    FILE *file = fopen(fileName, "r");
    if(!file)
      {
	dtkMsg.add(DTKMSG_ERROR,1,
		   "failed to open file \"%s\".\n",
		   fileName);
	return 1; // error
      }
    char str[BUFFSIZE];
    while(fgets(str,BUFFSIZE,file))
      {
	char *s = str;
	for(;*s == ' ' || *s == '\t';s++);
	if(*s != '#' && *s != '\n' && *s)
	  {
	    char *argString[5];
	    argString[4] = const_cast<char*>("");
	    int i =0;
	    for(;i<5 && *s;i++)
	      {
		for(;*s == ' ' || *s == '\t';s++);
		argString[i] = s;
		if(*s == '"')
		  {
		    argString[i] = s+1;
		    for(s++;*s && *s != '"';s++);
		  }
		else
		  for(s++;*s && *s != ' ' && *s != '\t';s++);
		if(*s)
		  {
		    *s = '\0';
		    s++;
		  }
		for(;*s && (*s == ' ' || *s == '\t');s++);
		//printf("arg[%d] =%s\n",i,argString[i]);
	      }
	    
	    int installFlag = atoi(argString[0]);
	    installFlag = (installFlag)?1:0;
	    if(i == 5 &&
	       strlen(argString[4]) > (size_t) 0)
	      {
		if(argString[4][strlen(argString[4])-1] == '\n')
		  argString[4][strlen(argString[4])-1] = '\0';
		config_Widget = (Config_Widget **)
		  realloc(config_Widget, sizeof(Config_Widget *)*
			  (Config_count+2));
		Config_Widget *o = config_Widget[Config_count] = new
		  Config_Widget(Config_count, installFlag, 
				argString[1], argString[2],
				argString[3], argString[4]
				);
		launch_scroll->add(o);
		//o->print();
		Config_count++;
		config_Widget[Config_count] = NULL;
	      }
	  }
      }
    fclose(file);
    if(Config_count<1)
      {
	dtkMsg.add(DTKMSG_ERROR,1,
		   "Failed to load any config widgets from file \"%s\".\n",
		   fileName);
	return 1; // error
      }

  }
  int maxHeight = 69 + Config_count*32;
  if(maxHeight > 709)
    maxHeight = 709;
  w->size_range(366, 101, 366, maxHeight, 1, 1, 0);
  w->end();
  w->show(argc, argv);
  w->size(366, maxHeight);
  return 0;
}
