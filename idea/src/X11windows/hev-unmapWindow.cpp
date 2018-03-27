/*
 * unmap all window with specified names
 * for best results specify the window with the X11 option -title
 *
 * John Kelso, 11/06
 */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include <dtk.h>
#include <idea/Utils.h>

int main(int argc, char **argv)
{
  if (argc<2)
    {
      fprintf(stderr,"Usage: %s name ...\n",argv[0]) ;
      return 1 ;
    }

  int ret = 0 ;
  for (unsigned int i=1; i<argc; i++)
  {
      dtkMsg.add(DTKMSG_DEBUG, "hev-unmapWindow: unmapping all windows with name or title \"%s\"\n", argv[i]) ;
      if (!idea::UnmapAllWindowsByDisplayName(NULL, argv[i]))
      {
	  dtkMsg.add(DTKMSG_INFO, "hev-unmapWindow: can't find window with name or title \"%s\"\n", argv[i]) ;
	  ret = 1 ;
      }
  }
  return ret;
}




