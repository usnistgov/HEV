#include <dtk.h>
#include <dtk/dtkService.h>

/*
 * This is a simple DTK DSO loadable plug-in service that is a generic
 * TCP server that all incomming data echos to stdout.  There is one
 * bound TCP socket that waits for input on a known service port.
 * This bound TCP socket we call the master socket.  This master
 * socket sets up slave sockets that are "connected" to the incomming
 * requests.
 *
 * The DTK server just provides the multiplexing of the incomming
 * data.  You can set the environment variable DTK_SPEW_LEVEL=warning
 * or DTK_SPEW_LEVEL=error to keep the DTK spew from inter-mixing with
 * the output of this service.  You'll have to not run the dtk-server
 * as a daemon in order to load in use this service DSO.
 *
 * If you remove the printf()s this can then be run with a daemon DTK
 * server.
 *
 */

class TCPEcho : public dtkService
{
public:
  TCPEcho(void);
  virtual ~TCPEcho(void);
  int serve(void);

private:
  dtkTCPSocket sock;
};


class TCPSlave : public dtkService
{
public:
  TCPSlave(dtkSocket *slave_in);
  virtual ~TCPSlave(void);
  int serve(void);

private:
  dtkSocket *sock;
  char *sname;
};


//                            domain,  "know service port"
TCPEcho::TCPEcho(void) : sock(PF_INET, "1234")
{
  if(sock.isInvalid()) return; // error

  if(dtkMsg.isSeverity(DTKMSG_INFO))
    {
      dtkMsg.add(DTKMSG_INFO,"");
      sock.print(dtkMsg.file());
    }

#ifdef DTK_ARCH_WIN32_VCPP
  fd = (HANDLE) 1;
#else
  fd = sock.getFileDescriptor();
#endif

  return;
}


int TCPEcho::serve(void)
{
  dtkSocket *slave = sock.accept(0);
  if(!slave) return DTKSERVICE_ERROR;

  new TCPSlave(slave);

  return DTKSERVICE_CONTINUE;
}


TCPEcho::~TCPEcho(void)
{
  // If there are any slaves working now they are from here out free,
  // the master is dead, and this is his/her will.
}

TCPSlave::TCPSlave(dtkSocket *slave_in): sock(slave_in), sname(NULL)
{
#ifdef DTK_ARCH_WIN32_VCPP
  fd = NULL;
#else
  fd = sock->getFileDescriptor();
#endif
  
  if(dtkMsg.isSeverity(DTKMSG_INFO))
    {
      dtkMsg.add(DTKMSG_INFO,"");
      sock->print(dtkMsg.file());
    }

  // Add this slave TCP socket to the list of DTK services.  The DTK
  // server will use select() or a thread to use a blocking read for
  // the service.
  serviceManager.add(this);

  // Let the DTK server handle the clean up.
  serviceManager.deleteOnRemove(this);

  sname = dtk_strdup(serviceManager.getName(this));
}

int TCPSlave::serve(void)
{
  const size_t BUFFER_SIZE = 256;
  char buff[BUFFER_SIZE+1];
  
  // READ
  ssize_t bytesRead = sock->read(buff,BUFFER_SIZE);
  if(bytesRead > 0 && bytesRead <= (ssize_t) BUFFER_SIZE)
    {
      buff[bytesRead]='\0';
      printf("%s  read(%d) <%s>\n", sname, bytesRead, buff);
    }
  else if(bytesRead == 0) // connection is ending.
    return DTKSERVICE_UNLOAD; // tell DTK server to unload this
			      // service
  else // error (bytesRead < 0 || bytesRead > BUFFER_SIZE)
    return DTKSERVICE_ERROR;

  // WRITE echo back to client
  if(sock->write(buff,(size_t)bytesRead)< 1)
    return DTKSERVICE_ERROR;
  else
    printf("%s wrote(%d) <%s>\n", sname, bytesRead, buff);

  return DTKSERVICE_CONTINUE;
}

TCPSlave::~TCPSlave(void)
{
  if(sname)
    {
      dtk_free(sname);
      sname = NULL;
    }
  delete sock;
}

static dtkService *dtkDSO_loader(const char *port)
{
  return new TCPEcho;
}

static int dtkDSO_unloader(dtkService *tcpEcho)
{
  delete tcpEcho;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
