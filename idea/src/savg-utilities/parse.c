#include <string.h>
#include <stdio.h>       
#include <stdlib.h>       
      
/*     
  pass a line and break it into argc and **argc sort of like a shell does   
  stuff within "" or '' is treated as one parameter. the \ character will   
  escape the next character.  A # throws away the rest of the line.    
    
  DOES NOT DO ERROR CHECKING FOR TERMINATING QUOTES- AN EOL OR COMMENT   
  TERMINATES THE QUOTE.   
*/   
   
static void dpf_charsToArgcArgv_addChar(char c, int *pargc, char ***pargv, int *inSpace)
{ 
  int i;
  int len;
  char *hold = NULL;
 
  if (*inSpace) 
  { 
    // declare *hold
    char *hold = NULL;
  
    // increment number of parameters   
    (*pargc)++ ;    
   
    // add one more array element   
    *pargv = (char**) realloc(*pargv, (*pargc)*(sizeof(char *))) ;   
   
    // create a null string   
    hold = (char *) malloc(sizeof(char)) ;   
    hold[0] = '\0' ;   
    (*pargv)[(*pargc)-1] = hold ;   
    *inSpace = 0 ;   
  }   
  
  i = (*pargc)-1 ;    
   
  // one for new character, one for terminator   
  len = strlen((*pargv)[i])+2 ;   
  hold = (char *) malloc(len*sizeof(char)) ;   
  strcpy(hold, (*pargv)[i]) ;   
  hold[len-2] = c ;   
  hold[len-1] = '\0' ;   
  free((*pargv)[i]) ;   
  (*pargv)[i] = hold ;   
   
}   
   
void dpf_charsToArgcArgv(char *line, int *pargc, char ***pargv) 
{   
  int inSpace = 1 ;   
  int inDoubleQuote = 0 ;   
  int inSingleQuote = 0 ;   
  int prevEscape = 0 ;   
  int i;

  // reset each line
  // free up memory, set argc to zero, set argv to NULL
  *pargc = 0;
  *pargv = NULL;
   
  for (i=0; i<strlen(line); i++)
  {   
    char c = line[i] ;   
    if (c == '\n')   
      continue;   
    if  (c == '\\') 
    {   
      prevEscape = 1 ;   
    }
    else 
    {   
      if (prevEscape) 
      {   
	// if previous character was an escape, just add to arg   
	dpf_charsToArgcArgv_addChar(c, pargc, pargv, &inSpace) ;   
      } 
      else if (c == '"') 
      {   
	if (inSingleQuote) 
        {   
	  dpf_charsToArgcArgv_addChar(c, pargc, pargv, &inSpace) ;   
	} 
        else    
	  if (inDoubleQuote) 
          {   
	    inDoubleQuote = 0 ;   
	  } 
          else 
          {   
	    inDoubleQuote = 1 ;   
	  }   
      } 
      else if  (c == '\'') 
      {   
	if (inDoubleQuote)   
	  dpf_charsToArgcArgv_addChar(c, pargc, pargv, &inSpace) ;   
	else    
	  if (inSingleQuote) 
          {   
	    inSingleQuote = 0 ;   
	  } 
          else 
          {   
	    inSingleQuote = 1 ;   
	  }   
   
      } 
      else if  (c == ' ') 
      {   
	if (prevEscape || inSingleQuote || inDoubleQuote) 
        {   
	  dpf_charsToArgcArgv_addChar(c, pargc, pargv, &inSpace) ;   
	} 
        else if (!inSpace) 
        {   
	  inSpace = 1 ;   
	}   
   
      } 
      else if (c == '#') 
      {   
	return ;   
   
      } 
      else 
      {   
	dpf_charsToArgcArgv_addChar(c, pargc, pargv, &inSpace) ;   
      }   
      prevEscape = 0 ;   
    }   
  }   
}  
