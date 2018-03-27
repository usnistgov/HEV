
#include <stdio.h>

main ()
        {
        double d;

        while (scanf ("%lf", &d) == 1)
                {
                if (d == 0.0)
                        {
                        printf ("0\n");
                        }
                else
                        {
                        printf ("%.6e\n", d);
                        }
                }
        }
