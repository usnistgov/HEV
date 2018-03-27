
#ifndef  _HEV_SEGMENT_SELECTION_H_
#define _HEV_SEGMENT_SELECTION_H_



extern char ProgName[1000];
extern char TmpDirName[1000];

extern bool Verbose;
extern bool Debug;

// typedef double Segment[2][3];


#define EXIT(n)  \
	{\
	if (Debug)\
		{\
		fprintf (stderr, "\n\n\n%s: Exiting at %s:%d\n\n\n", \
			ProgName, __FILE__, __LINE__); \
		}\
	exit (n);\
	}




#endif // _HEV_SEGMENT_SELECTION_H_
