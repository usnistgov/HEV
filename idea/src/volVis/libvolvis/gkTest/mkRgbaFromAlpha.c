
#include <stdio.h>

main ()
    {
    char tok[1000];

    while (fscanf (stdin, "%s", tok) == 1)
        {
        printf ("0.8 0.8 0.8 %s\n", tok);
        }

    }
