#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <dtk.h>

int running = 1;

void signal_catcher(int sig)
{
  dtkMsg.add(DTKMSG_NOTICE,
	     "caught signal %d ... exiting.\n", sig);
  running = 0;
}

int run_sliders(void)
{
  pid_t pid = fork();
  if(pid < (pid_t) 0) return 1; // error
  if(pid == (pid_t) 0) // I'm the child
    {
      // Run dtk-floatSliders
      execlp("dtk-floatSliders", "dtk-floatSliders",
	     "foo_float", "--number=1", NULL);
      dtkMsg.add(DTKMSG_WARN, 1,
		 "execlp(\"dtk-floatSliders\", ...) failed.\n");
      return 1;
    }
  return 0; // success
}


int main(void)
{
  if(run_sliders()) return 1; // error

  printf("MOVE THE SLIDER\n");
  float x = 0.0f;
  dtkSharedMem shm(sizeof(float), "foo_float", &x);
  if(shm.isInvalid()) return 1; // error

  signal(SIGINT, signal_catcher);
  signal(SIGTERM, signal_catcher);
  signal(SIGQUIT, signal_catcher);
  signal(SIGCHLD, signal_catcher);

  if(shm.queue()) return 1; // error

  int i = 1;
  while(running)
    {
      i = shm.qread(&x);
      while(i)
	{
	  if(i<0) return 1; // error
	  printf("%g\n", x);
	  i = shm.qread(&x);
	}
      usleep(100000); // sleep 0.1 seconds
    }

  return 0;
}

