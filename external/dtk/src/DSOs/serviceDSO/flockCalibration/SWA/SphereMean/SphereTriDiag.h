/*
 * SpherTriDiag.h: Release 2.1, June 2004.
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
// Spherical Tridiagonal Equation Solver.
//
//   Intended as subroutine for defining interpolating
//	 B-Splines.  (Main routines not currently available.)
//
// Algorithms based on											 
//    S. Buss and J. Fillmore, "Spherical Averages and           
//      Applications to Spherical Splines and Interpolation."    
//      ACM Transactions on Graphics 20 (2001) 95-126.
//
// Warning: code not particularly polished.  Was written only
//   for my own use.

#ifndef SPHERE_TRIDIAG_H
#define SPHERE_TRIDIAG_H

class VectorR3;
class VectorR4;
class Quaternion;			// Quaternion (x,y,z,w) values.


// Solves a system of spherical-mean equalities, where the system is
// given as a tridiagonal matrix.
// Uses Algorithm S2 of Buss-Fillmore
// See comments in SphereTriDiag.cpp for usage.

void SolveTriDiagSphere ( int numPoints, 
						   const double* tridiagvalues, const VectorR3* c,
						   VectorR3* p, 
						   double accuracy=1.0e-15, double bkupaccuracy=1.0e-13 );
void SolveTriDiagSphere ( int numPoints, 
						   const double* tridiagvalues, const VectorR4* c,
						   VectorR4* p, 
						   double accuracy=1.0e-15, double bkupaccuracy=1.0e-13 );

//  The "Slow" version uses a simpler but slower iteration with a linear rate of
//		convergence.  The base version uses a Newton iteration with a quadratic
//		rate of convergence.
//  This is Algorithm S1 of Buss-Fillmore
void SolveTriDiagSphereSlow ( int numPoints, 
						   const double* tridiagvalues, const VectorR3* c,
						   VectorR3* p, 
						   double accuracy=1.0e-15, double bkupaccuracy=5.0e-15 );
void SolveTriDiagSphereSlow ( int numPoints, 
						   const double* tridiagvalues, const VectorR4* c,
						   VectorR4* p, 
						   double accuracy=1.0e-15, double bkupaccuracy=5.0e-15 );


#endif // SPHERICAL_H
