
#include <stdio.h>

main ()
    {
    char line[1000];
    
    while (fgets(line, sizeof(line), stdin) != NULL)
        {
        if (strcmp (line, "ElementDataFile = LOCAL\n") == 0)
            {
            break;
            }
        }


    float rgba[4];

    while (fread (rgba, 4*sizeof(float), 4, stdin) == 4)
        {
        printf ("%.9g\n", rgba[3]);
        }

    }
