//
////////////////////////////////////////////////////////////////
//
// Here are the central routines in the correction:
//   setupCorrection
//   correctCoords
//
//
// They call the qshep3d routines for actually doing the interpolation.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define __DTKCALIBRATION_LOAD_IT__
#include <dtk.h>
#define __NOT_LOADED_DTKSERVICE__
#include <dtk/dtkService.h>
#include <string>
#include <fstream>

#include "tetraUtils.h"
#include "rigidXform.h"

#define MAX(a,b)	(((a)>(b))?(a):(b))
#define MIN(a,b)	(((a)<(b))?(a):(b))

#ifdef DTK_ARCH_WIN32_VCPP
#  define SEPARATOR ";"
#else
#  define SEPARATOR ":"
#endif


class fobcalibration: public dtkCalibration
{
public:
	
	fobcalibration();
	~fobcalibration();
	int initCalibration( dtkConfigService* svc, std::vector<std::string> params );
	int rawCalibrate( float* tracker_data, void* data );
	int calibrate( float* tracker_data, void* data ) { return 0; };

	enum correction_mode
	{
		NO_CORRECTION = 0,
		CALIBRATION_CORRECTION = 1,
		ADHOC_CORRECTION = 2,
		RAW_CORRECTION = 4
	};

	int mode;
	std::string calibration_file_name;
	int NumDataPts;
	Vertex *TrueXYZ;
	Quat *CorrectionRot;
	TetrahedronList TetraList;
	dtkSharedMem* calibration_ctrl;
};

fobcalibration::fobcalibration()
{
	mode = NO_CORRECTION;
	calibration_ctrl = NULL;
}

fobcalibration::~fobcalibration()
{
	if( calibration_ctrl )
		delete calibration_ctrl;
}

int fobcalibration::initCalibration( dtkConfigService* svc, std::vector<std::string> params )
{
	calibration_ctrl = new dtkSharedMem( 4, "calibration_ctrl" );
	if( !calibration_ctrl )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create dtkSharedMem calibration_ctrl\n" );
		return -1;
	}

	int rtn;

	if( !svc )
	{
		dtkMsg.add( DTKMSG_ERROR, "The dtkConfigService is null.\n" );
		return -1;
	}

	for( unsigned int i=0;i<params.size();i++ )
	{
		if( params[i] == "-c" || params[i] == "correction" )
		{
			mode = CALIBRATION_CORRECTION;
			i++;
			if( !params[i].size() )
			{
				dtkMsg.add( DTKMSG_ERROR, "Calibration parameter \"correction\" should be\n"
						"followed by a file name containing the calibration data.\n" );
				return -1;
			}
			else
				calibration_file_name = params[i];
		}
		else if( params[i] == "-a" || params[i] == "adhoc" )
		{
			mode = ADHOC_CORRECTION;
			i++;
			if( !params[i].size() )
			{
				dtkMsg.add( DTKMSG_ERROR, "Calibration parameter \"adhoc\" should be\n"
						"followed by a file name containing the calibration data.\n" );
				return -1;
			}
			else
				calibration_file_name = params[i];
		}
		else if( params[i] == "-n" || params[i] == "nocorrection" )
		{
			mode = NO_CORRECTION;
			i++;
			if( !params[i].size() )
			{
				dtkMsg.add( DTKMSG_ERROR, "Calibration parameter \"nocorrection\" should be\n"
						"followed by a file name containing the calibration data.\n" );
				return -1;
			}
			else
				calibration_file_name = params[i];
		}
		else if( params[i] == "-r" || params[i] == "raw" )
		{
			mode = RAW_CORRECTION;
			i++;
			if( !params[i].size() )
			{
				dtkMsg.add( DTKMSG_ERROR, "Calibration parameter \"raw\" should be\n"
						"followed by a file name containing the calibration data.\n" );
				return -1;
			}
			else
				calibration_file_name = params[i];
		}
		else
		{
			dtkMsg.add( DTKMSG_ERROR, "The parameter passed to the calibration is not\n"
					"valid. Please use one of the accepted parameters.\n" );
			return -1;
		}
	}

	calibration_ctrl->write( &mode );

	std::vector<std::string> paths;
	std::ifstream file;
	std::string file_name;
	tokenize( svc->getCalibrationConfigPath(), paths, SEPARATOR );
	for( unsigned int i=0;i<paths.size();i++ )
	{
		file_name = paths[i];
		if( file_name[file_name.size()-1] != '/' )
			file_name += std::string( "/" );
		file_name += calibration_file_name;
		file.open( file_name.c_str() );
		if( file.is_open() )
		{
			file.close();
			break;
		}
		file_name.clear();
	}

	if( !file_name.size() )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to locate the calibration configuration file\n"
				"%s in path:\n",
				calibration_file_name.c_str() );
		for( unsigned int i=0;i<paths.size();i++ )
		{
			dtkMsg.append( "%s\n", paths[i].c_str() );
		}
		return -1;
	}

	rtn = readCorrectionData (file_name.c_str(),
			&NumDataPts,
			&TrueXYZ,
			&CorrectionRot,
			&TetraList );

	if (rtn)
	{
		dtkMsg.add( DTKMSG_ERROR, "setupCorrection: calibration data file < %s >\n"
				"appears to be formatted improperly.\n", file_name.c_str());
		return -1;
	}

	return 0;
}  // end of setupCalibrationCorrection

