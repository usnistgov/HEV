/*
 * SphericalMean.h: Release 2.1, June 2004.
 *
 * Mathematics Subpackage (Splines)
 *
 * Author: Samuel R. Buss
 *
 * Software is "as-is" and carries no warranty.  It may be used without
 *   restriction, but if you modify it, please change the filenames to
 *   prevent confusion between different versions.  Please acknowledge
 *   all use of the software in any publications or products based on it.
 *
 * Bug reports: Sam Buss, sbuss@ucsd.edu.
 * Web page: http://math.ucsd.edu/~sbuss/ResearchWeb
 *
 */

//
// Spherical Averaging Classes and Functions
//
// A. SphericalInterpolator - SLERP packge
//
// B. ComputeMeanSphere
//		Algorithms A1 and A2 of Buss-Fillmore.
//		For VectorR3, VectorR4 and Quaternion.
//

#ifndef SPHERICALMEAN_H
#define SPHERICALMEAN_H

#include "../VrMath/LinearR3.h"
#include "../VrMath/LinearR4.h"
#include "../VrMath/MathMisc.h"
#include "../VrMath/Quaternion.h"

class SphericalInterpolator;		// Spherical linear interpolation of vectors

// *****************************************************
// SphericalInterpolator class                         *
//    - Does linear interpolation (slerp-ing)		   *
// * * * * * * * * * * * * * * * * * * * * * * * * * * *


class SphericalInterpolator {

private:
	VectorR3 startDir, endDir;	// Unit vectors (starting and ending)
	double startLen, endLen;	// Magnitudes of the vectors
	double rotRate;				// Angle between start and end vectors

public:
	SphericalInterpolator( const VectorR3& u, const VectorR3& v );

	VectorR3 InterValue ( double frac ) const;
};

// ****************************************************************
// Spherical Mean routines										  *
//																  *	
// Algorithms based on											  *
//    S. Buss and J. Fillmore, "Spherical Averages and            *
//      Applications to Spherical Splines and Interpolation."     *
//      ACM Transactions on Graphics 20 (2001) 95-126.            *
// ****************************************************************

// Weighted sum of vectors - Compute Euclidean averages
VectorR3 WeightedSum( long Num, const VectorR3 vv[], const double weights[] );
VectorR4 WeightedSum( long Num, const VectorR4 vv[], const double weights[] );

// Weighted average of vectors on the sphere.  
//		Sum of weights should equal one (but no checking is done)
//		Implements Algorithm A2 (quadratic convergence rate) of Buss-Fillmore
VectorR3 ComputeMeanSphere( long Num, const VectorR3 vv[], const double weights[],
						  double tolerance = 1.0e-15, double bkuptolerance = 1.0e-13 );
VectorR3 ComputeMeanSphere( long Num, const VectorR3 vv[], const double weights[],
						  const VectorR3& InitialVec,
						  double tolerance = 1.0e-15, double bkuptolerance = 1.0e-13 );
VectorR4 ComputeMeanSphere( long Num, const VectorR4 vv[], const double weights[],
						  double tolerance = 1.0e-15, double bkuptolerance = 1.0e-13 );
Quaternion ComputeMeanQuat( long Num, const Quaternion qq[], const double weights[],
						   double tolerance = 1.0e-15, double bkuptolerance = 1.0e-13 );

// Next function mostly for internal use.
//		It takes an initial estimate InitialVec (and a flag for
//		indicating quaternions).
const int SPHERICAL_NOTQUAT=0;
const int SPHERICAL_QUAT=1;
VectorR4 ComputeMeanSphere( long Num, const VectorR4 vv[], const double weights[],
						   const VectorR4& InitialVec, int QuatFlag=0,
						   double tolerance = 1.0e-15, double bkuptolerance = 1.0e-13 );

// Slower version, uses Steepest Descent method
// In the Buss-Fillmore paper, this is Algorithm A1.
VectorR3 ComputeMeanSphereSlow( long Num, const VectorR3 vv[], const double weights[],
							double tolerance = 1.0e-16, double bkuptolerance = 5.0e-16 );
VectorR4 ComputeMeanSphereSlow( long Num, const VectorR4 vv[], const double weights[],
							double tolerance = 1.0e-16, double bkuptolerance = 5.0e-16 );




#endif   // SPHERICALMEAN_H

