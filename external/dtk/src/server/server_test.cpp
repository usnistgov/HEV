/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
#include <stdio.h>
#include <string.h>

#include <dtk.h>

#define PROGRAM "server_test"


int Usage(void)
{
  printf("Usage: %s [ADDRESS_PORT] [--udp|-u] | [--help|-h]\n"
	 "\n"
	 "   By default this client uses TCP.\n",
	 PROGRAM);
  return 1;
}

int parse_args(int argc, char ** argv, char **addressPort, int *is_udp)
{
  int i;
  for(i=1;i<argc;i++)
    {
      if(!strcmp("--udp",argv[i]) || !strcmp("-u",argv[i]))
	*is_udp = 1;
      else if(!strcmp("--help",argv[i]) || !strcmp("-h",argv[i]))
	return Usage();
      else
	*addressPort = argv[i];
    }
  return 0;
}

int main(int argc, char ** argv)
{
  char *addressPort =  const_cast<char*>(DTK_DEFAULT_SERVER_CONNECTION);
  int is_udp = 0;
  if(parse_args(argc, argv, &addressPort, &is_udp)) return 1;

  dtkSocket *s;

  if(is_udp)
    s = (dtkSocket *) new dtkUDPSocket(addressPort);
  else
    s = (dtkSocket *) new dtkTCPSocket(addressPort);

  if(!s || s->isInvalid()) return 1;

  printf("See the DTK server protocols in "
	 "\"%s/include/dtk/serverProtocol.h\"\n",
	 dtkConfig.getString(dtkConfigure::ROOT_DIR));

  while(1)
    {
      char buff[1024];

      printf("enter a server request: ");
      fflush(stdout);

      size_t i;
      for(i=0;(buff[i] = getchar()) != '\n';i++ );
      if(i == 0) break;

      size_t req_size = 0;
      {
	// copy from buff to buff while replacing '\''0' with '\0'
	size_t k;
	char *str = buff;
	for(k=0;k<i;k++)
	  {
	    if(buff[k] == '\\' && k+1<i && buff[k+1] == '0')
	      {
		*str = '\0';
		k++;
	      }
	    else
	      *str = buff[k];
	    str++;
	    req_size++;
	  }
      }

      ssize_t j = s->write((const void *) buff, req_size);

      if(j != (ssize_t) req_size)
	return 1;

      j = s->read((void *) buff, (size_t) 1023,
		  (unsigned int) 2 /* timeout seconds */);

      if(j == DTK_SOCKET_TIMEOUT)
	  printf("NO server response in 2 seconds\n");
      if(j > (ssize_t) 0)
	{
	  buff[j] = '\0';
	  printf("server response(size=%d): <",j);
	  {
	    ssize_t m = 0;
	    for(;m<j;m++)
	      {
		if(buff[m])
		  putchar(buff[m]);
		else
		  printf("\\0");
	      }
	  }
	  printf(">\n");
	}
      else if(j < (ssize_t) 0)
	printf("read error\n");
    }
}

