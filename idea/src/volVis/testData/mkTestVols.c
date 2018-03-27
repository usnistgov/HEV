
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void
zeroVol (float v[19][20][21])
    {
    int i;
    float *f;
    f = &(v[0][0][0]);

    for (i = 0; i < 19*20*21; i++)
        {
        *f = 0;
        f++;
        }
    }  // zeroVol


static void
addNoBdry (float center[3], float v[19][20][21])
    {

    int i, j, k, n;
    float d;

    for (i = 0; i < 19; i++)
        {
        for (j = 0; j < 20; j++)
            {
            for (k = 0; k < 21; k++)
                {
                d = 0;
                d += (center[0]-i) * (center[0]-i);
                d += (center[1]-j) * (center[1]-j);
                d += (center[2]-k) * (center[2]-k);
                d = 1 / d;
                v[i][j][k] += d;
                }
            }
        }
    }  // end of addNoBdry

    
static void
addBdry (float center[3], float v[19][20][21])
    {

printf ("addBdry center = %f %f %f\n", center[0], center[1], center[2]);
    int i, j, k, n;
    float d;

    for (i = 0; i < 19; i++)
        {
        for (j = 0; j < 20; j++)
            {
            for (k = 0; k < 21; k++)
                {
                d = 0;
                d += (center[0]-i) * (center[0]-i);
                d += (center[1]-j) * (center[1]-j);
                d += (center[2]-k) * (center[2]-k);
                if (d < 25)
                    {
                    v[i][j][k] += 3;
printf ("setting pos %d %d %d to %f\n", i, j, k, v[i][j][k]);
                    }
                }
            }
        }
    }  // end of addBdry

    





static void
writeVol (char *nm, int t, float v[19][20][21])
    {
    char fn[100];
    FILE *fp;
    sprintf (fn, "%s.%03d.raw", nm, t);

    fp = fopen (fn, "w");
    if (fp == NULL)
        {
        fprintf (stderr, "Error opening %s.\n", fn);
        exit (-1);
        }

    fwrite (v, sizeof(float), 19*20*21, fp);

    fclose (fp);

    }  // end of writeVol







main ()
    {
    float noBdry[19][20][21];
    float bdry[19][20][21];

    float centers[2][3] = { {10.0, 15.0, 7.3}, {18.0, 5.0, 17.1} };
    float increments[2][3] = { {1.2, 0.3, -0.22}, {-.88, 0.4, -1.1} };


    int t, i;

    for (t = 0; t < 20; t++)
        {

printf ("\n\nStep t = %d\n", t);

        zeroVol (noBdry);
        zeroVol (bdry);

        addNoBdry (centers[0], noBdry);
        addNoBdry (centers[1], noBdry);
        addBdry (centers[0], bdry);
        addBdry (centers[1], bdry);


        writeVol ("noBdry", t, noBdry);
        writeVol ("bdry", t, bdry);

        for (i = 0; i < 3; i++)
            {
            centers[0][i] += increments[0][i];
            centers[1][i] += increments[1][i];
            }


        }

    exit (0);
    }
