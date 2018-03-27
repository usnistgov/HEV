/*
 * SpherTriDiag.cpp: Release 2.1, June 2004.
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

#include "SphereTriDiag.h"
#include "../VrMath/MathMisc.h"
#include "../VrMath/LinearR2.h"
#include "../VrMath/LinearR3.h"
#include "../VrMath/LinearR4.h"


// ********************************************************************************
// SolveTriDiagSphere
//
// Solves a system of spherical-mean equalities, where the system is
// given as a tridiagonal matrix.
//   numPoints is the number of equations.
//   The i-th equation says c_i = a_{i,1}p_{i-1} + a_{i,2}p_{i} + a_{i,3}p_{i+1}.
//   The weights must have sum 1 = \sum_{j=1,2,3} a_{i,j}.  And each a_{i,j} >= 0
//		the weighted sum is computed as a "SphericalMean".
//   The values a{i,j} are specified as coefs[i][j].
//	 The values c_i are c[i].
//   The values of p_i are returned in the array p, as p[i].
//   The values a_{0,1} and a_{numPoints-1,3} should be zero. 
//
// There are two versions of SolveTriDiagSphere.  The basic version 
//	 is "SolveTriDiagSphere" and uses Newton iteration with a quadratic
//	 rate of convergence.
//	 The "Slow" version is simpler, but uses a iterative method with a 
//	 linear rate of convergence.  This might be faster for low accuracy applications.
// The Newton iteration algorithm is Algorithm S2 of Buss-Fillmore.
// The "Slow" version is Algorithm S1 of Buss-Fillmore.
// 
// Algorithms based on											 
//    S. Buss and J. Fillmore, "Spherical Averages and           
//      Applications to Spherical Splines and Interpolation."    
//      ACM Transactions on Graphics 20 (2001) 95-126.
//
// *********************************************************************************

// The quadratic convergence rate version 
void SolveTriDiagSphere ( int numPoints, 
						   const double* coefs, const VectorR3* c,
						   VectorR3* p, double accuracy, double bkupaccuracy )
{
	// The c[] vector contains c_i values.  These are the interpolation points.
	// The p[] vector is to contain the p_i values returned.  These are control points.
	VectorR3* Clocalbasis = new VectorR3[numPoints+numPoints];		
										// Local bases for the c[] points.
	VectorR3* Plocalbasis = new VectorR3[numPoints+numPoints];		
										// Local bases for the p[] points.
	LinearMapR2* PM = new LinearMapR2[numPoints*3];					
										// Tridiagonal matrix of LinearMapR2's
	VectorR2* dir = new VectorR2[numPoints];						
										// Discrepancy of approximation

	const VectorR3* cPtr;	// Points to c[] array
	VectorR3* pPtr;			// Points to p[] array
	VectorR3* PlbPtr;		// Points to Plocalbasis[] array
	VectorR3* ClbPtr;		// Points to Clocalbasis[] array
	LinearMapR2* PMptr;		// Points to PM[][] array
	const double* cfPtr;	// Points to coefs[] array.
	VectorR2* dPtr;			// Points to dir[] array

	long i;
	cPtr = c;
	pPtr = p;
	ClbPtr = Clocalbasis;
	for (i=numPoints; i>0; i--) {
		GetOrtho(*cPtr, *ClbPtr, *(ClbPtr+1));		// Set up c_i 's local basis (x,y axes)
		*(pPtr++) = *(cPtr++);		// Copy c[] to p[] as inital guess for control points
		ClbPtr+=2;
	}
	// Instead of the above estimate p_i = c_i,
	//	call the "slow" version to get a better initial estimate
	SolveTriDiagSphereSlow(numPoints, coefs, c, p, 0.002, 0.002);

	double theta;
	double sintheta, costheta;
	double sinthetaInv;
	double cosphi, sinphi;
	VectorR3 vProj, vPerp;
	LinearMapR2 tempMat;
	VectorR3 dirR3;				// c's discrepancy as an x-y-z vector
	VectorR2 preSoln;
	double maxDeltaSq;			// Max. change (squared) in current update cycle
	double accuracySq = accuracy*accuracy;
	double bkupaccSq = bkupaccuracy*bkupaccuracy;

	while (1) {

		// Set up local bases for p[i]'s
		//	and begin the calculation of the discrepancies at c_i's 
		//  and calculate the derivative matrices PM[i][1].
		pPtr = p;
		cPtr = c;
		cfPtr = coefs;
		PlbPtr = Plocalbasis;
		ClbPtr = Clocalbasis;
		dPtr = dir;
		PMptr = PM;
		for ( i=0; i<numPoints; i++ ) {
			// Loop invariants:  pPtr points to p[i],   cPtr points to c[i]
			//		cfPtr points to coefs[i][0],  dPtr points to dir[i],
			//		PlbPtr and ClbPtr point to Plocalbasis[i][0] and Clocalbasis[i][0]
			//		PMptr points to PM[i][0]
			*PlbPtr = ProjectPerpUnitDiff ( *cPtr, *pPtr );
			sintheta = (*PlbPtr).Norm();
			if ( sintheta >= 1.0e-10 ) {
				vProj = *cPtr-(*PlbPtr);
				sinthetaInv = 1.0/sintheta;
				*PlbPtr *= -sinthetaInv;			// Normalize & Negate: x-axis at p_i
				*(PlbPtr+1) = (*pPtr)*(*PlbPtr);	// y-axis at p_i
				costheta = vProj^(*pPtr);
				theta = atan2( sintheta, costheta );			// angle from p_i to c_i
				vPerp = ProjectPerpUnitDiff ( *pPtr, *cPtr );	// From c_i towards p_i
				vPerp *= sinthetaInv;							// Normalize
				dirR3 = (-(*(cfPtr+1))*theta)*vPerp;
				dPtr->Set ( dirR3^(*ClbPtr), dirR3^(*(ClbPtr+1)) );
				cosphi = vPerp^(*ClbPtr);
				sinphi = vPerp^(*(ClbPtr+1));
				(PMptr+1)->Set( cosphi, sinphi, -sinphi, cosphi );
				tempMat.Set( *(cfPtr+1), 0.0, 0.0, (*(cfPtr+1))*theta*sinthetaInv );
				*(PMptr+1) *= tempMat;
			}
			else {
				dPtr->SetZero();							// p_i and c_i coincide
				*PlbPtr = *ClbPtr;							// Set up local basis for p_i
				*(PlbPtr+1) = *(ClbPtr+1);					//    (just use c_i's)
				(PMptr+1)->Set( *(cfPtr+1), 0.0, 0.0, *(cfPtr+1) );		// Initialize PM_i.
			}
			pPtr++;
			cPtr++;
			cfPtr+=3;
			dPtr++;
			PlbPtr+=2;
			ClbPtr+=2;
			PMptr+=3;
		}


		//	Finish the calculation of the discrepancies at c_i's 
		//  Calculate the derivative matrices PM[i][0] and PM[i][2].
		pPtr = p;
		cPtr = c;
		cfPtr = coefs;
		PlbPtr = Plocalbasis;
		ClbPtr = Clocalbasis;
		dPtr = dir;
		PMptr = PM;
		for ( i=0; i<numPoints; i++ ) {
			// Loop invariants:  pPtr points to p[i],   cPtr points to c[i]
			//		cfPtr points to coefs[i][0],  dPtr points to dir[i],
			//		PlbPtr and ClbPtr point to Plocalbasis[i][0] and Clocalbasis[i][0]
			//		PMptr points to PM[i][0]
			dirR3.SetZero();
			if (i>0) {
				vPerp = ProjectPerpUnitDiff( *(pPtr-1), *cPtr );
				sintheta = vPerp.Norm();
				if ( sintheta >= 1.0e-10 ) {
					sinthetaInv = 1.0/sintheta;
					vProj = (*(pPtr-1))-vPerp;
					vPerp *= sinthetaInv;						// Normalize
					costheta = vProj^(*cPtr);
					theta = atan2( sintheta, costheta );
					dirR3 = -(*(cfPtr))*theta*vPerp;
					cosphi = vPerp^(*ClbPtr);
					sinphi = vPerp^(*(ClbPtr+1));
					PMptr->Set(cosphi, sinphi, -sinphi, cosphi);	// Rotate into c_i's local basis
					tempMat.Set( *(cfPtr), 0.0, 0.0, (*(cfPtr))*theta*sinthetaInv );
					*PMptr *= tempMat;
					vPerp = ProjectPerpUnitDiff( *cPtr, *(pPtr-1) );
					vPerp *= -sinthetaInv;		// Normalize and negate to point away from c_i
					cosphi = vPerp^(*(PlbPtr-2));
					sinphi = vPerp^(*(PlbPtr-1));
					tempMat.Set(cosphi, -sinphi, sinphi, cosphi);		// Rotate out of p_i's local basis
					*PMptr *= tempMat;
				}
				else {
					PMptr->Set( *(cfPtr), 0.0, 0.0, *(cfPtr) );
				}
			}
			if (i<numPoints-1) {
				vPerp = ProjectPerpUnitDiff( *(pPtr+1), *cPtr );
				sintheta = vPerp.Norm();
				if ( sintheta >= 1.0e-10 ) {
					sinthetaInv = 1.0/sintheta;
					vProj = (*(pPtr+1))-vPerp;
					vPerp *= sinthetaInv;						// Normalize
					costheta = vProj^(*cPtr);
					theta = atan2( sintheta, costheta );
					dirR3 -= (*(cfPtr+2))*theta*vPerp;
					cosphi = vPerp^(*ClbPtr);
					sinphi = vPerp^(*(ClbPtr+1));
					(PMptr+2)->Set(cosphi, sinphi, -sinphi, cosphi);	// Rotate into c_i's local basis
					tempMat.Set( *(cfPtr+2), 0.0, 0.0, (*(cfPtr+2))*theta*sinthetaInv );
					*(PMptr+2) *= tempMat;
					vPerp = ProjectPerpUnitDiff( *cPtr, *(pPtr+1) );
					vPerp *= -sinthetaInv;						// Normalize and negate to point away from c_i
					cosphi = vPerp^(*(PlbPtr+2));
					sinphi = vPerp^(*(PlbPtr+3));
					tempMat.Set(cosphi, -sinphi, +sinphi, cosphi);		// Rotate out of p_i's local basis
					*(PMptr+2) *= tempMat;
				}
				else {
					(PMptr+2)->Set( *(cfPtr+2), 0.0, 0.0, *(cfPtr+2) );
				}
			}
			// dirR3 now has the discrepancy at c_i.
			//		Convert this to c_i's local coordinate system
			*dPtr += VectorR2( dirR3^(*ClbPtr), dirR3^(*(ClbPtr+1)) );
			pPtr++;
			cPtr++;
			cfPtr+=3;
			dPtr++;
			PlbPtr+=2;
			ClbPtr+=2;
			PMptr+=3;
		}

		// The PM[i][3] forms a tri-diagonal matrix where the entries are
		//		LinearMapR2's. 
		// Solve for this with the dir[] matrix.
		
		// Phase 1 - use row operations to make the make bi-upper diagonal
		//				with all diagonal elements the identity
		PMptr = PM+1;
		dPtr = dir;
		for (i=0; i<numPoints; i++) {
			// Loop invariants:		PMptr, PMptr+1 point to PM[i][1] and PM[i][2]
			//						PMptr+2, PMptr+3 point to PM[i+1][0] and PM[i+1][1]
			//						PMptr[i][0] is zero already
			//						dPtr points to d[i]
			tempMat = PMptr->Inverse();
			*dPtr = tempMat*(*dPtr);			// Mult row i by PM[i][1] inverse
			PMptr->SetIdentity();
			*(PMptr+1) = tempMat*(*(PMptr+1));
			if (i < numPoints-1) {
				*(dPtr+1) -= (*(PMptr+2))*(*dPtr);
				*(PMptr+3) -= (*(PMptr+2))*(*(PMptr+1));
				(PMptr+2)->SetZero();
			}
			dPtr++;
			PMptr += 3;
		}
		// Phase 2 - solve the tridiagonal systems.
		//		Rather than saving the solution, we will just go ahead and
		//		add the values on to the p[i] values.
		preSoln = dir[numPoints-1];
		PlbPtr = Plocalbasis + (2*(numPoints-1));
		p[numPoints-1].RotateUnitInDirection( preSoln.x*(*PlbPtr) 
											 +preSoln.y*(*(PlbPtr+1)) );
		maxDeltaSq = preSoln.NormSq();
		PlbPtr -= 2;
		dPtr = dir+(numPoints-2);
		pPtr = p+(numPoints-2);
		PMptr = PM+(3*numPoints-4);			// pointer to PM[numPoints-2][2]
		for (i=numPoints-2; i>=0; i--) {
			// LoopInvariants:   PMptr points to p[i][2]
			//					 dPtr points to d[i]
			//					 pPtr points to p[i]
			//					 PlbPtr points to Plocalbasis[i][0]
			//					 preSoln is solved for (i+1)st value
			preSoln = (*dPtr) - (*PMptr)*preSoln;
			pPtr->RotateUnitInDirection( preSoln.x*(*PlbPtr) 
										+preSoln.y*(*(PlbPtr+1)) );  // Update p_i
			double thisDeltaSq = preSoln.NormSq();
			if ( maxDeltaSq < thisDeltaSq ) {
				maxDeltaSq = thisDeltaSq;
			}
			dPtr--;
			pPtr--;
			PlbPtr -= 2;
			PMptr -= 3;
		}

		// Decide whether we are close enough to exit loop
		if ( maxDeltaSq < accuracySq ) {
			break;
		}
		if ( maxDeltaSq < bkupaccSq ) {
			accuracySq = bkupaccSq;
			// accuracy = bkupaccuracy;
		}
	}

	delete[] dir;
	delete[] PM;
	delete[] Plocalbasis;
	delete[] Clocalbasis;
	return;
}

// The "slow" version uses a iterative method with a linear rate of convergence.
void SolveTriDiagSphereSlow ( int numPoints, 
						   const double* coefs, const VectorR3* c,
						   VectorR3* p, double accuracy, double bkupaccuracy )
{

#define STDSS_SINGLEBUFFER
#ifndef STDSS_SINGLEBUFFER
	VectorR3* ptemp = new VectorR3[numPoints];
	VectorR3* tempPtr;
#else
	VectorR3* ptemp = p;
#endif

	VectorR3* fromPtr;
	VectorR3* toPtr;
	VectorR3* fromP;
	VectorR3* toP;
	const VectorR3* cPtr;
	const double* cfPtr;
	double sintheta, costheta, theta;
	VectorR3 vPerp, vProj;
	VectorR3 dir;
	VectorR3 oldPi;
	double maxError;

	long i;
	cPtr = c;
	toP = ptemp;
	for (i=numPoints; i>0; i--) {		// Copy c[] to ptemp[]
		*(toP++) = *(cPtr++);
	}

	fromPtr = ptemp;
	toPtr = p;
	while (1) {
		fromP = fromPtr;
		toP = toPtr;
		cPtr = c;
		cfPtr = coefs;
		maxError = 0.0;

		for ( i=0; i<numPoints; i++ ) {
			// Loop invariants:		fromP - points to fromPtr[i];
			//						toP - points to toPtr[i];
			//						cPtr - points to c[i];
			//						cfPtr - points to coefs[i][0]
			
			dir.SetZero();
			// Get direction from c_i to from-p_{i-1}
			if (i>0) {
				vPerp = ProjectPerpUnitDiff ( *(fromP-1), *cPtr );
				vProj = *(fromP-1)-vPerp;
				sintheta = vPerp.Norm();
				costheta = vProj^(*cPtr);
				theta = atan2( sintheta, costheta );	
				if ( sintheta != 0.0 ) {
					dir = ((*cfPtr)*theta/sintheta)*vPerp;
				}
			}
			// Add on the direction from c_i to p_{i+1}
			if ( i<numPoints-1 ) {
				vPerp = ProjectPerpUnitDiff ( *(fromP+1), *cPtr );
				vProj = (*(fromP+1))-vPerp;
				sintheta = vPerp.Norm();
				costheta = vProj^(*cPtr);
				theta = atan2( sintheta, costheta );
				if (sintheta>0.0) {
					dir += ((*(cfPtr+2))*theta/sintheta)*vPerp;
				}
			}

			oldPi = *fromP;						// Previous value of p_i
			(*toP) = *cPtr;
			dir /= -(*(cfPtr+1));					// Scale and negate
			toP->RotateUnitInDirection( dir );	// Here we set the new value of p_{i}

			double errorAmt = ((*toP)-oldPi).MaxAbs();
			if ( errorAmt>maxError) {
				maxError = errorAmt;
			}

			fromP++;
			toP++;
			cPtr++;
			cfPtr += 3;
		}

		if (maxError<accuracy) {
			break;				// We're done if sufficiently close: EXIT WHILE LOOP
		}
		else if ( maxError<bkupaccuracy ) {
			accuracy = bkupaccuracy;	// Quite close: so lower our future standards
		}

#ifndef STDSS_SINGLEBUFFER
		tempPtr = fromPtr;				// Swap to and from arrays
		fromPtr = toPtr;
		toPtr = tempPtr;
#endif
	}

#ifndef STDSS_SINGLEBUFFER
	// Copy values into p[] array, if necessary.
	if ( fromPtr==ptemp ) {
		VectorR3* pPtr = p;
		VectorR3* ptPtr = ptemp;
		for (int i=numPoints; i>0; i--) {
			*(pPtr++) = *(ptPtr++);
		}
	}
	delete[] ptemp;
#endif

	return;
}

// The "slow" version uses a iterative method with a linear rate of convergence.
// This is the version for the 3-sphere.

void SolveTriDiagSphereSlow ( int numPoints, 
						   const double* coefs, const VectorR4* c,
						   VectorR4* p, double accuracy, double bkupaccuracy )
{

#define STDSS_SINGLEBUFFER
#ifndef STDSS_SINGLEBUFFER
	VectorR4* ptemp = new VectorR3[numPoints];
	VectorR4* tempPtr;
#else
	VectorR4* ptemp = p;
#endif

	VectorR4* fromPtr;
	VectorR4* toPtr;
	VectorR4* fromP;
	VectorR4* toP;
	const VectorR4* cPtr;
	const double* cfPtr;
	double sintheta, costheta, theta;
	VectorR4 vPerp, vProj;
	VectorR4 dir;
	VectorR4 oldPi;
	double maxError;

	long i;
	cPtr = c;
	toP = ptemp;
	for (i=numPoints; i>0; i--) {		// Copy c[] to ptemp[]
		*(toP++) = *(cPtr++);
	}

	fromPtr = ptemp;
	toPtr = p;
	while (1) {
		fromP = fromPtr;
		toP = toPtr;
		cPtr = c;
		cfPtr = coefs;
		maxError = 0.0;

		for ( i=0; i<numPoints; i++ ) {
			// Loop invariants:		fromP - points to fromPtr[i];
			//						toP - points to toPtr[i];
			//						cPtr - points to c[i];
			//						cfPtr - points to coefs[i][0]
			
			dir.SetZero();
			// Get direction from c_i to p_{i-1}
			if (i>0) {
				vPerp = ProjectPerpUnitDiff ( *(fromP-1), *cPtr );
				vProj = *(fromP-1)-vPerp;
				sintheta = vPerp.Norm();
				costheta = vProj^(*cPtr);
				theta = atan2( sintheta, costheta );	
				if ( sintheta != 0.0 ) {
					dir = ((*cfPtr)*theta/sintheta)*vPerp;
				}
			}
			// Add on the direction from c_i to p_{i+1}
			if ( i<numPoints-1 ) {
				vPerp = ProjectPerpUnitDiff ( *(fromP+1), *cPtr );
				vProj = (*(fromP+1))-vPerp;
				sintheta = vPerp.Norm();
				costheta = vProj^(*cPtr);
				theta = atan2( sintheta, costheta );
				if (sintheta>0.0) {
					dir += ((*(cfPtr+2))*theta/sintheta)*vPerp;
				}
			}

			oldPi = *fromP;						// Previous value of p_i
			(*toP) = *cPtr;
			dir /= -(*(cfPtr+1));					// Scale and negate
			toP->RotateUnitInDirection( dir );	// Here we set the new value of p_{i}

			double errorAmt = ((*toP)-oldPi).MaxAbs();
			if ( errorAmt>maxError) {
				maxError = errorAmt;
			}

			fromP++;
			toP++;
			cPtr++;
			cfPtr += 3;
		}

		if (maxError<accuracy) {
			break;				// We're done if sufficiently close: EXIT WHILE LOOP
		}
		else if ( maxError<bkupaccuracy ) {
			accuracy = bkupaccuracy;	// Quite close: so lower our future standards
		}

#ifndef STDSS_SINGLEBUFFER
		tempPtr = fromPtr;				// Swap to and from arrays
		fromPtr = toPtr;
		toPtr = tempPtr;
#endif
	}

#ifndef STDSS_SINGLEBUFFER
	// Copy values into p[] array, if necessary.
	if ( fromPtr==ptemp ) {
		VectorR4* pPtr = p;
		VectorR4* ptPtr = ptemp;
		for (int i=numPoints; i>0; i--) {
			*(pPtr++) = *(ptPtr++);
		}
	}
	delete[] ptemp;
#endif

	return;
}

// The quadratic convergence rate version for the 3-sphere
void SolveTriDiagSphere ( int numPoints, 
						   const double* coefs, const VectorR4* c,
						   VectorR4* p, double accuracy, double bkupaccuracy )
{
	// The c[] vector contains c_i values.  These are the interpolation points.
	// The p[] vector is to contain the p_i values returned.  These are control points.
	VectorR4* Clocalbasis = new VectorR4[numPoints+numPoints+numPoints];		
										// Local bases for the c[] points.
	VectorR4* Plocalbasis = new VectorR4[numPoints+numPoints+numPoints];		
										// Local bases for the p[] points.
	LinearMapR3* PM = new LinearMapR3[numPoints*3];					
										// Tridiagonal matrix of LinearMapR3's
	VectorR3* dir = new VectorR3[numPoints];						
										// Discrepancy of approximation

	const VectorR4* cPtr;	// Points to c[] array
	VectorR4* pPtr;			// Points to p[] array
	VectorR4* PlbPtr;		// Points to Plocalbasis[] array
	VectorR4* ClbPtr;		// Points to Clocalbasis[] array
	LinearMapR3* PMptr;		// Points to PM[][] array
	const double* cfPtr;	// Points to coefs[] array.
	VectorR3* dPtr;			// Points to dir[] array
	RotationMapR4 RotMat;	// Used for storing 4 axes (Temporary)

	long i;
	cPtr = c;
	pPtr = p;
	ClbPtr = Clocalbasis;
	for (i=numPoints; i>0; i--) {
		GetOrtho(*cPtr, RotMat);		// Set up c_i 's local basis (x,y axes)
		*ClbPtr = RotMat.Column2();		// x axis at c_i
		*(ClbPtr+1) = RotMat.Column4();	// y axis at c_i
		*(ClbPtr+2) = RotMat.Column3();	// z axis at c_i
		*(pPtr++) = *(cPtr++);		// Copy c[] to p[] as inital guess for control points
		ClbPtr+=3;
	}
	// Instead of the above estimate p_i = c_i,
	//	call the "slow" version to get a better initial estimate
	SolveTriDiagSphereSlow(numPoints, coefs, c, p, 0.00001, 0.00001);

	double theta;
	double sintheta, costheta;
	double sinthetaInv;
	double sinphi, cosphi;
	double ttsinphi, ttcosphi;
	VectorR4 vProj, vPerp;
	LinearMapR3 tempMat, tempMat2;
	VectorR3 preSoln;
	double maxDeltaSq;			// Max. change (squared) in current update cycle
	double accuracySq = accuracy*accuracy;
	double bkupaccSq = bkupaccuracy*bkupaccuracy;
	VectorR3 tempR3;
	VectorR3 CrttdyR3;			// Rotated y/z axis at c_i in c_i's local basis
	VectorR4 CrttdyR4;			// Same as CrttdyR3 but in underlying R4 basis
	VectorR3 PrttdyzR3;			// Rotated y/z axis at p_i in p_i's local basis
	VectorR4 PrttdyzR4;			// Same as PrttdyR3 but in underlying R4 basis

	while (1) {

		// Set up local bases for p[i]'s
		//	and begin the calculation of the discrepancies at c_i's 
		//  and calculate the derivative matrices PM[i][1].
		pPtr = p;
		cPtr = c;
		cfPtr = coefs;
		PlbPtr = Plocalbasis;
		ClbPtr = Clocalbasis;
		dPtr = dir;
		PMptr = PM;
		for ( i=0; i<numPoints; i++ ) {
			// Loop invariants:  pPtr points to p[i],   cPtr points to c[i]
			//		cfPtr points to coefs[i][0],  dPtr points to dir[i],
			//		PlbPtr and ClbPtr point to Plocalbasis[i][0] and Clocalbasis[i][0]
			//		PMptr points to PM[i][0]
			*PlbPtr = ProjectPerpUnitDiff ( *cPtr, *pPtr );
			sintheta = (*PlbPtr).Norm();
			if ( sintheta >= 1.0e-10 ) {
				vProj = *cPtr-(*PlbPtr);
				sinthetaInv = 1.0/sintheta;
				*PlbPtr *= -sinthetaInv;			// Normalize & Negate: x-axis at p_i
				GetOrtho( *pPtr, *PlbPtr, RotMat );
				*(PlbPtr+1) = RotMat.Column4();		// y axis at p_i
				*(PlbPtr+2) = RotMat.Column3();		// z axis at p_i
				costheta = vProj^(*pPtr);
				theta = atan2( sintheta, costheta );			// angle from p_i to c_i
				vPerp = ProjectPerpUnitDiff ( *pPtr, *cPtr );	// From c_i towards p_i
				vPerp *= sinthetaInv;							// Normalize
				dPtr->Set ( vPerp^(*ClbPtr), vPerp^(*(ClbPtr+1)), vPerp^(*(ClbPtr+2)) );
				*(PMptr+1) = RotateToMap ( UnitVecIR3, *dPtr );
				CrttdyR3 = (PMptr+1)->Column2();
				CrttdyR4 = CrttdyR3.x*(*ClbPtr)+CrttdyR3.y*(*(ClbPtr+1))
											   +CrttdyR3.z*(*(ClbPtr+2));
				*dPtr *= -(*(cfPtr+1))*theta;			// Fix magnitude on discrepancy
				// Scale according to distance and weighting
				double tt = (*(cfPtr+1))*theta*sinthetaInv;
				// Rotate so that y and z axes are aligned
				//cosphi = CrttdyR4^(*(PlbPtr+1));
				//sinphi = CrttdyR4^(*(PlbPtr+2));
				//tempMat.Set( *(cfPtr+1), 0.0, 0.0, 0.0, tt, 0.0, 0.0, 0.0, tt );
				//*(PMptr+1) *= tempMat;
				//tempMat.Set( 1.0, 0.0, 0.0, 0.0, cosphi, -sinphi, 0.0, sinphi, cosphi);
				ttcosphi = tt*(CrttdyR4^(*(PlbPtr+1)));
				ttsinphi = tt*(CrttdyR4^(*(PlbPtr+2)));
				tempMat.Set( *(cfPtr+1), 0.0, 0.0, 0.0, ttcosphi, -ttsinphi,
												   0.0, ttsinphi,  ttcosphi );
				*(PMptr+1) *= tempMat;
			}
			else {
				dPtr->SetZero();							// p_i and c_i coincide
				*PlbPtr = *ClbPtr;							// Set up local basis for p_i
				*(PlbPtr+1) = *(ClbPtr+1);					//    (just use c_i's)
				*(PlbPtr+2) = *(ClbPtr+2);
				(PMptr+1)->Set( *(cfPtr+1), 0.0, 0.0, 
								0.0, *(cfPtr+1), 0.0, 
								0.0, 0.0, *(cfPtr+1) );		// Initialize PM_i.
			}
			pPtr++;
			cPtr++;
			cfPtr+=3;
			dPtr++;
			PlbPtr+=3;
			ClbPtr+=3;
			PMptr+=3;
		}


		//	Finish the calculation of the discrepancies at c_i's 
		//  Calculate the derivative matrices PM[i][0] and PM[i][2].
		pPtr = p;
		cPtr = c;
		cfPtr = coefs;
		PlbPtr = Plocalbasis;
		ClbPtr = Clocalbasis;
		dPtr = dir;
		PMptr = PM;
		for ( i=0; i<numPoints; i++ ) {
			// Loop invariants:  pPtr points to p[i],   cPtr points to c[i]
			//		cfPtr points to coefs[i][0],  dPtr points to dir[i],
			//		PlbPtr and ClbPtr point to Plocalbasis[i][0] and Clocalbasis[i][0]
			//		PMptr points to PM[i][0]
			if (i>0) {
				vPerp = ProjectPerpUnitDiff( *(pPtr-1), *cPtr );
				sintheta = vPerp.Norm();
				if ( sintheta >= 1.0e-10 ) {
					sinthetaInv = 1.0/sintheta;
					vProj = (*(pPtr-1))-vPerp;
					vPerp *= sinthetaInv;						// Normalize
					costheta = vProj^(*cPtr);
					theta = atan2( sintheta, costheta );
					tempR3.Set( vPerp^(*ClbPtr), vPerp^(*(ClbPtr+1)), vPerp^(*(ClbPtr+2)) );
					*dPtr -= ((*cfPtr)*theta)*tempR3;
					*PMptr = RotateToMap( UnitVecIR3, tempR3);
					CrttdyR3 = (PMptr)->Column2();
					CrttdyR4 = CrttdyR3.x*(*ClbPtr)+CrttdyR3.y*(*(ClbPtr+1))
												   +CrttdyR3.z*(*(ClbPtr+2));
					vPerp = ProjectPerpUnitDiff( *cPtr, *(pPtr-1) );
					vPerp *= -sinthetaInv;		// Normalize and negate to point away from c_i
					tempR3.Set( vPerp^(*(PlbPtr-3)), vPerp^(*(PlbPtr-2)), vPerp^(*(PlbPtr-1)) );
					tempMat2 = RotateToMap( tempR3, UnitVecIR3 );	// Rotate out of p_i's local basis
					PrttdyzR3 = tempMat2.Row2();
					PrttdyzR4 = PrttdyzR3.x*(*(PlbPtr-3)) + PrttdyzR3.y*(*(PlbPtr-2))
														  + PrttdyzR3.z*(*(PlbPtr-1));
					cosphi = CrttdyR4^PrttdyzR4;
					PrttdyzR3 = tempMat2.Row3();
					PrttdyzR4 = PrttdyzR3.x*(*(PlbPtr-3)) + PrttdyzR3.y*(*(PlbPtr-2))
														  + PrttdyzR3.z*(*(PlbPtr-1));
					sinphi = CrttdyR4^PrttdyzR4;
					// Scale according to distance and weighting
					double tt = (*(cfPtr))*theta*sinthetaInv;
					//tempMat.Set( *(cfPtr), 0.0, 0.0, 0.0, tt, 0.0, 0.0, 0.0, tt );
					//*PMptr *= tempMat;
					// Rotate so that y and z axes are aligned
					//tempMat.Set( 1.0, 0.0, 0.0, 0.0, cosphi, -sinphi, 0.0, sinphi, cosphi);
					ttcosphi = tt*cosphi;
					ttsinphi = tt*sinphi;
					tempMat.Set( *cfPtr, 0.0, 0.0, 0.0, ttcosphi, -ttsinphi,
												   0.0, ttsinphi,  ttcosphi );
					*PMptr *= tempMat;
					*PMptr *= tempMat2;
				}
				else {
					PMptr->Set( *(cfPtr), 0.0, 0.0, 0.0, *(cfPtr), 0.0, 0.0, 0.0, *cfPtr );
				}
			}
			if (i<numPoints-1) {
				vPerp = ProjectPerpUnitDiff( *(pPtr+1), *cPtr );
				sintheta = vPerp.Norm();
				if ( sintheta >= 1.0e-10 ) {
					sinthetaInv = 1.0/sintheta;
					vProj = (*(pPtr+1))-vPerp;
					vPerp *= sinthetaInv;						// Normalize
					costheta = vProj^(*cPtr);
					theta = atan2( sintheta, costheta );
					tempR3.Set( vPerp^(*ClbPtr), vPerp^(*(ClbPtr+1)), vPerp^(*(ClbPtr+2)) );
					*dPtr -= ((*(cfPtr+2))*theta)*tempR3;
					*(PMptr+2) = RotateToMap( UnitVecIR3, tempR3);
					CrttdyR3 = (PMptr+2)->Column2();
					CrttdyR4 = CrttdyR3.x*(*ClbPtr)+CrttdyR3.y*(*(ClbPtr+1))
												   +CrttdyR3.z*(*(ClbPtr+2));
					vPerp = ProjectPerpUnitDiff( *cPtr, *(pPtr+1) );
					vPerp *= -sinthetaInv;						// Normalize and negate to point away from c_i
					tempR3.Set( vPerp^(*(PlbPtr+3)), vPerp^(*(PlbPtr+4)), vPerp^(*(PlbPtr+5)) );
					tempMat2 = RotateToMap( tempR3, UnitVecIR3 );	// Rotate out of p_i's local basis
					PrttdyzR3 = tempMat2.Row2();
					PrttdyzR4 = PrttdyzR3.x*(*(PlbPtr+3)) + PrttdyzR3.y*(*(PlbPtr+4))
														  + PrttdyzR3.z*(*(PlbPtr+5));
					cosphi = CrttdyR4^PrttdyzR4;
					PrttdyzR3 = tempMat2.Row3();
					PrttdyzR4 = PrttdyzR3.x*(*(PlbPtr+3)) + PrttdyzR3.y*(*(PlbPtr+4))
														  + PrttdyzR3.z*(*(PlbPtr+5));
					sinphi = CrttdyR4^PrttdyzR4;
					// Scale according to distance and weighting
					double tt = (*(cfPtr+2))*theta*sinthetaInv;
					//tempMat.Set( *(cfPtr+2), 0.0, 0.0, 0.0, tt, 0.0, 0.0, 0.0, tt );
					//*(PMptr+2) *= tempMat;
					// Rotate so that y and z axes are aligned
					//tempMat.Set( 1.0, 0.0, 0.0, 0.0, cosphi, -sinphi, 0.0, sinphi, cosphi);
					ttcosphi = tt*cosphi;
					ttsinphi = tt*sinphi;
					tempMat.Set( *(cfPtr+2), 0.0, 0.0, 0.0, ttcosphi, -ttsinphi,
													   0.0, ttsinphi,  ttcosphi );
					*(PMptr+2) *= tempMat;
					*(PMptr+2) *= tempMat2;
				}
				else {
					(PMptr+2)->Set( *(cfPtr+2), 0.0, 0.0, 
									0.0, *(cfPtr+2), 0.0,
									0.0, 0.0, *(cfPtr+2)  );
				}
			}
			pPtr++;
			cPtr++;
			cfPtr+=3;
			dPtr++;
			PlbPtr+=3;
			ClbPtr+=3;
			PMptr+=3;
		}

		// The PM[i][3] forms a tri-diagonal matrix where the entries are
		//		LinearMapR2's. 
		// Solve for this with the dir[] matrix.
		
		// Phase 1 - use row operations to make the make bi-upper diagonal
		//				with all diagonal elements the identity
		PMptr = PM+1;
		dPtr = dir;
		for (i=0; i<numPoints; i++) {
			// Loop invariants:		PMptr, PMptr+1 point to PM[i][1] and PM[i][2]
			//						PMptr+2, PMptr+3 point to PM[i+1][0] and PM[i+1][1]
			//						PMptr[i][0] is zero already
			//						dPtr points to d[i]
			tempMat = PMptr->Inverse();
			*dPtr = tempMat*(*dPtr);			// Mult row i by PM[i][1] inverse
			PMptr->SetIdentity();
			*(PMptr+1) = tempMat*(*(PMptr+1));
			if (i < numPoints-1) {
				*(dPtr+1) -= (*(PMptr+2))*(*dPtr);
				*(PMptr+3) -= (*(PMptr+2))*(*(PMptr+1));
				(PMptr+2)->SetZero();
			}
			dPtr++;
			PMptr += 3;
		}
		// Phase 2 - solve the tridiagonal systems.
		//		Rather than saving the solution, we will just go ahead and
		//		add the values on to the p[i] values.
		preSoln = dir[numPoints-1];
		PlbPtr = Plocalbasis + (3*(numPoints-1));
		p[numPoints-1].RotateUnitInDirection( preSoln.x*(*PlbPtr) 
											 +preSoln.y*(*(PlbPtr+1))
											 +preSoln.z*(*(PlbPtr+2)));
		maxDeltaSq = preSoln.NormSq();
		PlbPtr -= 3;
		dPtr = dir+(numPoints-2);
		pPtr = p+(numPoints-2);
		PMptr = PM+(3*numPoints-4);			// pointer to PM[numPoints-2][2]
		for (i=numPoints-2; i>=0; i--) {
			// LoopInvariants:   PMptr points to p[i][2]
			//					 dPtr points to d[i]
			//					 pPtr points to p[i]
			//					 PlbPtr points to Plocalbasis[i][0]
			//					 preSoln is solved for (i+1)st value
			preSoln = (*dPtr) - (*PMptr)*preSoln;
			pPtr->RotateUnitInDirection( preSoln.x*(*PlbPtr) 
										+preSoln.y*(*(PlbPtr+1))
										+preSoln.z*(*(PlbPtr+2)) );  // Update p_i
			double thisDeltaSq = preSoln.NormSq();
			if ( maxDeltaSq < thisDeltaSq ) {
				maxDeltaSq = thisDeltaSq;
			}
			dPtr--;
			pPtr--;
			PlbPtr -= 3;
			PMptr -= 3;
		}

		// Decide whether we are close enough to exit loop
		if ( maxDeltaSq < accuracySq ) {
			break;
		}
		if ( maxDeltaSq < bkupaccSq ) {
			accuracySq = bkupaccSq;
			// accuracy = bkupaccuracy;
		}
	}

	delete[] dir;
	delete[] PM;
	delete[] Plocalbasis;
	delete[] Clocalbasis;
}




