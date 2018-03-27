// This uses the dtkVec3 class

#include <stdio.h>
#include <dtk.h>
 
int main(void)
{
  dtkVec3 v1;
  printf("v1: default: ") ;
  v1.print() ;

  dtkVec3 v2(1.f, 2.f, 3.f) ;
  printf("v2: set by 3 floats: ") ;
  v2.print() ;

  float d[3] = {4.f, 5.f, 6.f} ;
  dtkVec3 v3(d) ;
  printf("v3: set by an array of 3 floats: ") ;
  v3.print() ;

  v1 = v2 ;
  printf("\nv1 = v2, printing v1:") ;
  v1.print() ;

  printf("\nprint elements using d[]\n") ;
  printf("v1.d[0], v1.d[1], v1.d[2] = %f, %f, %f\n",
	 v1.d[0], v1.d[1], v1.d[2]) ;

  printf("print elements using xyz\n") ;
  printf("v1.x, v1.y, v1.z =          %f, %f, %f\n",
	 v1.x, v1.y, v1.z) ;

  printf("print elements using hpr\n") ;
  printf("v1.h, v1.p, v1.r =          %f, %f, %f\n",
	 v1.h, v1.p, v1.r) ;

  float a, b, c = {0.f} ;
  v1.get(&a, &b, &c) ;
  printf("\nv1.get(&a, &b, &c); a, b, c = %f, %f, %f\n", a, b, c) ;

  a = b = c = 0.f ;
  v3.set(a, b, c) ;
  printf("\na = b = c = 0.f ; v3.set(a, b, c); v3:") ;
  v3.print() ;

  int foo = v1.equals(&v2) ;
  printf("\n%d =  v1.equals(v2) ;\n",foo) ;

  foo = v1.equals(&v3) ;
  printf("\n%d =  v1.equals(v3) ;\n",foo) ;

  return 0;
}