int fobcalibration::rawCalibrate (float* tracker_data, void* data)
{
	calibration_ctrl->read( &mode );
	if( mode == NO_CORRECTION )
		return 0;
	else if( mode != CALIBRATION_CORRECTION )
	{
		dtkMsg.add( DTKMSG_ERROR, "The mode specified is not currently supported!\n" );
		return -1;
	}

	double measuredXYZ[3];
	double interpolatedXYZ[3];
	Quat interpolatedCorrectionRot;
	double euler[3];
	static int order[3] = {2, 0, 1};
	double uncorrectedRotMat[4][4];
	Quat uncorrectedRot;
	Quat correctedRot;
	double correctedRotMat[4][4];
	double correctedEulerAngles[3];

	measuredXYZ[0] = tracker_data[0];
	measuredXYZ[1] = tracker_data[1];
	measuredXYZ[2] = tracker_data[2];

	if( tetraInterpReal3 (
			&TetraList,
			measuredXYZ,
			TrueXYZ,
			interpolatedXYZ) )
		return -1;

	tracker_data[0] = interpolatedXYZ[0];
	tracker_data[1] = interpolatedXYZ[1];
	tracker_data[2] = interpolatedXYZ[2];

	tetraInterpQuatSWA (
			&TetraList,
			measuredXYZ,
			CorrectionRot,
			interpolatedCorrectionRot);

	euler[0] = tracker_data[5];
	euler[1] = tracker_data[4];
	euler[2] = tracker_data[3];

	eulerToMat (euler, order, uncorrectedRotMat);

#if 1
	matToQuat (uncorrectedRotMat, uncorrectedRot);
	multQuat (interpolatedCorrectionRot, uncorrectedRot, correctedRot);
	// convert correctedRot from quat back to matrix 
	quatToMat (correctedRot, correctedRotMat);

#if 0
        printf ("uncorrectedRot = %f %f %f %f\n",
                uncorrectedRot[0],
                uncorrectedRot[1],
                uncorrectedRot[2],
                uncorrectedRot[3]);

        printf ("interpolatedCorrectionRot = %f %f %f %f\n",
                interpolatedCorrectionRot[0],
                interpolatedCorrectionRot[1],
                interpolatedCorrectionRot[2],
                interpolatedCorrectionRot[3]);

        printf ("correctedRot = %f %f %f %f\n",
                correctedRot[0],
                correctedRot[1],
                correctedRot[2],
                correctedRot[3]);
#endif

#else
	quatToMat (interpolatedCorrectionRot, correctionMat);
	multMat (uncorrectedRotMat, correctionMat, correctedRotMat);
#endif

	// convert corrected rotation matrix back to euler angles
	matToEulerZXY (correctedRotMat, correctedEulerAngles);

	tracker_data[5] = correctedEulerAngles[0];
	tracker_data[4] = correctedEulerAngles[1];
	tracker_data[3] = correctedEulerAngles[2];

	return 0;
}  // end of calibrationCorrection

// Loader and unloader for DSO
static dtkCalibration *dtkDSO_loader( const char* arg )
{
    return new fobcalibration();
}

static int dtkDSO_unloader( dtkCalibration *fobcalib )
{
    delete fobcalib;
    return DTKDSO_UNLOAD_CONTINUE;
}

