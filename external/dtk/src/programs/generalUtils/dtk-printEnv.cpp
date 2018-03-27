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
#include <dtk.h>

// Just checking chars that may make trouble when pasting to shells.
static int check_special_space(const char *s)
{
  for(;
      !(*s < (char) 053) &&
	!(*s > (char) 072 && *s < (char) 077) &&
	!(*s > (char) 0132 && *s < (char) 0141) &&
	!(*s > (char) 0172) &&
	*s != '|' &&
	*s != '>' &&
	*s != '<'
	;
      s++);
  return ((*s));
}

int main(int argc, char **argv)
{
  if(argc > 1)
    {
      printf("\n Usage: dtk-printEnv [--help|-h]\n\n"
	     "   dtk-printEnv prints the current "
	     "DTK environment variables.\n\n");
      return 1;
    }

  dtkColor color(stdout);
  char *dtk_env[] =
    {
      const_cast<char*>("DTK_SHAREDMEM_DIR"),
      const_cast<char*>("DTK_COLOR"),
      const_cast<char*>("DTK_CONNECTION"),
      const_cast<char*>("DTK_DSO_PATH"),
      const_cast<char*>("DTK_PORT"),
      const_cast<char*>("DTK_ROOT"),
      const_cast<char*>("DTK_SPEW"),
      const_cast<char*>("DTK_SPEW_FILE"),
      const_cast<char*>("FLTK_ROOT"),
      NULL
    };

  char *export_syntex = const_cast<char*>("export");
  char equal = '=';
  
  char *shell = getenv("SHELL");

  char *non_shLike_shell[] = { const_cast<char*>("tcsh"), const_cast<char*>("csh"), NULL };
  char **shells = non_shLike_shell;

  if(shell)
    for(;*shells; shells++)
      {
	if(strlen(shell) >= strlen(*shells) &&
	   !strcmp(&(shell[strlen(shell) - strlen(*shells)]),
		   *shells))
	  {
	    export_syntex = const_cast<char*>("setenv");
	    equal = ' ';
	    break;
	  }
      }

  char **str = dtk_env;
  for(;*str;str++)
    {
      char *env = getenv(*str);
      if(env)
	{
	  if(check_special_space(env))
	    printf("%s %s%s%s%c%s'%s'\n",
		   export_syntex, color.tur,
		   *str, color.yel, equal, color.end, env);
	  else
	    printf("%s %s%s%s%c%s%s\n",
		   export_syntex, color.tur,
		   *str, color.yel, equal, color.end, env);
	}
    }
  return 0;
}
