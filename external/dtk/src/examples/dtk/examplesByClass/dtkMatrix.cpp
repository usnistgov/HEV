// This uses the dtkMatrix class

#include <stdio.h>
#include <dtk.h>
 
int main(void)
{
  dtkMatrix m;

  //m.scale(4.2f);
  m.translate(100.0f, -91.0f, 130.0f);
  m.rotateHPR(2.0f, 80.0f, 1.000f);
  m.rotateHPR(0.0f, 0.0f, 0.0f);
  printf("m=\n");
  m.print();
 
  dtkMatrix i;
  i.copy(&m);
  i.invert();
 
  printf("i=\n");
  i.print();
 
  m.mult(&i);
  printf("i*m=\n");
  m.print();
 
  return 0;
}
