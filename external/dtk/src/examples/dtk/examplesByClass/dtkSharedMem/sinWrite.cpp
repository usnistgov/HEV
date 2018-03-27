#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <math.h>
#include <dtk.h>

int running = 1;

void signal_catcher(int sig)
{
  dtkMsg.add(DTKMSG_NOTICE,
	     "caught signal %d ... exiting.\n", sig);
  running = 0;
}

int run_scope(void)
{
  pid_t pid = fork();
  if(pid < (pid_t) 0) return 1; // error
  if(pid == (pid_t) 0) // I'm the child
    {
      // Run dtk-floatScope
      execlp("dtk-floatScope", "dtk-floatScope",
	     "foo_float", NULL);
      dtkMsg.add(DTKMSG_WARN, 1,
		 "execlp(\"dtk-floatScope\", ...) failed.\n");
      return 1;
    }
  return 0; // success
}


int main(void)
{
  if(run_scope()) return 1; // error

  dtkTime t;
  float x = sinf((float) t.get()*4.0);
  dtkSharedMem shm(sizeof(float), "foo_float", &x);
  if(shm.isInvalid()) return 1; // error

  signal(SIGINT, signal_catcher);
  signal(SIGTERM, signal_catcher);
  signal(SIGQUIT, signal_catcher);
  signal(SIGCHLD, signal_catcher);

  while(running)
    {
      x = sinf((float) t.get()*4.0f);
      if(shm.write(&x)) return 1; // error
      usleep(10000); // sleep 0.01 seconds
    }
  return 0;
}

