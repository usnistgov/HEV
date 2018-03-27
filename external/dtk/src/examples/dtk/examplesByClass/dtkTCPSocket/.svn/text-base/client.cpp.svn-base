#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dtk.h>

int running = 1;

void sig_caughter(int sig) {
  dtkMsg.add(DTKMSG_NOTICE,
	     "caught signal: %d  ... exiting\n", sig);
  //fclose(stdin); // will force getchar() to return.
  running = 0;
}

int Usage(void) {
  printf("Usage: client DOMAIN TO\n\n"
	 " Where DOMAIN = inet         | unix\n"
	 "  and      TO = address:port | filename\n" 
	 );
  return 1;
}

int main(int argc, char **argv)
{
  if(argc < 3) return Usage();
  int domain = (!strcmp(argv[1],"inet")) ? PF_INET : PF_UNIX;
  dtkTCPSocket sock(argv[2], domain);
  if(sock.isInvalid()) return 1; // error

  sock.print();

  //signal(SIGINT, sig_caughter);
  //signal(SIGTERM, sig_caughter);
  //signal(SIGPIPE, sig_caughter);

  while(running)
    {
      const size_t BUFFER_SIZE = (size_t) 1024;

      char buff[BUFFER_SIZE+1];
      printf("type stuff and hit <enter>: ");
      fflush(stdout);
      int i;
      for(i=0;(buff[i] = getchar()) != '\n';i++ );

      if(i==0) return 0; // exit
      if(sock.write((const void *)buff,(size_t)i)< 1)
	{
	  return 1; // error
	}
      else
	{
	  unsigned int timeOut = 20; // seconds
	  ssize_t j = sock.read((void *)buff, BUFFER_SIZE, timeOut);
	  if(j > 0)
	    {
	      buff[j]='\0';
	      printf(" read(%d) <%s>\n",j,buff);
	    }
	  else if(j == DTK_SOCKET_TIMEOUT)
	    {
	      printf("read() timed out after %d seconds or lost server\n",
		     timeOut);
	      return 0;
	    }
	  else if(j == -1)
	    return 1; // error
	}
    }
  return 0;
}
