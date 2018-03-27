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
// written by: Chris Wright linux-ha-dev@lists.community.tummy.com

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
        int     fd;
        struct ifreq    if_info;
        struct in_addr  addr;
        char            *ifname = NULL;

        if (argc >= 2)
                ifname = argv[1];       

        memset(&if_info, 0, sizeof(if_info));
        if (ifname) {
                strncpy(if_info.ifr_name, ifname, IFNAMSIZ-1);
        } else {        /* ifname is NULL, so use any address */
                addr.s_addr = INADDR_ANY;
                printf("addr = %s\n", inet_ntoa(addr));
                return 0;
        }

        if ((fd=socket(AF_INET, SOCK_DGRAM, 0)) == -1)  {
                printf("Error getting socket\n");
                return -1;
        }

        printf( "looking up address for %s\n", if_info.ifr_name);

        if (ioctl(fd, SIOCGIFADDR, &if_info) == -1) {
                printf("Error ioctl(SIOCGIFADDR)\n");
                close(fd);
                return -1;
        }
        memcpy(&addr, &((struct sockaddr_in *)&if_info.ifr_addr)->sin_addr,
                sizeof(struct in_addr));
        close(fd);

        printf("addr = %s\n", inet_ntoa(addr));
        return 0;
}
