// This uses the dtkCoord class

#include <stdio.h>
#include <dtk.h>
 
int main(void)
{
  dtkCoord c1;
  printf("c1: default: ") ;
  c1.print() ;

  dtkCoord c2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f) ;
  printf("c2: set by 6 floats: ") ;
  c2.print() ;

  float d[6] = {4.f, 5.f, 6.f, 7.f, 8.f, 9.f} ;
  dtkCoord c3(d) ;
  printf("c3: set by an array of 6 floats: ") ;
  c3.print() ;

  dtkVec3 v1(-1.f, -2.f, -3.f) ;
  dtkVec3 v2(-4.f, -5.f, -6.f) ;
  dtkCoord c4(&v1, &v2) ;
  printf("c4: set by 2 dtkCoords: ") ;
  v1.print() ;
  v2.print() ;
  c4.print() ;

  c1 = c2 ;
  printf("\nc1 = c2, printing c1:") ;
  c1.print() ;

  printf("\nprint elements using d[]\n") ;
  printf("c1.d[0], c1.d[1], c1.d[2], c1.d[3], c1.d[4], c1.d[5] =\n "
	 "%f, %f, %f, %f, %f, %f\n",
	 c1.d[0], c1.d[1], c1.d[2], c1.d[3], c1.d[4], c1.d[5]) ;

  printf("print elements using xyzhpr\n") ;
  printf("c1.x, c1.y, c1.z, c1.h, c1.p, c1.r =\n %f, %f, %f, %f, %f, %f\n",
	 c1.x, c1.y, c1.z, c1.h, c1.p, c1.r) ;

  float x, y, z, h, p, r = {0.f} ;
  c1.get(&x, &y, &z, &h, &p, &r) ;
  printf("\nc1.get(&x, &y, &z, &h, &p, &r); x, y, z, h, p, r = \n"
	 "%f, %f, %f, %f, %f, %f\n", x, y, z, h, p, r) ;

  x = y = z = h = p = r = 0.f ;
  c3.set(x, y, z, h, p, r) ;
  printf("\nx = y = z = h = p = r = 0.f ; c3.set(x, y, z, h, p, r); c3:") ;
  c3.print() ;

  int foo = c1.equals(&c2) ;
  printf("\n%d =  c1.equals(c2) ;\n",foo) ;

  foo = c1.equals(&c3) ;
  printf("\n%d =  c1.equals(c3) ;\n",foo) ;


  return 0;
}
