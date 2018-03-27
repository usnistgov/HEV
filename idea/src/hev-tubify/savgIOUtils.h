
#ifndef _SAVG_IO_UTILS_H_
#define _SAVG_IO_UTILS_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// maximum number of tokens, token length, line length
#define SAVG_MAX_TOKENS      (50)
#define SAVG_MAX_TOKEN_LEN   (256)
#define SAVG_MAX_LINE_LEN   (51*257)


// savg line types
#define SAVG_EOF                 (-2)
#define SAVG_UNRECOGNIZED_LINE   (-1)
#define SAVG_BLANK_LINE          (0)
#define SAVG_COMMENT_LINE        (1)
#define SAVG_KEYWORD_LINE        (2)
#define SAVG_VERTEX_LINE         (3)


// vertex components
#define SAVG_VERTEX_XYZ       (0x1)
#define SAVG_VERTEX_RGBA      (0x2)
#define SAVG_VERTEX_NORM      (0x4)


// keyword types
#define SAVG_UNKNOWN_KEYWORD   (0)
#define SAVG_POLYPRIM_KEYWORD  (1)
#define SAVG_LINEPRIM_KEYWORD  (2)
#define SAVG_POINTPRIM_KEYWORD (3)

// returns keyword type
int 
savgIsKeyword (char *kw);


// returns next line and its type
int
savgGetLine (FILE *inFP, char line[SAVG_MAX_LINE_LEN], int *lineType);


// Parses the tokens in a vertex line; tells which parts of the
// vertex are present (xyz, rgba, or norm) and returns them as doubles
int
savgGetVertexComponents (
	char *line,    // text of input line
	int *whichComponents,
	double xyz[3],
	double rgba[4],
	double norm[3] );




// Gets the keyword from the line and returns it in upper case
int
savgGetKeywordUpper (
	char *line,
	char keyword[SAVG_MAX_TOKEN_LEN] );



// Looks at first token in line and returns the keyword type
// of that token.
int
savgGetKeywordType (char *line);

#endif   /* _SAVG_IO_UTILS_H_ */
