//
// Testing program for looking at shared memory file produced
// by hev-segmentSelection
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <dtk.h>


#define PTSHM_MAX_NUM_SEGS          (200)
#define PTSHM_NSEG_POS              (0)
#define PTSHM_EDIT_SEG_POS          (PTSHM_NSEG+sizeof(int))
#define PTSHM_LAST_SEG_CHANGED_POS  (PTSHM_EDIT_SEG+sizeof(int))
#define PTSHM_CHANGE_COUNTER_POS    (PTSHM_LAST_SEG_CHANGED+sizeof(int))
#define PTSHM_HDR_SIZE              (8*sizeof(int))

#define PTSHM_SEG_DATA_START        (PTSHM_HDR_SIZE)
#define PTSHM_SEG_LENGTH            (sizeof(long int)+(6*sizeof(float)))
#define PTSHM_TOTAL_LENGTH          (PTSHM_HDR_SIZE+(PTSHM_MAX_NUM_SEGS*PTSHM_SEG_LENGTH)) 

#define PTSHM_SEG_POS(n)            (PTSHM_SEG_DATA_START+(n*PTSHM_SEG_LENGTH))

dtkSharedMem *SegmentShm = NULL;

static int 
setupSegmentShm (char *ptsShmName)
	{

        SegmentShm = new dtkSharedMem (PTSHM_TOTAL_LENGTH, ptsShmName);
        if (SegmentShm->isInvalid())
                {
                fprintf (stderr, "%s: Unable to open shared memory %s.\n",
                                ptsShmName);
                return -1;
                }

	return 0;
	}  // end of setupSegmentShm


static int
printHdr ()
	{
	int hdr[4];

	if (SegmentShm->read (hdr, 4*sizeof(int), 0))
		{
		fprintf (stderr, "Error reading header.\n");
		return -1;
		}

	printf (
	  "nSeg: %d   editSeg: %d   last editSeg: %d   counter: %d\n", 
		hdr[0], hdr[1], hdr[2], hdr[3]);

	return 0;
	}  // end of printHdr


static void
printPts ()
	{

	int totalValid = 0;

	for (int i = 0; i < 200; i++)
		{
		long int valid;
		if (SegmentShm->read (&valid, sizeof (long int),
			PTSHM_SEG_POS (i)))
			{
			fprintf (stderr, 
			   "Error reading valid flag for seg %d.\n", i);
			}

		else if (valid)
			{
			float seg[2][3];
			totalValid++;
			if (SegmentShm->read (seg, 6*sizeof(float),
				PTSHM_SEG_POS (i) + sizeof(long int)))
				{
				fprintf (stderr, 
			   	"Error reading coords for seg %d.\n", i);
				}
			else
				{
				printf ("   Seg %d: (%f %f %f)  (%f %f %f)\n",
					i, 
					seg[0][0], seg[0][1], seg[0][2], 
					seg[1][0], seg[1][1], seg[1][2]);
				}
			}
		

		}
	printf ("   Total number of valid segments = %d\n", totalValid);
	}


main (int argc, char **argv)
	{

	if (setupSegmentShm (argv[1]))
		{
		exit (-1);
		}

	while (1)
		{
		printHdr ();
		printPts ();
		usleep (1000000);
		}

	}  // end of main
