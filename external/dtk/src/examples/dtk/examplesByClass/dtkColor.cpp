// Simple example using dtkColor

// Setting env DTK_COLOR=off will turn off the color.

#include <dtk.h>

int main(void)
{
  dtkColor c;

  printf("%srev red%s %sred%s %syellow%s %sgreen%s\n",
	 c.rred, c.end,
	 c.red, c.end,
	 c.yel, c.end,
	 c.grn, c.end);

  return 0;
}
