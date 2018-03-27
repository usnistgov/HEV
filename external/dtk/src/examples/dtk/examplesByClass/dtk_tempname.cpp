// Example code using the function dtk_tempname()

#include <stdio.h>
#include <stdlib.h>
#include <dtk.h>


int main(void)
{
  char *tmp_file = dtk_tempname("/tmp/foo");
  printf("dtk_tempname(\"/tmp/foo\") = %s\n", tmp_file);
  // free the memory that was allocated using strdup().
  free(tmp_file);

  tmp_file = dtk_tempname("/tmp/foo");
  printf("dtk_tempname(\"/tmp/foo\") = %s\n", tmp_file);
  free(tmp_file);

  return 0;
}
