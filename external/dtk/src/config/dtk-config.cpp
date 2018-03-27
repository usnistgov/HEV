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
 */
#include "_config.h"
#include "config.h"

#include <stdio.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <io.h>
#else
# include <unistd.h>
#endif /* DTK_ARCH_WIN32_VCPP */
#include <string.h>
#include <stdlib.h>

#include "privateConstants.h"
#include "dtkConfigure.h"
#include "dtkColor.h"

// color escape sequences

#define END     "\033[0m"
#define RED     "\033[31m"
#define GRN     "\033[32m"
#define VIL     "\033[35m"
#define TUR     "\033[36m"


extern int dtk_printEnv(FILE *file=stdout, int is_color=DTKCOLOR_AUTO);

static char *getEdit(void)
{
  static char str[16];
  strcpy(str, DTK_VERSION);
  char *edit=str;
  while(*edit != '.')
    edit++;
  edit++;
  while(*edit != '.')
    edit++;
  edit++;
  return edit;
}

static char *getMajor(void)
{
  static char str[16];
  strcpy(str, DTK_VERSION);
  char *end=str;
  while(*end != '.')
    end++;
  *end = '\0';
  return str;
}

static char *getMinor(void)
{
  static char str[16];
  strcpy(str, DTK_VERSION);
  char *minor=str;
  while(*minor != '.')
    minor++;
  minor++;
  char *end=minor;
  while(*end != '.')
    end++;
  *end = '\0';
  return minor;
}

static int Usage(int returnVal)
{
  char *end = const_cast<char*>("\0"), *tur = const_cast<char*>("\0");
  if(isatty(fileno(stdout)))
    end =  const_cast<char*>(END), tur =  const_cast<char*>(TUR);

  printf("Usage: dtk-config [--about|--augment-dso-dir|--compiler|--cxxflags|\\\n"
         "    --date|--env|--filter-dso-dir|--homepage|--include|--lib-dir|\\\n"
         "    --libs|--libsX|--os|--other-libs|--port|--root|--service-dso-dir|\\\n"
	 "    --sharedMem-dir|--source|--test|--version|--version-edit|\\\n"
	 "    --version-major|--version-minor]\n"
         "\n"
         "Get information about the DIVERSE Toolkit (DTK) and this DTK installation.\n"
         "dtk-config helps you compile and link your code with a given DTK installation,\n"
         "which in affect makes your DTK application code more portable.  The output of\n"
         "dtk-config depends on a given DTK installation, and the environment variable\n"
	 "%sDTK_ROOT%s.  The environment variable %sDTK_ROOT%s is in general not required to be\n"
	 "set.  The environment variable %sDTK_ROOT%s is needed if you installed a binary\n"
	 "release of DTK not in the default installation directory, so that running DTK\n"
	 "programs can find DTK files that may be needed.\n"
	 "\n",
	 tur,end, tur,end,tur,end);

  printf(" --about    print stuff about DTK and this DTK installation\n"
	 " --augment-dso-dir print the directory of the installed dtkAugment DSOs\n"
         " --compiler print the program name of the C++ compiler used to compile DTK code\n"
         " --cxxflags print compiler (-I) include paths and related compiler options\n"
	 " --date     print the DTK release date (%s)\n"
         " --env      print all about DTK environment variables and there current state\n"
	 " --filter-dso-dir print the default shared memory filter DSO directory\n"
         " --include  print compiler -I include path options\n"
	 " --homepage print the DTK home page\n"
         " --lib-dir  print the directory that the DTK library is in\n"
         " --libs     print DTK compile link library options\n"
         " --libsX    print DTK compile link library options including libdtk and libdtkX11\n"
	 " --os       print the OS (%s).\n",
	 DTK_RELEASE_DATE, DTK_ARCH);

  printf(" --port     print the default DTK network socket port\n"
         " --root     print the top DTK installation directory\n"
         " --service-dso-dir print the default directory where the DTK service DSOs are\n"
	 " --sharedMem-dir   print the default shared memory directory ignoring the\n"
	 "            environment variable DTK_SHAREDMEM_DIR\n"
         " --source   print the top DTK source directory\n"
         " --test     returns 0, lets you know `dtk-config' can run\n"
         " --version  print the DTK version number (%s)\n"
	 " --version-edit  print the DTK edit version number (%s)\n"
	 " --version-major print the DTK major version number (%s)\n"
	 " --version-minor print the DTK minor version number (%s)\n"
	 "\n",
         DTK_VERSION, getEdit(), getMajor(), getMinor());

  return returnVal;
}




