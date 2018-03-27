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
#include <stdio.h>

int main(int argc, char **argv)
{
  dtkColor color(stdout);  

  printf("\n/**************************** EXAMPLE code ******************************/\n"
	 "/* compile with:\n"
	 " *  `dtk-config --compiler` `dtk-config --include` x.cpp `dtk-config --libs`\n"
	 " */\n"
	 "#include <stdio.h>\n"
	 "#include <dtk.h>\n"
	 "int main(void) {\n"
	 "   dtkColor color(stdout);\n"
	 "   printf(\" %%sThis text is red%%s\\n\",color.red,color.end);");
  printf("\n"
	 "   return 0;\n"
	 " }\n"
	 "/************************************************************************/\n");

  printf("\n produces the text:\n");
  
  printf(" %sThis text is red%s\n",
	 color.red,color.end);

  printf("\n Here is a list of all the color escape strings that are in\n"
	 " the class %sdtkColor%s and the globel dtkColor object %sdtkMsg.color%s\n\n",
	 color.grn,color.end, color.tur,color.end);

  printf("   %sbld%s bld\n",color.bld,color.end);
  printf("   %sund%s und\n\n",color.und,color.end);
  printf("   %srev%s rev\n",color.rev,color.end);
  printf("   %sred%s red\n",color.red,color.end);
  printf("   %sgrn%s grn\n",color.grn,color.end);
  printf("   %syel%s yel\n",color.yel,color.end);
  printf("   %sblu%s blu\n",color.blu,color.end);
  printf("   %svil%s vil\n",color.vil,color.end);
  printf("   %stur%s tur\n",color.tur,color.end);
  printf("  %srred%s rred\n",color.rred,color.end);
  printf("  %srgrn%s rgrn\n",color.rgrn,color.end);
  printf("  %sryel%s ryel\n",color.ryel,color.end);
  printf("  %srblu%s rblu\n",color.rblu,color.end);
  printf("  %srvil%s rvil\n",color.rvil,color.end);
  printf("  %srtur%s rtur\n\n",color.rtur,color.end);

  printf(" The environment variable %sDTK_COLOR%s can be set to\n"
	 " \"%sON%s\", \"%sOFF%s\","
	 " and \"%sAUTO%s\" to the ecsape sequences in these strings.\n\n",
	 color.tur, color.end, color.vil, color.end,
	 color.vil, color.end, color.vil, color.end);

  printf(" Try running: `env DTK_COLOR=OFF dtk-checkColor'\n\n");
   
  return 0;
}
