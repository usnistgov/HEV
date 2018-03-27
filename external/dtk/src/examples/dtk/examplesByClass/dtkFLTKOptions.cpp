#include <stdio.h>
#include <dtk.h>

int main(int argc, char **argv)
{

  if(argc < 2)
    {
      dtkFLTKOptions_print();
      return 1;
    }

  int fltk_argc;
  char **fltk_args;
  int other_argc;
  char **other_args;
  
  if(dtkFLTKOptions_get(argc, (const char **) argv,
			&fltk_argc, &fltk_args,
			&other_argc, &other_args))
    return 1; // error
  int i;
  printf("Your FLTK arguments were:");
  for(i=0;i<fltk_argc;i++)
    printf(" %s", fltk_args[i]);
  printf("\n");

  printf("Your OTHER arguments were:");
  for(i=0;i<other_argc;i++)
    printf(" %s", other_args[i]);
  printf("\n");
  
  return 0;
}
