
#include <stdio.h>
#include <errno.h>

int
main (int argc, char **argv)
        {
        FILE *fp;
        int nVal;
        int i;
        unsigned char val;
        int sz;

        sz = sizeof(val);

        nVal = atoi (argv[1]);

        fp = fopen (argv[2], "r");
        if (fp == NULL)
                {
                fprintf (stderr, "Unable to open file %s\n", argv[2]);
                return -1;
                }


        if (fseek (fp, 0L, SEEK_END))
                {
                fprintf (stderr, "Error with fseek to end.\n");
                return -1;
                }
        long fileLen = ftell (fp);

        long dataLen = nVal * sizeof (val);

        if (dataLen > fileLen)
                {
                fprintf (stderr, 
                  "Error: asking for more data (%ld) than is in file (%ld).\n", 
                  dataLen, fileLen);
                return -1;
                }

        if (dataLen == fileLen)
                {
                rewind (fp);
                }
        else
                {
                if (fseek (fp, fileLen - dataLen, SEEK_SET))
                        {
                        fprintf (stderr, "Error with fseek\n");
                        return -1;
                        }
                }
        long curr = ftell (fp);
        


        for (i = 0; i < nVal; i++)
                {

                if (fread (&val, sizeof(val), 1, fp) != 1)
                        {
                        fprintf (stderr, "Error with fread for i = %d\n", i);
                        return -1;
                        }

                printf ("%g\n", (double) val);
                }

        return 0;
        }  // end of main
