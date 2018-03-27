#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dtk.h>

int running = 1;

void sig_caughter(int sig) {
  dtkMsg.add(DTKMSG_NOTICE,
	     "caught signal: %d  ... exiting\n", sig);
  running = 0;
}

int Usage(void) {
  printf("Usage: server DOMAIN PORT\n\n"
	 " Where DOMAIN = inet        | unix\n"
	 "  and   PORT  = port number | filename\n"
	 );
  return 1;
}


int main(int argc, char **argv)
{
  if(argc < 3) return Usage();
  int domain = !strcmp(argv[1],"inet")?PF_INET:PF_UNIX;
  // argv[2] = NULL will work too.
  dtkTCPSocket sock(domain,argv[2]);
  if(sock.isInvalid()) return 1; // error
  sock.print();

  signal(SIGINT, sig_caughter);
  signal(SIGTERM, sig_caughter);
  signal(SIGPIPE, sig_caughter);

  dtkSocket *slave = sock.accept(0);
  if(!slave) return 1;
  slave->print(stdout);

  while(running)
    {
      const size_t BUFFER_SIZE = (size_t) 1024;
      const unsigned int waitSeconds = 20;
      char buff[BUFFER_SIZE+1];
      ssize_t j = slave->read(buff,BUFFER_SIZE, waitSeconds);
      if(j > 0)
	{
	  buff[j]='\0';
	  printf(" read(%d) <%s>\n",j, buff);
	  if(slave->write(buff,(size_t)j)< 1)
	    break;// error
	  else
	    printf("wrote(%d) <%s>\n", j, buff);
	}
      else if(j == DTK_SOCKET_TIMEOUT)
	{
	  printf("read() timed out after %d seconds or lost client\n",
		 waitSeconds);
	  return 0;
	}
      else if(j == -1)
	break;// error
    }
  delete slave;
  return 0;
}
