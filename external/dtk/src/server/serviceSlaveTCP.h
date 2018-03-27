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
// It keeps a linked list to many of it's kind using dtkSocketList
// serverSocketList; from dtk-server.C

// This TCP socket based service, ServiceSlaveTCP, and the one other
// TCP socket based service, ServiceClientTCP, are the two services
// that define the web of connections between DTK servers.  This web
// of TCP connects provides a way for server to "reliably" know when
// DTK servers come and go.  If a TCP connection is broken, it is
// assumed that any remote shared memory that are associated with a
// remote server is to be disconnected, and it is removed from the
// associated shared memory write lists that are keep in the DTK
// shared memory files.

// In addition, ServiceSlaveTCP also connects to dtkClients to
// send command requests to the DTK server.

class DTKSERVERAPI ServiceSlaveTCP : public dtkService
{
public:

  ServiceSlaveTCP(dtkTCPSocket *s, const char *addressPort, int domain);
  virtual ~ServiceSlaveTCP(void);
  int serve(void);
  void stop(void);

private:

  dtkTCPSocket *sock;

  // This is to id the remote end point.
  char *addressPort;

};

