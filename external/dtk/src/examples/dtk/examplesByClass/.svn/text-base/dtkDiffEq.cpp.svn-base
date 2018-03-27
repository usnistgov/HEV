// Example code using class dtkDiffEq with class dtkRK4

// This outputs a sin wave.

#include <stdio.h>
#include <stdlib.h>
#include <dtk.h>
#include <dtk/dtkDiffEq.h>
#include <dtk/dtkRK4.h>

// This is the two first order differential equations to make a sine
// wave with period 2 Pi with a little damping.

// SHO - simple harmonic oscillator
class SHO : public dtkDiffEq<float>
{
public:
  SHO(void);
  void getDerivatives(float *xdot, const float *x, long double t) const;
};

SHO::SHO(void) { numberOfEquations = 2;}

void SHO::
getDerivatives(float *xdot, const float *x, long double t) const
{
  xdot[0] = x[1];
  xdot[1] = -x[0] - 0.1 * x[1];
}

int main()
{
  // 2 equations, time step set to 0.1.

  SHO sho;
  dtkRK4<float,double> rk4((dtkDiffEq<float> *) &sho, 0.1);

  /* My state variables, initialized:
   * x[0] = position
   * x[1] = velocity
   * t = time.
   */
  float x[2] = { 1.0, 0.0 };
  long double t = 0.0;

  fprintf(stderr,"Pipe stdout to a plotter:\n");

  while(t < ((long double) 17.4))
    printf("%.16Lg %.13g\n",t, x[0]),
      rk4.go(x, &t);

  printf("%.16Lg %.13g\n",t, x[0]);

  fprintf(stderr,"Pipe stdout to a plotter:\n");

  return 0;
}
