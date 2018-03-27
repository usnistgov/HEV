// Example code using class dtkTime

/* dtkTime is a small gettimeofday() wrapper.  It returns time as long
 * double so that it contains the full resolution of the system clock.
 * double will not work.
 */

#include <stdio.h>
#include <dtk.h>


int main(void)
{
  dtkTime t;

  int i;
  for(i=0;i<5;i++)
    printf("      time=%.12Lf\n",t.get());
  for(i=0;i<5;i++)
    printf("delta time=%.12Lf\n",t.delta());
  printf("reset(0.0,1) sets time to zero\n");
  t.reset(0.0,1);
  for(i=0;i<5;i++)
    printf("     time=%.12Lf\n",t.get());
  printf("reset(0.0) Sets offset to zero"
	 " so that time is the number of seconds "
	 "since Jan1,1970\n");
  t.reset(0.0);
  for(i=0;i<5;i++)
    {
      long double T = t.get();
      printf("     time=%.12Lf sec /(365*24*3600) = %.15Lg years\n",
	     T,T/(365*24*3600));
    }
  printf("I'll bet most of this time is used in printing.\n");
  
  return 0;
}
