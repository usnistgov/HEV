#include <stdio.h>

main ()
        {
        char line[1000];
        char a[1000];
        char b[1000];
        char c[1000];
        char tok[1000];


        while (fgets (line, sizeof(line), stdin) != NULL)
                {
                if (sscanf (line, "%s %s %s", a, b, c) == 3)
                        {
                        if ( (strcasecmp (a, "ElementDataFile") == 0) &&
                             (strcasecmp (b, "=") == 0) &&
                             (strcasecmp (c, "LOCAL") == 0)                 )
                                {
                                break;
                                }
                        }
                }

        while (fscanf (stdin, "%s", tok) == 1)
                {
                printf ("%s\n", tok);
                }

        return 0;
        }


