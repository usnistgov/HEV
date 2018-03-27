#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <ctype.h>

char str[] = "Hello world";
char *saveptr;
char *foo, *bar, *inputstring;

char *trimLeadingWhiteSpace(char *inString, int length)
{
  int l;
  l=length;
  while (l-- > 0) {
    printf("inString = '%s'\n", inString);
    if (isspace(*inString) == 0) break;
    inString++;
  }
  return(inString);
}

void trimTrailingWhiteSpace(char *inString, int length)
{
  char *p;
  int l;
  l=length;
  p=inString+length-3;
  while (l-- > 0) {
    printf("Trim Trail inString = '%s'\n", inString);
    printf("p=%d\n",*p);
    if (isspace(*p) == 0) break;
    p--;
  }
  p++;
  *p=0;
}

main(int argc, char *argv[]) {
  char tmp[1000];
  int l;
  
  strncpy(tmp, argv[1], 1000);
  printf("tmp = '%s'\n", tmp);
  fflush(stdout);
  
  foo = strtok_r(tmp, " ", &saveptr);
  bar = strtok_r(NULL, " ", &saveptr);
  printf("foo = '%s'\n",foo);
  printf("bar = '%s'\n",bar);
  printf("remaining = '%s'\n", saveptr);

  printf("\n");
  fflush(stdout);
  
  // ----------------------
  strncpy(tmp, argv[1], 1000);
  printf("tmp = '%s'\n", tmp);
  inputstring=tmp;
  
  foo = strsep(&inputstring, " \t");
  bar = strsep(&inputstring, " \t");

  printf("foo = '%s'\n",foo);
  printf("bar = '%s'\n",bar);
  printf("remaining = '%s'\n", inputstring);

  printf("\n");
  fflush(stdout);

  // ----------------------
  strncpy(tmp, argv[1], 1000);
  printf("tmp = '%s'\n", tmp);
  inputstring=tmp;

  l=strnlen(inputstring, 1000);
  inputstring=trimLeadingWhiteSpace(inputstring, l);

  printf("inputstring before trim trailing= '%s'\n", inputstring);
  trimTrailingWhiteSpace(inputstring, l);
  printf("inputstring after trim trailing= '%s'\n", inputstring);
  foo = strsep(&inputstring, " \t");

  inputstring=trimLeadingWhiteSpace(inputstring, l);
  bar = strsep(&inputstring, " \t");
  inputstring=trimLeadingWhiteSpace(inputstring, l);

  printf("foo = '%s'\n",foo);
  printf("bar = '%s'\n",bar);
  printf("remaining = '%s'\n", inputstring);


  
}
