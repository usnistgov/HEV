#include <stdio.h>
#include <stdlib.h> // IRIX needs

#include <dtk.h>

int main(int argc, char **argv)
{
  if(argc < 3)
    return printf("Usage: %s path file [mode]\n"
		  "\n"
		  "  This example program calls "
		  "dtk_getFileFromPath(path, file, mode)\n"
		  "and then prints the file it finds.\n"
		  "mode is assumed to be in octal.\n",
		  argv[0]);
  
  unsigned long int mode = 0;
  if(argc > 3)
    {
      mode =  strtoul(argv[3], NULL, 8);
      if(mode == (unsigned long int) -1)
	{
	  printf("ERROR: fail to convert argv[3]=\"%s\" to a"
		 " unsigned long (mode).\n",
		 argv[3]);
	  return 1;
	}
    }

  char *file = dtk_getFileFromPath(argv[1], argv[2], mode);

  printf("dtk_getFileFromPath(path=\"%s\","
	 "file=\"%s\",mode=0%lo)=\"%s\"\n",
	 argv[1], argv[2], mode,
	 file);

  // If you use this in a program you may want to free memory.
  if(file) free(file);

  return 0;
}

  
