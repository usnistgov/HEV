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
#include <string.h>
#include <dtk.h>

int main(int argc, char **argv)
{
  int i=1;
  for(;i<argc;i++)
    if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
      return printf("Usage: %s [ADDRESS:PORT]\n", argv[0]);

  char *addressPort = "localhost:12345";
  if(argv[1])
    addressPort = argv[1];

  // This will connect.
  dtkTCPSocket sock(addressPort);
  if(sock.isInvalid()) return 1; // error

  while(1)
    {
      printf("type here and <ENTER> ===> ");
      fflush(stdout);
      char buffer[256];
      int i=0,k;
      for(;i<255 && (k = getchar()) != EOF && k != (int) '\n';i++)
	buffer[i] = k;
      if(i>0)
	{
	  buffer[i] = '\0';
	  if(i+1 != sock.write(buffer, i+1))
	    return 1; // error
	  ssize_t j = sock.read(buffer,255,20);
          if(j < 0)
            return 1; // error
          else if(j > 0)
            {
              buffer[j]='\0';
              printf(" read(%d) <%s>\n",j,buffer);
            }
          else // j == 0
            {
              printf("read() timed out after 20 seconds\n");
              return 0;
            }
	}
    }
  return 0;
}
