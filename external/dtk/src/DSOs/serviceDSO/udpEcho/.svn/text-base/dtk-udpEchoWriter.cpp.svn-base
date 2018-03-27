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
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dtk.h>
#include "config.h"



int Usage(void)
{
  printf("  Usage: dtk-udpEchoWriter [ADDRESS:PORT] [-h|--help]\n\n" 
	 " dtk-udpEchoWriter writes from stdin to a Internet domain\n"
	 " UDP socket.\n\n"
	 "  ADDRESS:PORT   use the socket address and port ADDRESS:PORT,\n"
	 "                 the default is %s\n"
	 "  -h|--help      print this help and then exit\n\n",
	 DEFAULT_SOCKET_ADDRESS_PORT);
  return 1;
}

int main(int argc, char **argv)
{
  if(argc > 3 ||
     (argc > 1 &&
      (!strcmp("-h",argv[1]) || !strcmp("--help",argv[1]))))
    return Usage();

  dtkMsg.setPreMessage("dtk-udpEchoWriter");

  char *addressPort =  const_cast<char*>(DEFAULT_SOCKET_ADDRESS_PORT);
  if(argc > 1)
    addressPort = argv[1];

  dtkUDPSocket sock(PF_INET);
  if(sock.isInvalid() || sock.setRemote(addressPort)) return 1; // error
  sock.print();

  while(1)
    {
#define BUFFER_SIZE ((size_t) (DTK_SHARED_MEMORY_SIZE))

      char buff[BUFFER_SIZE+1];
      printf("> ");
      fflush(stdout);
      size_t i;
      for(i=0;i<BUFFER_SIZE-1 && (buff[i] = getchar()) != '\n' ;i++ );
      buff[i++] = '\0';
      if(sock.write(buff, i)< 1)
	return 1; // error
    }
  return 0;
}
