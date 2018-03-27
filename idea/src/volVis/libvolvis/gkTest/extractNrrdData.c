
#include <stdio.h>
#include <stdlib.h>

main ()
    {
    char line[100000];
    int c;


    while (fgets (line, sizeof(line), stdin) != NULL)
        {
        if (line[0] == '\n')
            {
            break;
            }
        }


    while ((c = fgetc(stdin)) != EOF)
        {
        fputc (c, stdout);
        }

    exit (0);
    }
