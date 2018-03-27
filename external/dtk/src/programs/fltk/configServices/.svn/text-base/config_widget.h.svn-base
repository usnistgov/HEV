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
#define NUM_STR  6

class Config_Widget : public Fl_Group
{
public:

  Config_Widget(int count, char *name,char *directory,
		char *configScript, char *helpScript,
		int isInstalled);

  Config_Widget(int num, int install, 
		char *directory, char *name,
		char *configScript, char *helpScript);
  ~Config_Widget(void);

  int getCount(void) const;
  void print(FILE *file=stdout) const;
  int org_installFlag;
  Fl_Check_Button *install_Button;

  Fl_Button *helpButton;
  Fl_Button *configButton;

  pid_t help_pid;
  pid_t config_pid;

  friend void help_callback(Fl_Widget *who, void *data);
  friend void config_callback(Fl_Widget *who, void *data);

private:

  char *str[NUM_STR];
  int yPos;
  int count;
};
