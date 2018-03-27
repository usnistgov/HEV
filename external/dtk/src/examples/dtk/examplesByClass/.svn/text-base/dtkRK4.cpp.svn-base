// Example code using class dtkRK4

// This outputs a sin wave.

#include <stdio.h>
#include <stdlib.h>
#include <dtk.h>
#include <dtk/dtkDiffEq.h>
#include <dtk/dtkRK4.h>


// This is the two first order differential equations
// to make a sine wave with period 2 Pi.

void difeq(double *xdot, const double *x, long double t)
{
  xdot[0] = x[1];
  xdot[1] = - x[0];
}

int main()
{
  // 2 equations, time step set to 0.1.

  dtkRK4<double,double> rk4(difeq, 2, 0.1);

  /* My state variables, initialized:
   * x[0] = position
   * x[1] = velocity
   *    t = time.
   */
  double x[2] = { 1.0, 0.0 };
  long double t = 0.0;

  fprintf(stderr,"Pipe stdout to a plotter:\n");

  while(t < ((long double) 9.4))
    printf("%.16Lg %.13g\n",t, x[0]),
      rk4.go(x, &t);

  printf("%.16Lg %.13g\n",t, x[0]);

  fprintf(stderr,"Pipe stdout to a plotter:\n");

  return 0;
}
