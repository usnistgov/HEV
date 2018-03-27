
#include <stdio.h>

main ()
	{
	float f;

	while (scanf ("%f", &f) == 1)
		{
		fwrite (&f, sizeof(f), 1, stdout);
		}

	
	}
