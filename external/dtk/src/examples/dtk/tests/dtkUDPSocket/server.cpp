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
      return printf("Usage: %s [PORT]\n", argv[0]);

  char *port = (argv[1]) ? (argv[1]) : (char *) "12345";
  dtkUDPSocket sock(PF_INET, port);
  if(sock.isInvalid()) return 1; // error
  sock.print();

  while(1)
    {
#define BUFFER_SIZE ((size_t) 256)

      char buff[BUFFER_SIZE+1];
      ssize_t j = sock.read(buff,BUFFER_SIZE, 30);
      if(j < 0)
        break;// error
      else if(j > 0)
        {
          buff[j]='\0';
          printf(" read(%d) <%s>\n",j, buff);
          if(sock.write(buff,(size_t)j)< 1)
            break;// error
          else
            printf("wrote(%d) <%s>\n", j, buff);
        }
      else // j == 0
        {
          printf("read() timed out after 30 seconds\n");
          return 0;
        }
    }
  return 0;
}
