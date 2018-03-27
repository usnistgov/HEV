/*
 * SphericalMean.cpp: Release 2.1, June 2004.
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
// B. SphereMean
//		For VectorR3, VectorR4 and Quaternion
//


#include "SphereMean.h"
#include "../VrMath/LinearR2.h"

// ******************************************************************
// * SphericalInterpolation class - math library functions			*
// *	SLERP: Spherical Linear Interpolator						*
// *    Includes magnitude interpolation							*
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **
	
SphericalInterpolator::SphericalInterpolator( 
									const VectorR3& u, const VectorR3& v )
: rotRate(0.0) 
{
	startLen = u.Norm();
	endLen = v.Norm();
	if ( startLen==0.0 ) {
		startDir.SetZero();
		if (endLen==0.0) {
			endDir.SetZero();
		}
		else {
			endDir = v/endLen;
		}
	}
	else {
		startDir = u/startLen;
		if ( endLen==0.0 ) {
			endDir.SetZero();
		}
		else {
			endDir = v/endLen;
			VectorR3 w = ProjectPerpUnit( endDir, startDir );
			double wNorm = w.Norm();
			rotRate = atan2 ( wNorm , startDir^endDir );
			if (rotRate==0.0) {
				endDir = VectorR3(0.0,0.0,0.0);  // u and v colinear.
			}
			else {
				w /= wNorm;
				endDir = w;
			}
		}
	}
}

VectorR3 SphericalInterpolator::InterValue ( double frac ) const
{
	return ( startLen*startDir*cos(rotRate*frac)
				+ endLen*endDir*sin(rotRate*frac) );
}


// ****************************************************************
// Spherical Mean routines										  *
//																  *	
// Algorithms based on											  *
//    S. Buss and J. Fillmore, "Spherical Averages and            *
//      Applications to Spherical Splines and Interpolation."     *
//      ACM Transactions on Graphics 20 (2001) 95-126.            *
// ****************************************************************

//
// ComputeMeanSphere ( long Num, const VectorR3[] ) takes an array of N unit vectors.
//	It returns the linear interpolation of these N vectors.
//	The vectors should all lie in a hemisphere otherwise odd results or even
//  floating point exceptions may occur.
//
VectorR3 ComputeMeanSphere( long Num, const VectorR3 vv[], const double weights[],
							double tolerance, double bkuptolerance ) 
{
	VectorR3 xVec;

	//  Step 1: get an initial estimate for the mean.
	xVec = WeightedSum ( Num, vv, weights );
	xVec.Normalize();	// Initial estimate for the mean.

	// Step 2:
	return ComputeMeanSphere( Num, vv, weights, xVec, tolerance, bkuptolerance );
}

VectorR3 ComputeMeanSphere( long Num, const VectorR3 vv[], const double weights[],
						    const VectorR3& InitialVec,
							double tolerance, double bkuptolerance ) 
{
	// InitialVec is the initial estimate for the value of the weighted average.

	// Loop, doing Newton-style iterations to improve the estimate.
	
	VectorR3 xVec(InitialVec);
	const VectorR3 *vvPtr = vv;
	VectorR3 Clocalx, Clocaly;		// Local basis at xVec.
	VectorR3 vPerp;
	double costheta, sintheta, theta;
	double sinthetaInv;
	LinearMapR2 Hlocal;	
	LinearMapR2 Hessian;
	VectorR2 gradient;
	VectorR2 gradlocal;
	const double* wPtr;
	const VectorR3* vi;
	double cosphi, sinphi;
	double cosphiSq, sinphiSq;
	double tt, offdiag;
	VectorR2 xDisplocal;
	VectorR3 xDisp;
	
	while (1) {

		// Get local basis for estimate c = xVec.:
		GetOrtho( xVec, Clocalx, Clocaly );

		// For each vv vector, compute the tangent vector from xVec 
		//		towards the vv vector -- its length is the spherical length 
		//		from xVec to the vv vector.  
		//	Then compute its contribution to the Hessian
		long i;
		gradient.SetZero();
		Hessian.SetZero();
		vi = vv;						// Loop invariant: vi points to vv[i]
		wPtr = weights;					// Loop invariant: wPtr points to weights[i]
		for ( i=0; i<Num; i++ ) {
			vPerp = ProjectPerpUnitDiff ( *vi, xVec );
			sintheta = vPerp.Norm();
			if ( sintheta==0.0 ) {
				Hessian += LinearMapR2( *wPtr, 0.0, 0.0, *wPtr );
			}
			else {
				costheta = (*vi)^xVec;
				theta = atan2( sintheta, costheta );	// Angle from xVec to (*vi)
				sinthetaInv = 1.0/sintheta;
				vPerp *= sinthetaInv;						// Normalize
				cosphi = vPerp^Clocalx;
				sinphi = vPerp^Clocaly;
				gradlocal.Set(cosphi, sinphi);
				gradient += ((*wPtr)*theta)*gradlocal;	// Added on weighted discrepancy to gradient
				//Hlocal.Set( cosphi, sinphi, -sinphi, cosphi);
				//Hlocal *= LinearMapR2( *wPtr, 0.0, 0.0, (*wPtr)*(theta*sinthetaInv)*costheta );
				//Hlocal *= LinearMapR2( cosphi, -sinphi, sinphi, cosphi);
				sinphiSq = sinphi*sinphi;
				cosphiSq = cosphi*cosphi;
				tt = (*wPtr)*(theta*sinthetaInv)*costheta;
				offdiag = cosphi*sinphi*((*wPtr)-tt);
				Hlocal.Set( (*wPtr)*cosphiSq+tt*sinphiSq, 
							offdiag, offdiag,
							(*wPtr)*sinphiSq+tt*cosphiSq );
				Hessian += Hlocal;
			}
			wPtr++;
			vi++;
		}

		xDisplocal = Hessian.Solve( gradient );
		xDisp = xDisplocal.x*Clocalx + xDisplocal.y*Clocaly;

		// cout << "    xDisp = " << xDisp << "\n";  // DEBUG

		// Step 2c: rotate xVec in direction xDisp, for new estimate.

		VectorR3 xVecOld = xVec;
		xVec.RotateUnitInDirection(xDisp);

		xVec.Normalize();				// Avoid roundoff error problems

		// cout << xVec << "\n";	// DEBUG

		double errorAmt = (xVec-xVecOld).MaxAbs();
		if ( errorAmt <= tolerance ) {
			break;					// return xVec as answer
		}
		else {
			if (errorAmt <= bkuptolerance) {
				tolerance = bkuptolerance;
			}
		}
	}

	return xVec;
}

//
// ComputeMeanSphere ( long Num, const VectorR4[] ) takes an array of N unit vectors.
//	It returns the linear interpolation of these N vectors.
//	The vectors should all lie in a hemisphere otherwise odd results or even
//  floating point exceptions may occur.
//
VectorR4 ComputeMeanSphere( long Num, const VectorR4 vv[], const double weights[],
							double tolerance, double bkuptolerance ) 
{
	//  Step 1: get an initial estimate for the mean
	VectorR4 xVec = WeightedSum ( Num, vv, weights );
	xVec.Normalize();	// Initial estimate for the mean.

	return (ComputeMeanSphere( Num, vv, weights, 
							   xVec, SPHERICAL_NOTQUAT, tolerance, bkuptolerance ) );
}

// ComputeMeanQuat computes the weighted average a vector of quaternions.
//     This is a generalization of linear interpolation.
//     It is basically the same as ComputeMeanSphere, except in
//		choice of initial vector.

Quaternion ComputeMeanQuat( long Num, const Quaternion qq[], const double weights[],
						   double tolerance, double bkuptolerance ) 
{
	// Transform into an array of VectorR4's.
	VectorR4* vvArray = new VectorR4[Num];
	long i;
	for (i=0; i<Num; i++) {
		vvArray[i] = VectorR4( *(qq+i) );
	}

	// Find the coordinate with the max. total absolute value.
	double xAbsSum=0, yAbsSum=0, zAbsSum=0, wAbsSum=0;
	for (i=0; i<Num; i++) {
		xAbsSum += fabs(vvArray[i].x);
		yAbsSum += fabs(vvArray[i].y);
		zAbsSum += fabs(vvArray[i].z);
		wAbsSum += fabs(vvArray[i].w);
	}

	VectorR4 InitialVec(0.0, 0.0, 0.0, 0.0);
	if (xAbsSum>yAbsSum) {
		if (xAbsSum>zAbsSum) {
			if (xAbsSum>wAbsSum) {
				InitialVec.x = 1.0;
			}
			else {
				InitialVec.w = 1.0;
			}
		}
		else if (zAbsSum>wAbsSum) {
			InitialVec.z = 1.0;
		}
		else {
			InitialVec.w = 1.0;
		}
	}
	else if (yAbsSum>zAbsSum) {
		if (yAbsSum>wAbsSum) {
			InitialVec.y = 1.0;
		}
		else {
			InitialVec.w = 1.0;
		}
	}
	else if (zAbsSum>wAbsSum) {
		InitialVec.z = 1.0;
	}
	else {
		InitialVec.w = 1.0;
	}

	for (i=0; i<Num; i++) {
		if ( (InitialVec^vvArray[i]) < 0 ) {
			vvArray[i] = -vvArray[i];
		}
	}

	InitialVec = WeightedSum ( Num, vvArray, weights );
	InitialVec.Normalize();

	Quaternion MeanQuat;
	MeanQuat.Set( ComputeMeanSphere( Num, vvArray, weights,
									 InitialVec, SPHERICAL_QUAT,
									 tolerance, bkuptolerance ) );

	delete[] vvArray;
	return MeanQuat;
}



VectorR4 ComputeMeanSphere( long Num, const VectorR4 vv[], const double weights[],
						   const VectorR4& InitialVec, int QuatFlag,
						   double tolerance, double bkuptolerance )
{
	// Step 1. Get the initial guess

	VectorR4 xVec = InitialVec;

#if 0
printf ("\nComputeMeanSphere initial xVec = %f %f %f %f\n",
xVec.x, xVec.y, xVec.z, xVec.w);
#endif

	// Step 2: loop, doing Newton-style iterations to improve the estimate.

	VectorR4 *localvv = new VectorR4[Num];

	VectorR4 Clocalx, Clocaly, Clocalz;
	RotationMapR4 CbasisMat;
	VectorR4 vPerp;
	long i;
	VectorR3 gradient;			// net gradient: Actually (-1/2) times the gradient
	VectorR3 gradlocal;
	VectorR3 vPlocaly, vPlocalz;  // Local y z axes that complement gradlocal
	const VectorR4* vi;
	double costheta, sintheta, theta;
	double sinthetaInv;
	const double* wPtr;
	VectorR3 xDisplocal;
	VectorR4 xDisp;

	RotationMapR3 RotMat;
	LinearMapR3 Hlocal;		// Single point's Hessian in local coordinates
	LinearMapR3 Hessian;	// Net Hessian in local coordinates

	while (1) {



#if 0
printf ("Iteration\n");
#endif




		// Step 2a: for each vv vector, compute the tangent vector from xVec 
		//		towards the vv vector -- its length is the spherical length 
		//		from xVec to the vv vector.

		GetOrtho( xVec, CbasisMat);
		Clocalx = CbasisMat.Column2();
		Clocaly = CbasisMat.Column3();
		Clocalz = CbasisMat.Column4();
		gradient.SetZero();
		Hessian.SetZero();
		vi = vv;
		wPtr = weights;
		for ( i=0; i<Num; i++ ) {
			//  Loop invariants:    vi points to vv[i]  (i-th vector v_i)
			//						wPtr points to weights[i].
			vPerp = ProjectPerpUnitDiff( *vi, xVec );
			sintheta = vPerp.Norm();
			if ( sintheta==0.0 ) {
				Hessian += LinearMapR3( *wPtr, 0.0, 0.0, 0.0, *wPtr, 0.0, 0.0, 0.0, *wPtr);
			}
			else {
				sinthetaInv = 1.0/sintheta;
				costheta = (*vi)^xVec;
				if ( QuatFlag==SPHERICAL_QUAT && costheta<0.0 ) {
					vPerp = -vPerp;
				}
				theta = atan2( sintheta, costheta );
				vPerp *= sinthetaInv;
				gradlocal.Set ( vPerp^Clocalx, vPerp^Clocaly, vPerp^Clocalz );
				gradient += ((*wPtr)*theta)*gradlocal;
				double tt = (*wPtr)*(theta*sinthetaInv)*costheta;

				GetOrtho( gradlocal, vPlocaly, vPlocalz );
				RotMat.SetByRows (gradlocal, vPlocaly, vPlocalz ); 
						// Translates from vPrep coordinates into Clocal coordinates.

				Hlocal = RotMat;
				Hlocal.MakeTranspose();		// To translate into Clocal coordinates
				//Hlocal *= LinearMapR3(*wptr, 0.0, 0.0, 0.0, tt, 0.0, 0.0, 0.0, tt);
				Hlocal.m11 *= *wPtr;
				Hlocal.m21 *= *wPtr;
				Hlocal.m31 *= *wPtr;
				Hlocal.m12 *= tt;
				Hlocal.m22 *= tt;
				Hlocal.m32 *= tt;
				Hlocal.m13 *= tt;
				Hlocal.m23 *= tt;
				Hlocal.m33 *= tt;
				Hlocal *= RotMat;			// To translate out of Clocal coordinates
				Hessian += Hlocal;
			}
			vi++;
			wPtr++;
		}

		xDisplocal = Hessian.Solve( gradient );
		xDisp = xDisplocal.x*Clocalx + xDisplocal.y*Clocaly + xDisplocal.z*Clocalz;

		// cout << "    xDisp = " << xDisp << "\n";  // DEBUG

		// Step 2c: rotate xVec in direction xDisp, for new estimate.

		VectorR4 xVecOld = xVec;
		xVec.RotateUnitInDirection(xDisp);

		xVec.Normalize();				// Avoid roundoff error problems

		// cout << xVec << "\n";	// DEBUG

		double errorAmt = (xVec-xVecOld).MaxAbs();
#if 0
printf ("errorAmt = %le   tolerance = %le \n", errorAmt, tolerance);
#endif

		if ( errorAmt <= tolerance ) {
			break;					// return xVec as answer
		}
		else {
			if (errorAmt <= bkuptolerance) {
				tolerance = bkuptolerance;
			}
		}
	}

	delete[] localvv;
#if 0
printf ("\nComputeMeanSphere final   xVec = %f %f %f %f\n\n",
xVec.x, xVec.y, xVec.z, xVec.w);
#endif
	return xVec;
}


//
// ComputeMeanSphereSlow ( long Num, const Vector[], double weights ) 
//  takes an array of N unit vectors.
//	It returns the spherical mean of these N vectors.
//	The vectors should all lie in a hemisphere or odd results or even
//  floating point exceptions may occur.
//
//  This "slow" version is much easier to code.  It may be faster when
//	the tolerance is low.  It implements a linear convergence rate
//  algorithm (Algorithm A1 of Buss-Fillmore).

VectorR3 ComputeMeanSphereSlow( long Num, const VectorR3 vv[], const double weights[],
							double tolerance, double bkuptolerance ) 
{
	VectorR3 xVec;
	const VectorR3 *vvPtr = vv;
	const double *wghtPtr = weights;

	//  Step 1: get an initial estimate for the mean
	xVec = WeightedSum ( Num, vv, weights );
	xVec.Normalize();	// Initial estimate for the mean.

	// Step 2: loop, doing Newton-style iterations to improve the estimate.
	VectorR3 *localvv = new VectorR3[Num];
	
	while (1) {

		// Step 2a: for each vv vector, compute the tangent vector from xVec 
		//		towards the vv vector -- its length is the spherical length 
		//		from xVec to the vv vector.

		const VectorR3 *vi = vv;
		for ( long i=0; i<Num; i++, vi++ ) {
			double costheta = (*vi)^xVec;
			VectorR3 vPerp = (*vi) - costheta*xVec;
			double sintheta = vPerp.Norm();
			if (sintheta == 0) {
				localvv[i].SetZero();
			}
			else {
				double theta = atan2( sintheta, costheta );
				localvv[i] = (theta/sintheta)*vPerp;
			}
		}

		// Step 2b: compute the mean of the vectors resulting from Step 2a

		VectorR3 xDisp = WeightedSum ( Num, localvv, weights );

		// cout << "xDisp = " << xDisp << "\n";  // DEBUG

		// Step 2c: rotate xVec in direction xDisp, for new estimate.

		VectorR3 xVecOld = xVec;
		xVec.RotateUnitInDirection(xDisp);

		xVec.Normalize();				// Avoid roundoff error problems

		// cout << xVec << "\n";	// DEBUG


		double errorAmt = (xVec-xVecOld).MaxAbs();
		if ( errorAmt <= tolerance ) {
			break;					// return xVec as answer
		}
		else {
			if (errorAmt <= bkuptolerance) {
				tolerance = bkuptolerance;
			}
		}
	}

	delete[] localvv;
	return xVec;
}

// Same as previous routine, but for VectorR4's.

VectorR4 ComputeMeanSphereSlow( long Num, const VectorR4 vv[], const double weights[],
							double tolerance, double bkuptolerance ) 
{
	VectorR4 xVec;
	const VectorR4 *vvPtr = vv;
	const double *wghtPtr = weights;

	//  Step 1: get an initial estimate for the mean
	xVec = WeightedSum ( Num, vv, weights );
	xVec.Normalize();	// Initial estimate for the mean.

	// Step 2: loop, doing non-Newton-style iterations to improve the estimate.
	VectorR4 *localvv = new VectorR4[Num];
	
	while (1) {

		// Step 2a: for each vv vector, compute the tangent vector from xVec 
		//		towards the vv vector -- its length is the spherical length 
		//		from xVec to the vv vector.

		const VectorR4 *vi = vv;
		for ( long i=0; i<Num; i++, vi++ ) {
			double costheta = (*vi)^xVec;
			VectorR4 vPerp = (*vi) - costheta*xVec;
			double sintheta = vPerp.Norm();
			if (sintheta == 0) {
				localvv[i].SetZero();
			}
			else {
				double theta = atan2( sintheta, costheta );
				localvv[i] = (theta/sintheta)*vPerp;
			}
		}

		// Step 2b: compute the mean of the vectors resulting from Step 2a
		VectorR4 xDisp = WeightedSum ( Num, localvv, weights );
		// cout << "xDisp = " << xDisp << "\n";  // DEBUG

		// Step 2c: rotate xVec in direction xDisp, for new estimate.
		VectorR4 xVecOld = xVec;
		xVec.RotateUnitInDirection(xDisp);

		xVec.Normalize();				// Avoid roundoff error problems
		// cout << xVec << "\n";	// DEBUG


		double errorAmt = (xVec-xVecOld).MaxAbs();
		if ( errorAmt <= tolerance ) {
			break;					// return xVec as answer
		}
		else {
			if (errorAmt <= bkuptolerance) {
				tolerance = bkuptolerance;
			}
		}
	}

	delete[] localvv;
	return xVec;
}

// **********************************************************************
// General purpose routine for computing weighted average of a set		*
//	of VectorR3's.														*
// **********************************************************************

VectorR3 WeightedSum( long Num, const VectorR3 vv[], const double weights[] ) 
{

	VectorR3 xVec;
	const VectorR3 *vvPtr = vv;
	const double *wghtPtr = weights;

	long i;
	for (i=0; i<Num; i++) {
		xVec += (*vvPtr)*(*wghtPtr);
		vvPtr++;
		wghtPtr++;
	}

	return xVec;
}

// General purpose routine for computing weighted average of a set
//	of VectorR4's.

VectorR4 WeightedSum( long Num, const VectorR4 vv[], const double weights[] ) 
{

	VectorR4 xVec;
	const VectorR4 *vvPtr = vv;
	const double *wghtPtr = weights;

	long i;
	for (i=0; i<Num; i++) {
#if 0
printf ("weightedSum:   vv[%d] = %f %f %f %f\n",
i, vvPtr->x, vvPtr->y, vvPtr->z, vvPtr->w);
#endif

		xVec += (*vvPtr)*(*wghtPtr);
		vvPtr++;
		wghtPtr++;
	}

	return xVec;
}





