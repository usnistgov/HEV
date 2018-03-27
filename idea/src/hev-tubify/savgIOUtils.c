
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "savgIOUtils.h"


#define MIN(a,b)   (((a)<(b))?(a):(b))
#define MAX(a,b)   (((a)>(b))?(a):(b))




int
savgIsKeyword (char *kw)
	{


	if (strncasecmp (kw, "poly", 4) == 0)
		{
		return SAVG_POLYPRIM_KEYWORD;
		}

	if (strncasecmp (kw, "line", 4) == 0)
		{
		return SAVG_LINEPRIM_KEYWORD;
		}

	if (strncasecmp (kw, "point", 5) == 0)
		{
		return SAVG_POINTPRIM_KEYWORD;
		}


	return SAVG_UNKNOWN_KEYWORD;

	}  // end of savgIsKeyword


static int
tokenize (
	char *line, 
	char token[SAVG_MAX_TOKENS][SAVG_MAX_TOKEN_LEN], 
	int *numTokens)
        {
        char *tok;
        char locLine[SAVG_MAX_LINE_LEN];

        strcpy (locLine, line);
        line = locLine;

        *numTokens = 0;

        while ((tok = strtok (line, " ,\t\n\r")) != NULL)
                {
                line = NULL;
                strcpy (token[*numTokens], tok);
                (*numTokens)++;
                if (*numTokens >= SAVG_MAX_TOKENS)
                        {
                        return -1;
                        }
                }

        return 0;
        }  // end of tokenize


int
savgGetLine (FILE *inFP, char line[SAVG_MAX_LINE_LEN], int *lineType)
	{

	char token[SAVG_MAX_TOKENS][SAVG_MAX_TOKEN_LEN];
	int numTokens, numNumericComponents;
	int i, numNumericTokens;
	double dummy;



	if (fgets (line, SAVG_MAX_LINE_LEN, inFP) != line)
		{
		*lineType = SAVG_EOF;
		return SAVG_EOF;
		}


	tokenize (line, token, &numTokens);

	if (numTokens == 0)
		{
		*lineType = SAVG_BLANK_LINE;
		return SAVG_BLANK_LINE;
		}

	if (token[0][0] == '#')
		{
		*lineType = SAVG_COMMENT_LINE;
		return SAVG_COMMENT_LINE;
		}

	if (savgIsKeyword (token[0]))
		{
		*lineType = SAVG_KEYWORD_LINE;
		return SAVG_KEYWORD_LINE;
		}


	numNumericTokens = 0;
	for (i = 0; i < numTokens; i++)
		{
		if (sscanf (token[i], "%lf", &dummy) != 1)
			{
			break;
			}
		numNumericTokens++;
		}


	if ( (numNumericTokens != 3) &&
	     (numNumericTokens != 6) &&
	     (numNumericTokens != 7) &&
             (numNumericTokens != 10) )
		{
		*lineType = SAVG_UNRECOGNIZED_LINE;
		return SAVG_UNRECOGNIZED_LINE;
		}

	*lineType = SAVG_VERTEX_LINE;
	return SAVG_VERTEX_LINE;

	}  // end of savgGetLine


int
savgGetVertexComponents (
	char *line,    // text of input line
	int *whichComponents,
	double xyz[3],
	double rgba[4],
	double norm[3] )
	{

	char token[SAVG_MAX_TOKENS][SAVG_MAX_TOKEN_LEN];
	int numTokens, numNumericComponents;
	int i, numNumericTokens, normStart;
	double numericTokens[SAVG_MAX_TOKENS];

	tokenize (line, token, &numTokens);

	numNumericTokens = 0;
	for (i = 0; i < MIN (numTokens, 10); i++)
		{
		if (sscanf (token[i], "%lf", &(numericTokens[i])) != 1)
			{
			break;
			}
		numNumericTokens++;
		}

	if ( (numNumericTokens != 3) &&
	     (numNumericTokens != 6) &&
	     (numNumericTokens != 7) &&
             (numNumericTokens != 10) )
		{
		return -1;
		}



	if (numNumericTokens >= 3)
		{
		*whichComponents = SAVG_VERTEX_XYZ;
		xyz[0] = numericTokens[0];
		xyz[1] = numericTokens[1];
		xyz[2] = numericTokens[2];
		}
	else
		{
		return -1;
		}
		

	if (numNumericTokens >= 7)
		{
		*whichComponents |= SAVG_VERTEX_RGBA;
		rgba[0] = numericTokens[3];
		rgba[1] = numericTokens[4];
		rgba[2] = numericTokens[5];
		rgba[3] = numericTokens[6];
		normStart = 7;
		}
	else
		{
		normStart = 3;
		}

	if (numNumericTokens > normStart)
		{
		*whichComponents |= SAVG_VERTEX_NORM;
		norm[0] = numericTokens[normStart+0];
		norm[1] = numericTokens[normStart+1];
		norm[2] = numericTokens[normStart+2];
		}

	return 0;
	}  // end of getVertexComponents



int
savgGetKeywordUpper (
	char *line,
	char keyword[SAVG_MAX_TOKEN_LEN] )
	{
	char *s;


	if (sscanf (line, "%s", keyword) != 1)
		{
		keyword[0] = 0;
		return -1;
		}

	for (s = keyword; *s; s++)
		{
		*s = toupper (*s);
		}

	return 0;
	} // end of savgGetKeyword
	



int 
savgGetKeywordType (char *line)
	{
	char keyword[SAVG_MAX_TOKEN_LEN];

	savgGetKeywordUpper (line, keyword);
	return savgIsKeyword (keyword);
	}  // end of savgGetKeywordType
