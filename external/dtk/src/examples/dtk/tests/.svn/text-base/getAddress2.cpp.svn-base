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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  int fd;
  struct ifconf ifc;
  struct ifreq *ifr;
  struct sockaddr_in *sin;
  char buf[256];

    
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    return 1;
  }

  ifc.ifc_buf = buf;
  ifc.ifc_len = sizeof(buf);
  if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
    perror("ioctl SIOCGIFCONF");
    return 1;
  }

  int i;
  ifc.ifc_len /= sizeof(struct ifreq);
  for (i=0; i<ifc.ifc_len; i++) {
    ifr = (struct ifreq *) &ifc.ifc_req[i];
    sin = (struct sockaddr_in *) &ifr->ifr_addr;
    if (sin->sin_family == AF_INET) {
      printf("interface name = <%s>\n", ifr->ifr_name);
      printf("addr = %s\n", inet_ntoa(sin->sin_addr));
    }
  }

  return 0;
}