int main(int argc, char **argv)
{
  if(argc < 2 || argc > 3)
    return Usage(1);

  if(!strncmp("--about",argv[1],4))
    {  
      char *end = const_cast<char*>("\0"), *vil = const_cast<char*>("\0"), *tur = const_cast<char*>("\0"), *grn = const_cast<char*>("\0");
      if(isatty(fileno(stdout)))
	end =  const_cast<char*>(END), vil =  const_cast<char*>(VIL), tur =  const_cast<char*>(TUR), grn =  const_cast<char*>(GRN);

      printf("\n"
	     " ---------------- %sThe DIVERSE Toolkit%s -------------\n"
	     "\n"
	     "version %s%s%s\n"
	     "release date: %s\n"
	     "The DIVERSE Toolkit homepage: %s%s%s\n"
	     "compiled on the date: %s\n"
	     "by user: %s on host: %s\n"
	     "The top installation directory is: %s%s%s\n"
	     "The building source directory was: %s%s%s\n"
	     "The building compile directory was: %s%s%s\n"
	     "\n"
	     "The DIVERSE Toolkit comes with ABSOLUTELY NO WARRANTY.\n"
	     "This is free software, and you are welcome to\n"
	     "redistribute it under certain conditions.\n"
	     "The DIVERSE Toolkit (DTK) librarys and the DTK server are\n"
	     "Licensed under the GNU Lesser General Public License.\n"
	     "See http://www.gnu.org/copyleft/lesser.html for License details.\n"
	     "This program, and"
	     " the rest of DIVERSE Toolkit are\n"
	     "Licensed under the GNU General Public License.\n"
	     "See http://www.gnu.org/copyleft/gpl.html for License details.\n"
	     "\n",
	     grn,end,
	     vil, DTK_VERSION,end, DTK_RELEASE_DATE,
	     tur,DTK_HOMEPAGE,end, DTK_COMPILE_DATE,
	     DTK_WHO_COMPILED, DTK_COMPILE_HOST,
	     vil, dtkConfig.getString(dtkConfigure::ROOT_DIR), end,
	     vil, DTK_SOURCE_DIR, end, vil, DTK_BINARY_DIR, end);
      return 0;
    }
  if(!strcmp("--augment-dso-dir",argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::AUGMENT_DSO_DIR));
      return 0;
    }
  // hidden option
  if(!strcmp("--build-libs", argv[1]))
    {
      printf("lib_dtk lib_dtkX11 lib_dtkFLTK\n");
      return 0;
    }
  // remove "--cflags" in the future
  if(!strcmp("--cxxflags",argv[1]) || !strcmp("--cflags",argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::CXXFLAGS));
      return 0;
    }
  if(!strcmp("--compiler",argv[1]))
    {
      printf("%s\n", DTK_COMPILER);
      return 0;
    }
  if(!strcmp("--date",argv[1]))
    {
      printf("%s\n", DTK_RELEASE_DATE);
      return 0;
    }
  if(!strncmp("--env",argv[1],3))
    {
      dtk_printEnv();
      return 0;
    }
  if(!strcmp("--filter-dso-dir",argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::FILTER_DSO_DIR));
      return 0;
    }
  if(!strcmp("--include", argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::INCLUDE_FLAGS));
      return 0;
    }
  if(!strcmp("--homepage", argv[1]))
    {
      printf("%s\n",DTK_HOMEPAGE);
      return 0;
    }
  if(!strcmp("--lib-dir", argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::LIB_DIR));
      return 0;
    }
  if(!strcmp("--libs", argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::LINK_LIBS));
      return 0;
    }
  if(!strcmp("--libsX", argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::LINK_LIBSX));
      return 0;
    }
  if(!strcmp("--os", argv[1]))
    {
      printf("%s\n", DTK_ARCH);
      return 0;
    }
  if(!strcmp("--other-libs", argv[1]))
    {
      printf("%s\n", DTK_OTHER_LDFLAGS);
      return 0;
    }
  if(!strcmp("--port", argv[1]))
    {
      printf("%s\n", DTK_DEFAULT_SERVER_PORT);
      return 0;
    }
  if(!strcmp("--root", argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::ROOT_DIR));
      return 0;
    }
  // remove "--server-dso-dir" in the future
  if(!strcmp("--service-dso-dir",argv[1]) || !strcmp("--server-dso-dir", argv[1]))
    {
      printf("%s\n", dtkConfig.getString(dtkConfigure::SERVICE_DSO_DIR));
      return 0;
    }
  if(!strcmp("--source", argv[1]))
    {
      printf("%s\n", DTK_SOURCE_DIR);
      return 0;
    }
  if(!strncmp("--shared", argv[1],8))
    {
      printf("%s\n", DTK_DEFAULT_SHAREDMEM_DIR);
      return 0;
    }
  if(!strcmp("--test", argv[1]))
    {
      return 0;
    }
  if(!strcmp("--version", argv[1]))
    {
      printf("%s\n", DTK_VERSION);
      return 0;
    }
  if(!strcmp("--version-edit", argv[1]))
    {
      printf("%s\n", getEdit());
      return 0;
    }
  if(!strcmp("--version-major", argv[1]))
    {
      printf("%s\n", getMajor());
      return 0;
    }
  if(!strcmp("--version-minor", argv[1]))
    {
      printf("%s\n", getMinor());
      return 0;
    }

  return Usage(1);
}
