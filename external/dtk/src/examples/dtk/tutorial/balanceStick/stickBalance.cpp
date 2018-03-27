// This example is the dynamics for a 1-D pendulum whos pivot point
// can move from side to side. It reads input and writes output from
// and to shared memory.  It's the simplest non-trival dynamics model
// that I could think of, which has realtime human input and output or
// operator-in-the-loop and can be displayed as a rigid body.

// Don't edit this code without checking
// docs_src/tutor_realtime.html.in which refers to this code.

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <dtk.h>
#include <dtk/dtkDiffEq.h>
#include <dtk/dtkRK4.h>

// main loop run state variable
int running = 1;

// Though units made be scaled away we include them in comments in
// order to help get a feel for something that's real.

float pivot_acceleration = 0.0f; // meters / seconds^2
const float damping_rate = 0.1f; // 1 / seconds
const float g = 9.8f; // meters / second^2
const float length = 6.0f; // meters

// It can be shown that the equations of motion are the two first
// order differential equations. Reference: Barger & Olsson, Classical
// Mechanics, A Modern Perspective, McGraw-Hill Co. There are lots
// more books on classical mechanics.
void difeq(float *xdot, const float *x, long double t)
{
  // x[0] = angle
  xdot[0] = x[1];

  // x[1] = angle rate
  xdot[1] = - pivot_acceleration*cosf(x[0])/length +
    g*sinf(x[0])/length - damping_rate*x[1];
}

void sig_catcher(int sig)
{
  printf("stickBalance caught signal %d, exiting ...\n", sig);
  running = 0;
}


int main(void)
{
  // Get a shared memory object for the, "translate", pivot_position
  // input.
  dtkSharedMem positionShm(sizeof(float), "translate");
  if(positionShm.isInvalid()) return 1; // error

  // pivot_position[] is used to calculate pivot_acceleration using a
  // straight forward approximation using the last three tip position
  // values read.
  float pivot_position[3] = { 0.0f, 0.0f, 0.0f };
  // reset the value of the pivot_position
  if(positionShm.write(pivot_position)) return 1; // error

  // The stick as a rigid body has 6 degrees of freedom, x, y, z,
  // heading, pitch, and roll.  In this case we just be changing x
  // (pivot_position) and roll. The stick is at 0,0,0,0,0,0 if it is
  // pointing straight up.
  float x[6] = { 0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f };  

  // Get a shared memory object for the, "stick", x, y, z, heading,
  // pitch, roll, or rigid body position output, rotations are in
  // degrees.
  dtkSharedMem stickShm(6*sizeof(float), "stick" , x);
  if(stickShm.isInvalid()) return 1; // error

  // gettimeofday() wrapper to measure "real" time as a long double
  dtkTime time;
  long double t = time.get();

  // angle of the stick (pendulum) measured from the vertical or the
  // roll of the stick or rotation about the y axis in radians.
  float roll[2] = { /*roll*/ x[5]*M_PI/180,  /*roll rate*/ 0.0f };

  // Get an dtkRK4 differential equations solver object.  dynamical
  // state is a float, time step is a double, total time is always a
  // long double, difeq() has 2 differential equations, time step 0.01
  // seconds, starting at time t.
  dtkRK4<float,double> rk4(difeq, 2, 0.01, t);

  // caught signals SIGINT (Cntl-c) and SIGTERM (default kill signal)
  // so that this can exit gracefully if that happens.
  signal(SIGINT, sig_catcher);
  signal(SIGTERM, sig_catcher);

  // system interval timer wrapper
  dtkRealtime_init(0.01);

  u_int64_t count=0;

  while(running)
    {
      // get input
      if(positionShm.read(&(pivot_position[count%3]))) return 1; // error

      // calculate acceleration from the last 3 positions. There are
      // better ways to do this, but this is the simplest I've found.
      if(count > 2)
	pivot_acceleration =
	  (pivot_position[count%3]-
	   2*pivot_position[(count-1)%2]+
	   pivot_position[(count-2)%2])/(0.01*0.01);

      // Get the current time.
      t = time.get();

      // advance the dynamics to the current time
      rk4.go(roll, t);

      // Set 2 of the 6 output rigid body position variables.
      x[0] = pivot_position[count%3]; // x

      if(roll[0] > M_PI)
	roll[0] -= M_PI*2;
      else if(roll[0] < - M_PI)
	roll[0] += M_PI*2;
      x[5] = roll[0]*180/M_PI; // roll

      // Write the output.
      if(stickShm.write(x)) return 1; // error

      count++;
    
      // Wait until the next cycle in the reminder of the 0.01 seconds
      dtkRealtime_wait();
    }

  dtkRealtime_close();
}
