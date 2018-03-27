// Example code using class dtkMessage

/* dtkMessage is the DTK error message class.  You can set messages
 * and print them.  Most DTK classes use dtkMsg which is a global
 * dtkMessage object that is in the DTK library.  Set environment
 * variable DTK_SPEW to DEBUG INFO NOTICE WARN ERROR or FATAL to
 * change the spew level of the dtkMessage objects.
 *
 * Run `dtk-config --env' for more information about other DTK
 * environment variables. */

#include <stdio.h>
#include <dtk.h>

int foo(int i)
{
  dtkMsg.add(DTKMSG_DEBUG, "trying %d\n",i);
  dtkMsg.add(DTKMSG_INFO, "doing %d\n",i);
  dtkMsg.append("          yes really\n");
  if(i < 4)
    {
      dtkMsg.add(DTKMSG_NOTICE, "got %d\n",i);
      return 0;
    }
  else if(i < 5)
    {
      dtkMsg.add(DTKMSG_WARN, "got %d\n",i);
      return 0;
    }
  else
    {
      dtkMsg.add(DTKMSG_ERROR, "got %d\n",i);
      return -1;
    }
}


int main(void)
{
  dtkMsg.setPreMessage("%sFOO%s",
		       dtkMsg.color.tur,
		       dtkMsg.color.end);
  int i;
  for(i=0;;i++)
    if(foo(i)) return 1;

  return 0;
}
