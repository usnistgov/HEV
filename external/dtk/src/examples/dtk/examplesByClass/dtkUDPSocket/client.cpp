#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dtk.h>

int running = 1;

void sig_caughter(int sig) {
  dtkMsg.add(DTKMSG_NOTICE,
	     "caught signal: %d  ... exiting\n", sig);
  fclose(stdin); // will force getchar() to return.
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
  dtkUDPSocket sock(domain);
  if(sock.isInvalid()) return 1; // error
  if(sock.setRemote(argv[2])) 
    return 1; // error
  sock.print();

  signal(SIGINT, sig_caughter);
  signal(SIGTERM, sig_caughter);
  signal(SIGPIPE, sig_caughter);

  while(running)
    {
#define BUFFER_SIZE ((size_t) 1024)

      char buff[BUFFER_SIZE+1];
      printf("type stuff and hit <enter>: ");
      fflush(stdout);
      int i;
      for(i=0;(buff[i] = getchar()) != '\n';i++ );
      if(i==0) return 0; // exit
      if(sock.write((const void *)buff,(size_t)i)< 1)
	return 1; // error
      else
	{
	  ssize_t j = sock.read((void *)buff,BUFFER_SIZE,20);
	  if(j < 0)
	    return 1; // error
	  else if(j > 0)
	    {
	      buff[j]='\0';
	      printf(" read(%d) <%s>\n",j,buff);
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
