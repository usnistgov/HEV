
/*

This program will make a bunch of raw files that will be used for
testing hev-convertToMha.

All of the files will be 10 x 11 x 12 with the first dimension varying
fastest.

data will be constant along the 11 dimensions and will vary 
in the 12 dimension.  

along the 10 dimension, the volume will be split in half: data values
will be constant from 0:4 and constant 5:9.  In 0:4 the data will
ascend in value as ascending in the 12 dimension.  In the 5:9 range
data values will descend as it ascends in the 12 direction.

I'm only going to write out these data types for binary files: unsigned char, 
int, float.



*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


static int
writeBinFile (char *fn, int totalByteLen, void *ptr)
        {
        FILE *fp;

        fp = fopen (fn, "wb");
        if (fp == NULL)
                {
                fprintf (stderr, "Unable to open file %s\n", fn);
                return -1;
                }

        if (fwrite (ptr, totalByteLen, 1, fp) != 1)
                {
                fprintf (stderr, "Error writing file %s\n", fn);
                fclose (fp);
                return -1;
                }

        fclose (fp);
        return 0;
        } // end of writeFile


static int
writeAscIntFile (char *fn, int nInt, int *ptr)
        {
        FILE *fp;
        int i;

        fp = fopen (fn, "w");
        if (fp == NULL)
                {
                fprintf (stderr, "Unable to open file %s\n", fn);
                return -1;
                }

        for (i = 0; i < nInt; i++)
                {
                fprintf (fp, "%d ", ptr[i]);
                if ((i % 7) == 6)
                        {
                        fprintf (fp, "\n");
                        }
                }

        fclose (fp);
        return 0;
        } // end of writeFile







main ()
        {

        double dbl[12][11][10];
        float flt[12][11][10];
        int intgr[12][11][10];
        unsigned char uc[12][11][10];
        char fn[1000];
        FILE *fp;

        int i, j, k, n, nn;

        for (i = 0; i < 10; i++)
                {
                for (j = 0; j < 11; j++)
                        {
                        for (k = 0; k < 12; k++)
                                {
                                uc[k][j][i] = 
                                intgr[k][j][i] = 
                                flt[k][j][i] = 
                                dbl[k][j][i] = (i < 5) ? k : 11-k ;
                                }
                        }
                }


        writeBinFile ("bin_10_11_12_dbl.raw", 10*11*12*sizeof(double), dbl);
        writeBinFile ("bin_10_11_12_flt.raw", 10*11*12*sizeof(float), flt);
        writeBinFile ("bin_10_11_12_int.raw", 10*11*12*sizeof(int), intgr);
        writeBinFile ("bin_10_11_12_uch.raw", 10*11*12*sizeof(unsigned char), uc);
        writeAscIntFile ("asc_10_11_12_int.raw", 10*11*12, (int *)intgr);

        for (i = 0; i < 10; i++)
                {
                for (j = 0; j < 11; j++)
                        {
                        for (k = 0; k < 12; k++)
                                {
                                intgr[k][j][i] = 
                                flt[k][j][i] = 
                                dbl[k][j][i] -= 5;
                                }
                        }
                }

        writeBinFile ("bin_10_11_12_dbl_neg.raw", 10*11*12*sizeof(double), dbl);
        writeBinFile ("bin_10_11_12_flt_neg.raw", 10*11*12*sizeof(float), flt);
        writeBinFile ("bin_10_11_12_int_neg.raw", 10*11*12*sizeof(int), intgr);

        writeAscIntFile ("asc_10_11_12_int_neg.raw", 10*11*12, (int *)intgr);

        // Now we'll output multiple channels of the float data

        for (n = 2; n <= 5; n++)
                {
                sprintf (fn, "asc_10_11_12_flt_neg_n%d.raw", n);
                fp = fopen (fn, "w");
                if (fp == NULL)
                        {
                        fprintf (stderr, "Unable to open file %s\n", fn);
                        exit (-1);
                        }
                for (k = 0; k < 12; k++)
                    {
                    for (j = 0; j < 11; j++)
                        {
                        for (i = 0; i < 10; i++)
                            {
                            for (nn = 0; nn < n; nn++)
                                {
                                fprintf (fp, "%g\n", flt[k][j][i] * pow (10.0, nn));
                                }  // end of inner loop over number of components
                            }
                        }
                    }
                fclose (fp);
                }  // end of out loop over number of components

        exit (0);
        }  // end of main
