/* 
 * The DIVERSE Toolkit
 * Copyright (C) 2007 Virginia Tech
 * Author: Patrick Shinpaugh
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */

#include <dtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifndef DTK_ARCH_WIN32_VCPP
#  include <unistd.h>
#  include <termios.h>
#  include <sys/ioctl.h>
#endif
#include <signal.h>
#include <fcntl.h>
#include <dtk/dtkService.h>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <algorithm>
#include <errno.h>

#define TEMP_BUFFER_SIZE 1024

using namespace std;

class flockofbirds : public dtkConfigService
{
public:
	flockofbirds( const char* arg );
	virtual ~flockofbirds();
	int serve();

	// Initialize the service - called by the dtkServer
	virtual int init();

	enum trackd_type
	{
		TRACKD_NONE,
		TRACKD_TRACKER,
		TRACKD_CONTROLLER
	};

	enum addressing_mode
	{
		ADDR_NORMAL = 0,
		ADDR_EXPANDED = 1,
		ADDR_SUPER_EXPANDED = 3
	};

	enum data_output_mode
	{
		DATA_NONE,
		DATA_FACTORY_USE_ONLY,
		DATA_ANGLES,
		DATA_MATRIX,
		DATA_POSITION,
		DATA_POSITION_ANGLES,
		DATA_POSITION_MATRIX,
		DATA_POSITION_QUATERNION,
		DATA_QUATERNION
	};

	enum metal_flag
	{
		METAL_OFF,
		METAL_ON,
		METAL_ON_SENSITIVITY,
		METAL_ON_OFFSET,
		METAL_ON_SLOPE,
		METAL_ON_ALPHA
	};

	enum report_rate
	{
		FOB_RATE_1 = 1,
		FOB_RATE_2 = 2,
		FOB_RATE_8 = 8,
		FOB_RATE_32 = 32
	};

	enum sync_type
	{
		SYNC_NONE,
		SYNC_TYPE1,
		SYNC_TYPE2,
		SYNC_TYPE3,
		SYNC_HOST = 8,
		SYNC_TYPE255 = 255
	};

	enum error_mask
	{
		ERR_MASK_NORMAL,
		ERR_MASK_CONTINUE,
		ERR_MASK_IGNORE = 3
	};

	enum transmitter_mode
	{
		TX_MODE_0,
		TX_MODE_1,
		TX_MODE_2
	};

	enum command
	{
		ANGLES,
		ANGLE_ALIGN_1,
		ANGLE_ALIGN_2,
		BORESIGHT,
		BORESIGHT_REMOVE,
		BUTTON_MODE,
		BUTTON_READ,
		FACTORY_TEST,
		FBB_RESET,
		HEMISPHERE,
		MATRIX,
		METAL,
		NEXT_TRANSMITTER,
		OFFSET,
		POINT,
		POSITION,
		POSITION_ANGLES,
		POSITION_MATRIX,
		POSITION_QUATERNION,
		REFERENCE_FRAME_1,
		REFERENCE_FRAME_2,
		REPORT_RATE,
		// RS232_TO_FBB,
		RUN,
		SLEEP,
		STREAM,
		STREAM_STOP,
		SYNC,
		ZOFF,
		XON,
		EXAMINE_BIRD_STATUS,
		EXAMINE_SOFTWARE_REVISION_NUMBER,
		EXAMINE_BIRD_COMPUTER_CRYSTAL_SPEED,
		EXAMINE_POSITION_SCALING,
		CHANGE_POSITION_SCALING,
		EXAMINE_FILTER_STATUS,
		CHANGE_FILTER_STATUS,
		EXAMINE_DC_FILTER_ALPHA_MIN,
		CHANGE_DC_FILTER_ALPHA_MIN,
		EXAMINE_BIRD_MEASUREMENT_RATE_COUNT,
		CHANGE_BIRD_MEASUREMENT_RATE_COUNT,
		EXAMINE_BIRD_MEASUREMENT_RATE,
		CHANGE_BIRD_MEASUREMENT_RATE,
		EXAMINE_DATA_READY_OUTPUT,
		CHANGE_DATA_READY_OUTPUT,
		EXAMINE_DATA_READY_CHARACTER,
		CHANGE_DATA_READY_CHARACTER,
		EXAMINE_ERROR_CODE,
		EXAMINE_ERROR_DETECT_MASK,
		CHANGE_ERROR_DETECT_MASK,
		EXAMINE_DC_FILTER_VM,
		CHANGE_DC_FILTER_VM,
		EXAMINE_DC_FILTER_ALPHA_MAX,
		CHANGE_DC_FILTER_ALPHA_MAX,
		EXAMINE_SUDDEN_OUTPUT_CHANGE_LOCK,
		CHANGE_SUDDEN_OUTPUT_CHANGE_LOCK,
		EXAMINE_SYSTEM_MODEL_IDENTIFICATION,
		EXAMINE_EXPANDED_ERROR_CODE,
		EXAMINE_XYZ_REFERENCE_FRAME,
		CHANGE_XYZ_REFERENCE_FRAME,
		EXAMINE_TRANSMITTER_OPERATION_MODE,
		CHANGE_TRANSMITTER_OPERATION_MODE,
		EXAMINE_FBB_ADDRESSING_MODE,
		EXAMINE_FILTER_LINE_FREQUENCY,
		CHANGE_FILTER_LINE_FREQUENCY,
		EXAMINE_FBB_ADDRESS,
		EXAMINE_HEMISPHERE,
		CHANGE_HEMISPHERE,
		EXAMINE_ANGLE_ALIGN_2,
		CHANGE_ANGLE_ALIGN_2,
		EXAMINE_REFERENCE_FRAME_2,
		CHANGE_REFERENCE_FRAME_2,
		EXAMINE_BIRD_SERIAL_NUMBER,
		EXAMINE_SENSOR_SERIAL_NUMBER,
		EXAMINE_TRANSMITTER_SERIAL_NUMBER,
		EXAMINE_METAL_DETECTION,
		CHANGE_METAL_DETECTION,
		EXAMINE_REPORT_RATE,
		CHANGE_REPORT_RATE,
		EXAMINE_FBB_HOST_RESPONSE_DELAY,
		CHANGE_FBB_HOST_RESPONSE_DELAY,
		EXAMINE_GROUP_MODE,
		CHANGE_GROUP_MODE,
		EXAMINE_FLOCK_SYSTEM_STATUS,
		EXAMINE_FBB_AUTOCONFIGURATION,
		CHANGE_FBB_AUTOCONFIGURATION
	};

	struct bird_state
	{
		bird_state()
		{
			used_by_service = false;
			accessible = false;
			running = false;
			has_sensor = false;
			has_ert = false;
			has_ERT0 = has_ERT1 = has_ERT2 = has_ERT3 = false;
			master = false;
			inited = false;
			error_detected = false;
			host_sync = false;
			addr_expanded = false;
			crt_sync = false;
			has_sync = false;
			sync_mode = SYNC_NONE;
			factory_test = false;
			xoff = false;
			angle_degrees = true;
			metal = false;
			metal_sensitivity = -128;
			metal_offset = -128;
			metal_slope = -128;
			metal_alpha = -128;
			data_mode = DATA_POSITION_ANGLES;
			dtkshm = NULL;
			trackd_shm_type = TRACKD_NONE;
			trackd_key = 0;
			for( int i=0;i<3;i++ )
			{
				sensor_offset[i] = 0.0f;
				sensor_rotation[i] = 0.0f;
				for( int j=0;j<4;j++ )
					tracker_data[i*3+j] = 0.0f;
			}
		}
		bool used_by_service;
		bool accessible;
		bool running;
		bool has_sensor;
		bool has_ert;
		bool has_ERT0;
		bool has_ERT1;
		bool has_ERT2;
		bool has_ERT3;
		bool master;
		bool inited;
		bool error_detected;
		bool host_sync;
		bool addr_expanded;
		bool crt_sync;
		bool has_sync;
		sync_type sync_mode;
		bool factory_test;
		bool xoff;
		bool sleep;
		bool stream;
		bool angle_degrees;
		bool metal;
		char metal_sensitivity;
		char metal_offset;
		char metal_slope;
		char metal_alpha;
		data_output_mode data_mode;
		string dtkshmname;
		dtkSharedMem* dtkshm;
		trackd_type trackd_shm_type;
		int trackd_key;
		float sensor_offset[3];
		float sensor_rotation[3];
		float tracker_data[12];
		char metal_error_value;
	};

	struct device_state
	{
		device_state()
		{
			standalone = false;
			run = false;
			group = false;
			stream = false;
			trackd = false;
			position_scaling = false;
			position_scale = 36.0f;
			baud_rate = 0;
			addr_mode = ADDR_NORMAL;
			data_size = 0;
			max_data_size = 0;
			buffer_size = 0;
			for( int i=0;i<3;i++ )
			{
				transmitter_offset[i] = 0.0f;
				transmitter_rotation[i] = 0.0f;
				for( int j=0;j<4;j++ )
					axis_tracker_data[i*4+j] = 0.0f;
			}
			for( int i=0;i<6;i++ )
			{
				axis_map[i] = i;
				axis_sign[i] = 1.0f;
			}
			transmitter_address = -1;
			transmitter_number = -1;
			buffer = cur_ptr = begin_ptr = end_ptr = NULL;
			diverse_unit = 1.524f;
		}
		bool standalone;
		bool run;
		bool group;
		bool stream;
		bool trackd;
		bool position_scaling;;
		float position_scale;
		int baud_rate;
		addressing_mode addr_mode;
		int data_size;
		int max_data_size;
		string port;
		map<char, bird_state> birds;
		int buffer_size;
		string hemisphere;
		float transmitter_offset[3];
		float transmitter_rotation[3];
		char transmitter_address;
		char transmitter_number;
		char* buffer;
		char* cur_ptr;
		char* begin_ptr;
		char* end_ptr;
		float diverse_unit;
		int axis_map[6];
		float axis_sign[6];
		float axis_tracker_data[12];
	};

private:
	device_state device;
	map<data_output_mode, int> data_sizes;

#ifdef DTK_ARCH_WIN32_VCPP
	OVERLAPPED io_overlapped;
#endif

#ifndef DTK_ARCH_WIN32_VCPP
	dtkVRCOSharedMem* VRCO;
#endif

	// Sets configuration options
	int configureFlock( int addr_size );
	// Check for errors on all birds
	int checkError( const char* msg, char* buffer, int unit = 0, int milli_seconds = 1000 );
	// read values returned for examine commands
	int readValue( char* buffer, int expected_size, int milli_seconds = 1000,
			bool show_errors = false );
	// take raw input and add offset and orientation for both transmitter and sensor
	int rawOffsetAndOrientation( bird_state& bird );
	// Convert position/orientation data from birds to short which will
	// later be converted to a float
	short convertData( const char *data );
	// Clean up all the stuff that needs cleaning
	void clean_up();

	// This function toggles the RTS of the flock - not recommended - 
	// seems to cause problems...
	void resetMaster();

	// Send the specified command
	int sendCommand( const string& cmd, const char& unit = 0 );
	int changeValue( const string& change, const char& unit = 0 );
	int examineValue( const string& examine, const char& unit = 0 );
	string sendRS232toFBB( const string& cur_cmd, const char& unit );

	// Ascension Flock of Birds commands
	// ************************************************************************
	// Switch data mode to ANGLES
	int sendAngles( const char& unit );
	// Change angle offset of sensor with sin and cosine of heading (H),
	// pitch (P), and roll (R)
	int sendAngleAlign1( float sinA, float cosA, float sinE, float cosE,
		float sinR, float cosR, const char& unit = 0 );
	// Change angle offset of sensor with the actual angles of heading (H),
	// pitch (P), and roll (R)
	int sendAngleAlign2( float A, float E, float R, const char& unit = 0 );
	// Cause the specified bird's sensor to be aligned to the bird's reference frame.
	// Any angle align will not be removed.
	int sendBoresight( const char& unit = 0 );
	// Removes the effect of a previous boresight command
	int sendBoresightRemove( const char& unit = 0 );
	// Set whether button values are returned with data records
	// NOTE - only used with the Bird 6D mouse
	int sendButtonMode( bool mode, const char& unit = 0 );
	// Request a signle byte containing the state of buttons
	// NOTE - only used with the Bird 6D mouse
	int sendButtonRead( const char& unit = 0 );
	// Intended only for factory use - DO NOT USE
	int sendFactoryTest()
	{
		dtkMsg.add( DTKMSG_WARNING, "WARNING: This is not allowed!!!!!\n" );
		return 0;
	};
	// Sent to the master, will reset all slave birds
	int sendFBBReset();
	// Set the hemisphere in which the sensor will be operating
	// Valid values are X, Y, Z, +X, +Y, +Z, -X, -Y, -Z, X+, Y+, Z+, X-, Y-, Z-
	int sendHemisphere( const string& hemisphere, const char& unit = 0 );
	// Switch data mode to MATRIX
	int sendMatrix( const char& unit );
	// Specify the metal error configuration
	int sendMetal( bool enable, const char& unit = 0 );
	// Specify the metal error configuration
	int sendMetalSensitivity( char sensitivity, const char& unit = 0 );
	// Specify the metal error configuration
	int sendMetalOffset( char offset, const char& unit = 0 );
	// Specify the metal error configuration
	int sendMetalSlope( char slope, const char& unit = 0 );
	// Specify the metal error configuration
	int sendMetalAlpha( char alpha, const char& unit = 0 );
	// Specify the metal error configuration
	int sendMetal( metal_flag flag, char data, const char& unit = 0 );
	// Requests a single byte representing the metal error if metal mode
	// has been enabled with the metal command or change value
	int sendMetalError( const char& unit = 0 );
	// If there are multiple transmitters, this command is used to turn them on
	// @param address is the FBB with the transmitter to be enabled
	// @param num is the number of the transmitter at the FBB address
	int sendNextTransmitter( char address, char num );
	// Specify the location to retrieve with the offset from the sensor
	int sendOffset( float x, float y, float z, const char& unit = 0 );
	// Request one data record from a bird or if in group mode, request a data record
	// from each running bird.
	int sendPoint( const char& unit = 0 );
	// Switch data mode to POSITION
	int sendPosition( const char& unit );
	// Switch data mode to POSITION/ANGLES
	int sendPositionAngles( const char& unit );
	// Switch data mode to POSITION/MATRIX
	int sendPositionMatrix( const char& unit );
	// Switch data mode to POSITION/QUATERNION
	int sendPositionQuaternion( const char& unit );
	// Switch data mode to QUATERNION
	int sendQuaternion( const char& unit );
	// Change the reference frame of the sensor with sin and cosine of heading (H),
	// pitch (P), and roll (R)
	int sendReferenceFrame1( float sinA, float cosA, float sinE, float cosE,
		float sinR, float cosR, const char& unit = 0 );
	// Change the reference frame of the sensor with the actual angles of heading (H),
	// pitch (P), and roll (R)
	int sendReferenceFrame2( float A, float E, float R, const char& unit = 0 );
	// Change the report rate of the bird data records
	int sendReportRate( report_rate rate, const char& unit = 0 );
	// Sent to master to start the birds flying or wake them from sleeping
	int sendRun();
	// Turns off the transmitter and halts the system
	int sendSleep();
	// Requests that data records be sent continuously.
	// NOTE - can be sent to a standalone flock (a single bird system) or sent
	// to the master in group mode
	int sendStream( const char& unit = 0 );
	// Turns of the stream mode
	int sendStreamStop( const char& unit = 0 );
	// 
	// NOTE - Should only be used in conjunction with  the CRT sync cable shipped
	// with your unit
	int sendSync( sync_type type, const char& unit = 0 );
	// Bird halts transmssion of data at the end of the current output record
	int sendXOff( const char& unit = 0 );
	// Bird resumes transmission of any data records that were pending when it was XOFFed
	int sendXOn( const char& unit = 0 );

	// Ascension Flock of Birds change/examine values
	// ************************************************************************
	// Get status information for the bird
	int examineBirdStatus( const char& unit = 0 );
	// Get the software revision number for the bird
	int examineSoftwareRevisionNumber( const char& unit = 0 );
	// Get the bird crystal speed
	int examineBirdComputerCrystalSpeed( const char& unit = 0 );
	// Get the position scaling
	int examinePositionScaling( const char& unit = 0 );
	// Set the position scaling
	// @param enlarge is false for 36 inch range, and true for 72 inch range
	// NOTE - Do not use with ERT
	int changePositionScaling( bool enlarge, const char& unit = 0 );
	// Get filter status
	int examineFilterStatus( const char& unit = 0 );
	// Set filter status
	int changeFilterStatus( bool ac_narrow, bool ac_wide, bool dc_filter,
		const char& unit = 0 );
	int examineDCFilterAlphaMin( const char& unit = 0 );
	int changeDCFilterAlphaMin( short alpha[7], const char& unit = 0 );
	int examineBirdMeasurementRateCount( const char& unit = 0 );
	int changeBirdMeasurementRateCount( unsigned char count, const char& unit = 0 );
	int examineBirdMeasurementRate( const char& unit = 0 );
	int changeBirdMeasurementRate( unsigned char rate, const char& unit = 0 );
	int examineDataReadyOutput( const char& unit = 0 );
	int changeDataReadyOutput( bool enable, const char& unit = 0 );
	int examineDataReadyCharacter( const char& unit = 0 );
	int changeDataReadyCharacter( char character, const char& unit = 0 );
	int examineErrorCode( const char& unit = 0 );
	int examineErrorDetectMask( const char& unit = 0 );
	int changeErrorDetectMask( error_mask mask, const char& unit = 0 );
	int examineDCFilterVm( const char& unit = 0 );
	int changeDCFilterVm( short levels[7], const char& unit = 0 );
	int examineDCFilterAlphaMax( const char& unit = 0 );
	int changeDCFilterAlphaMax( short alpha[7], const char& unit = 0 );
	// The sudden output change lock prevents update of position and orientation
	// if the system detects a sudden large change in their values
	int examineSuddenOutputChangeLock( const char& unit = 0 );
	int changeSuddenOutputChangeLock( bool enable, const char& unit = 0 );
	// Get the name of the device
	int examineSystemModelIdentification( const char& unit = 0 );
	// Get expanded error information
	int examineExpandedErrorCode( const char& unit = 0 );
	int examineXYZReferenceFrame( const char& unit = 0 );
	int changeXYZReferenceFrame( bool enable, const char& unit = 0 );
	int examineTransmitterOperationMode( const char& unit = 0 );
	int changeTransmitterOperationMode( transmitter_mode mode, const char& unit = 0 );
	int examineFBBAddressingMode( const char& unit = 0 );
	int examineFilterLineFrequency( const char& unit = 0 );
	int changeFilterLineFrequency( const char& unit = 0 );
	int examineFBBAddress( const char& unit = 0 );
	int examineHemisphere( const char& unit = 0 );
	int changeHemisphere( const char* hemisphere, const char& unit = 0 );
	int examineAngleAlign2( const char& unit = 0 );
	int changeAngleAlign2( float A, float E, float R, const char& unit = 0 );
	int examineReferenceFrame2( const char& unit = 0 );
	int changeReferenceFrame2( float A, float E, float R, const char& unit = 0 );
	int examineBirdSerialNumber( const char& unit = 0 );
	int examineSensorSerialNumber( const char& unit = 0 );
	int examineTransmitterSerialNumber( const char& unit = 0 );
	int examineMetalDetection( const char& unit = 0 );
	int changeMetalDetection( short metal[5], const char& unit = 0 );
	int examineReportRate( const char& unit = 0 );
	int changeReportRate( char rate, const char& unit = 0 );
	int examineFBBHostResponseDelay( const char& unit = 0 );
	int changeFBBHostResponseDelay( short delay, const char& unit = 0 );
	int examineGroupMode( const char& unit = 0 );
	int changeGroupMode( bool enable, const char& unit = 0 );
	int examineFlockSystemStatus( const char& unit = 0 );
	int examineFBBAutoConfigure( const char& unit = 0 );
	int changeFBBAutoConfigure( char num, const char& unit = 0 );
};

flockofbirds::flockofbirds( const char* arg )
{
#ifdef DTK_ARCH_WIN32_VCPP
	fd = NULL;
	io_overlapped.Offset = 0;
	io_overlapped.OffsetHigh = 0;
	io_overlapped.hEvent = 0;
#else
	fd = -1;
	VRCO = NULL;
#endif
	device.buffer = device.cur_ptr = device.begin_ptr = device.end_ptr = NULL;
}

int flockofbirds::checkError( const char* msg, char* buffer, int unit, int milli_seconds )
{
	string message;
	if( msg )
		message = msg;
	examineExpandedErrorCode( unit );
	if( readValue( buffer, 2, milli_seconds ) )
	{
		dtkMsg.add( DTKMSG_ERROR, "%s - Unable to determine previous errors for bird %d\n",
				message.c_str(), unit );
		return -2;
	}
	else if( buffer[0] )
	{
		dtkMsg.add( DTKMSG_ERROR, "%s - Error bird #%d: %x %x\n",
				message.c_str(), unit, buffer[0], buffer[1] );
		string error;
		switch( buffer[0] )
		{
			case 1:
			{
				error = "RAM Failure";
				break;
			}
			case 2:
			{
				error = "Non-Volatile Storage Write Failue";
				break;
			}
			case 3:
			{
				error = "PCB Configuration Data Corrupt";
				break;
			}
			case 4:
			{
				error = "Transmitter Configuration Data Corrupt";
				break;
			}
			case 5:
			{
				error = "Sensor Configuration Data Corrupt";
				break;
			}
			case 6:
			{
				error = "Invalid RS232 Command";
				break;
			}
			case 7:
			{
				error = "Not an FBB Master";
				break;
			}
			case 8:
			{
				error = "No Birds Accessible in Device List";
				break;
			}
			case 9:
			{
				error = "Bird is not Initialized";
				break;
			}
			case 10:
			{
				error = "FBB Receive Error - Intra Bird Bus";
				break;
			}
			case 11:
			{
				error = "RS232 Receive Overrun or Framing Error";
				break;
			}
			case 12:
			{
				error = "FBB Receive Error - FBB Host Bus";
				break;
			}
			case 13:
			{
				error = "No FBB Command Response";
				break;
			}
			case 14:
			{
				error = "Invalid FBB Host Command";
				break;
			}
			case 15:
			{
				error = "FBB Run Time Error";
				break;
			}
			case 16:
			{
				error = "Invalid CPU Speed";
				break;
			}
			case 17:
			{
				error = "No Data Error";
				break;
			}
			case 18:
			{
				error = "Illegal Baud Rate Error";
				break;
			}
			case 19:
			{
				error = "Slave Acknowledge Error";
				break;
			}
			case 20:
			case 21:
			case 22:
			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			{
				error = "Fatal Error";
				break;
			}
			case 28:
			{
				error = "CRT Synchronization Error";
				break;
			}
			case 29:
			{
				error = "Transmitter Not Accessible Error";
				break;
			}
			case 30:
			{
				error = "Extended Range Transmitter Not Attached Error";
				break;
			}
			case 31:
			{
				error = "CPU Time Overflow Error";
				break;
			}
			case 32:
			{
				error = "Sensor Saturated Error";
				break;
			}
			case 33:
			{
				error = "Slave Configuration Error";
				break;
			}
			case 34:
			{
				error = "Watch Dog Error";
				break;
			}
			case 35:
			{
				error = "Over Temperature Error";
				break;
			}
			default:
			{
				error = "Invalid Error Code";
			}
		}
		dtkMsg.add( DTKMSG_ERROR, "Error: %s\n", error.c_str() );
		return -1;
	}
	return 0;
}

int flockofbirds::readValue( char* buffer, int expected_size, int milli_seconds, bool show_errors )
{
	int bytes_read = 0;
	int timeout = milli_seconds;
#ifdef DTK_ARCH_WIN32_VCPP
	DWORD read_size = 0;
#endif
	while( bytes_read < expected_size )
	{
		usleep( 1000 );
#ifdef DTK_ARCH_WIN32_VCPP
		ReadFile( fd, buffer + bytes_read, TEMP_BUFFER_SIZE, &read_size, &io_overlapped );
		bytes_read += (int)read_size;
#else
		bytes_read += read( fd, buffer + bytes_read, TEMP_BUFFER_SIZE );
#endif
		timeout--;
		if( timeout < 0 )
		{
			if( show_errors )
				dtkMsg.add( DTKMSG_ERROR, "Timed out after %d milliseconds.\n"
						"Read %d bytes, expected %d bytes.\n",
						milli_seconds, bytes_read, expected_size );
			return expected_size - bytes_read;
		}
	}
	return 0;
}

int flockofbirds::configureFlock( int addr_size )
{
	int sensor_id;
	bool any_error = false;
	bool has_error = false;
	string error;
	vector<string> error_strings;
	vector<vector<string> > config = getConfig();
	dtkMsg.add( DTKMSG_NOTICE, "Examining configuration...\n" );
	for( unsigned int i=0;i<config.size();i++ )
	{
		error_strings.clear();
		has_error = false;
		if( config[i][0] == "trackdtype" )
		{
			int shm_key = 0;
			if( config[i].size() != 4 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			else if( tolower( config[i][2] ) != "tracker" &&
					config[i][2] != "controller" )
			{
				has_error = true;
				error_strings.push_back( "The first parameter to the trackdtype option\n"
						"should be either \"tracker\" or \"controller\". No other types\n"
						"are currently supported.\n" );
			}
			else if( convertStringToNumber( shm_key, config[i][3].c_str() ) )
			{
				has_error = true;
				error_strings.push_back( "The shm_key parameter is not valid. It should\n"
						"be a decimal, octal, or hexadecimal numeric value.\n" );
			}
			if( has_error )
			{
				error = "The trackdtype parmeter is used to specify the shm type and\n"
						"key for any sensors that you desire to use trackd emulation.\n"
						"Syntax:  serviceOption name trackdType sensorID type shmKey\n"
						"Example: serviceOption ";
				error += getName();
				error += " trackdType 2 tracker 4126\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor,\n"
						"type is either \"tracker\" or \"controller\",\n"
						"and shmKey is the System V shared memory key to use.\n";
				error_strings.push_back( error );
			}
			else
			{
				if( config[i][2] == "tracker" )
					device.birds[sensor_id].trackd_shm_type = TRACKD_TRACKER;
				else if( config[i][2] == "controller" )
					device.birds[sensor_id].trackd_shm_type = TRACKD_CONTROLLER;
				device.birds[sensor_id].trackd_key = shm_key;
			}
		}
		else if( config[i][0] == "hemisphere" )
		{
			if( config[i].size() != 2 )
				has_error = true;
			else if( tolower( config[i][1] ) != "forward" &&
					config[i][1] != "rear" && config[i][1] != "upper" &&
					config[i][1] != "lower" && config[i][1] != "left" &&
					config[i][1] != "right" )
				has_error = true;
			if( has_error )
			{
				error = "The hemisphere option specifies the hemisphere of the\n"
						"transmitter in which tracking should be based\n"
						"Syntax: serviceOption name hemisphere direction\n"
						"Example: serviceOption ";
				error += getName();
				error += " hemisphere upper\n"
						"where name was defined with the serviceType definition,\n"
						"and direction is one of \"forward\", \"rear\", \"upper\","
						"\"lower\", \"left\", or \"right\".\n";
				error_strings.push_back( error );
			}
			else
				device.hemisphere = tolower( config[i][1] );
		}
		else if( tolower( config[i][0] ) == "metal" )
		{
			if( config[i].size() != 3 )
				has_error = true;
			else if( tolower( config[i][2] ) != "on" && config[i][2] != "off" &&
					config[i][2] != "yes" && config[i][2] != "no" &&
					config[i][2] != "true" && config[i][2] != "false" )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The metal option should be formatted as:\n"
						"Syntax: serviceOption name metal sensorID boolean\n"
						"Example: serviceOption ";
				error += getName();
				error += " metal 2 on\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set the metal\n"
						"option, and boolean is one of the following values:\n"
						"\"on\", \"off\", \"yes\", \"no\", \"true\", or \"false\".\n";
				error_strings.push_back( error );
			}
			else
			{
				if( config[i][2] == "on" || config[i][2] == "yes" ||
						config[i][2] == "true" )
				{
					device.birds[sensor_id].metal = true;
				}
				else if( config[i][2] == "off" || config[i][2] == "no" ||
						config[i][2] == "false" )
				{
					device.birds[sensor_id].metal = false;
				}
			}
		}
		else if( config[i][0] == "metalsensitivity" )
		{
			int value;
			if( config[i].size() != 3 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			else if( convertStringToNumber( value, config[i][2].c_str() ) )
			{
				has_error = true;
				error = "The metal sensitivity value ";
				error += config[i][2];
				error += " is not formatted\n"
						"properly. Should be a number between 0 and 127.\n";
				error_strings.push_back( error );
			}
			else if( value < 0 || value > 127 )
			{
				has_error = true;
				error = "The value ";
				error += config[i][2];
				error += " passed to metal_sensitivity\n"
						"option. Should be a number between 0 and 127.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				string error = "The metal_sensitivity option should be formatted as:\n"
						"Syntax: serviceOption name metal_sensitivity sensorID value\n"
						"Example: serviceOption ";
				error += getName();
				error += " metal_sensitivity 2 20\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set the metal\n"
						"sensitivity option, and value is a number between 0 and 127\n"
						"representing the sensitivity.\n";
				error_strings.push_back( error );
			}
			else
				device.birds[sensor_id].metal_sensitivity = (char)value;
		}
		else if( config[i][0] == "metaloffset" )
		{
			int value;
			if( config[i].size() != 3 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			else if( convertStringToNumber( value, config[i][2].c_str() ) )
			{
				has_error = true;
				error = "The metal offset value ";
				error += config[i][2];
				error += " is not formatted\n"
						"properly. Should be a number between -127 and 127.\n";
				error_strings.push_back( error );
			}
			else if( value < -127 || value > 127 )
			{
				has_error = true;
				error = "The value ";
				error += config[i][2];
				error += " passed to metal_offset\n"
						"option. Should be a number between -127 and 127.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The metal_offset option should be formatted as:\n"
						"Syntax: serviceOption name metal_offset sensorID value\n"
						"Example: serviceOption ";
				error += getName();
				error += " metal_offset 2 20\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set the metal\n"
						"offset option, and value is a number between 0 and 127\n"
						"representing the offset.\n";
				error_strings.push_back( error );
			}
			else
				device.birds[sensor_id].metal_offset = (char)value;
		}
		else if( config[i][0] == "metalslope" )
		{
			int value;
			if( config[i].size() != 3 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			else if( convertStringToNumber( value, config[i][2].c_str() ) )
			{
				has_error = true;
				error = "The metal slope value ";
				error += config[i][2];
				error += " is not formatted\n"
						"properly. Should be a number between -127 and 127.\n";
				error_strings.push_back( error );
			}
			else if( value < -127 || value > 127 )
			{
				has_error = true;
				error = "The value ";
				error += config[i][2];
				error += " passed to metal_slope\n"
						"option. Should be a number between -127 and 127.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The metal_slope option should be formatted as:\n"
						"Syntax: serviceOption name metal_slope sensorID value\n"
						"Example: serviceOption ";
				error += getName();
				error += " metal_slope 2 20\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set the metal\n"
						"slope option, and value is a number between 0 and 127\n"
						"representing the slope.\n";
				error_strings.push_back( error );
			}
			else
				device.birds[sensor_id].metal_slope = (char)value;
		}
		else if( config[i][0] == "metalalpha" )
		{
			int value = 0;
			if( config[i].size() != 3 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			else if( convertStringToNumber( value, config[i][2].c_str() ) )
			{
				has_error = true;
				error = "The metal alpha value ";
				error += config[i][2];
				error += " is not formatted\n"
						"properly. Should be a number between -127 and 127.\n";
				error_strings.push_back( error );
			}
			else if( value < -127 || value > 127 )
			{
				has_error = true;
				error = "The value ";
				error += config[i][2];
				error += " passed to metal_alpha\n"
						"option. Should be a number between -127 and 127.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The metal_alpha option should be formatted as:\n"
						"Syntax: serviceOption name metal_alpha sensorID value\n"
						"Example: serviceOption ";
				error += getName();
				error += " metal_alpha 2 20\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set the metal\n"
						"alpha option, and value is a number between 0 and 127\n"
						"representing the alpha.\n";
				error_strings.push_back( error );
			}
			else
				device.birds[sensor_id].metal_alpha = (char)value;
		}
		else if( config[i][0] == "sensoroffset" )
		{
			if( config[i].size() != 5 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			else
			{
				float value;
				for( int k=0;k<3;k++ )
				{
					if( convertStringToNumber( value, config[i][k+2].c_str() ) )
					{
						has_error = true;
						error = "The value \"";
						error += config[i][k+2];
						error += "\" is not a valid floating\n"
								"point number in the configuration line\n";
						error_strings.push_back( error );
					}
					else
						device.birds[sensor_id].sensor_offset[k] = value;
				}
			}
			if( has_error )
			{
				error = "The sensoroffset option is used to specify the offset of the\n"
						"output sensor location relative to its real location.\n"
						"Syntax: serviceOption name sensorOffset sensorID X Y Z\n"
						"Example: serviceOption ";
				error += getName();
				error += " sensorOffset 2 1.2 0.3 -0.6\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set,\n"
						"and X, Y, and Z are measured in inches.\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "sensorrotation" )
		{
			if( config[i].size() != 5 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			else
			{
				float value;
				for( int k=0;k<3;k++ )
				{
					if( convertStringToNumber( value, config[i][k+2].c_str() ) )
					{
						has_error = true;
						error = "The value \"";
						error += config[i][k+2];
						error += "\" is not a valid floating\n"
								"point number in the configuration line\n";
						error_strings.push_back( error );
					}
					device.birds[sensor_id].sensor_rotation[k] = value;
				}
			}
			if( has_error )
			{
				error = "The sensorrotation option is used to specify the rotation\n"
						"of the sensor relative to the transmitter\n"
						"Syntax: serviceOption name sensorRotation sensorID H P R\n"
						"Example: serviceOption ";
				error += getName();
				error += " sensorRotation 3 -90.0 21.5 15.0\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set,\n"
						"and H is the heading, P is the pitch, and R is\n"
						"the roll measured in degrees.\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "transmitteroffset" )
		{
			if( config[i].size() != 4 )
				has_error = true;
			else
			{
				float value;
				for( int k=0;k<3;k++ )
				{
					if( convertStringToNumber( value, config[i][k+1].c_str() ) )
					{
						has_error = true;
						error = "The value \"";
						error += config[i][k+1];
						error += "\" is not a valid floating\n"
								"point number in the configuration line\n";
						error_strings.push_back( error );
					}
					device.transmitter_offset[k] = value;
				}
			}
			if( has_error )
			{
				error = "The transmitter offset option is used to specify the\n"
						"offset from the transmitter to the origin.\n"
						"Syntax: serviceOption name transmitterOffset X Y Z\n"
						"Example: serviceOption ";
				error += getName();
				error += " transmitterOffset 2.1 -35.0 14.3\n"
						"where name was defined with the serviceType definition,\n"
						"and X, Y, and Z are measured in inches.\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "transmitterrotation" )
		{
			if( config[i].size() != 4 )
				has_error = true;
			else
			{
				float value;
				for( int k=0;k<3;k++ )
				{
					if( convertStringToNumber( value, config[i][k+1].c_str() ) )
					{
						has_error = true;
						error = "The value \"";
						error += config[i][k+1];
						error += "\" is not a valid floating\n"
								"point number in the configuration line\n";
						error_strings.push_back( error );
					}
					device.transmitter_rotation[k] = value;
				}
			}
			if( has_error )
			{
				error = "The transmitterRotation option is used to specify the rotation\n"
						"of the axes about the origin relative to the transmitter.\n"
						"Syntax: serviceOption name transmitterRotation H P R\n"
						"Example: serviceOption ";
				error += getName();
				error += " transmitterRotation 12.0 30.0 -20.5\n"
						"where name was defined with the serviceType definition,\n"
						"and H is the heading, P is the pitch, and R is\n"
						"the roll measured in degrees.\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "datamode" )
		{
			if( config[i].size() != 3 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			if( tolower( config[i][2] ) == "angles" )
			{
				device.birds[sensor_id].data_mode = DATA_ANGLES;
			}
			else if( config[i][2] == "matrix" )
			{
				device.birds[sensor_id].data_mode = DATA_MATRIX;
			}
			else if( config[i][2] == "quaternion" )
			{
				device.birds[sensor_id].data_mode = DATA_QUATERNION;
			}
			else if( config[i][2] == "position" )
			{
				device.birds[sensor_id].data_mode = DATA_POSITION;
			}
			else if( config[i][2] == "positionangles" )
			{
				device.birds[sensor_id].data_mode = DATA_POSITION_ANGLES;
			}
			else if( config[i][2] == "positionmatrix" )
			{
				device.birds[sensor_id].data_mode = DATA_POSITION_MATRIX;
			}
			else if( config[i][2] == "positionquaternion" )
			{
				device.birds[sensor_id].data_mode = DATA_POSITION_QUATERNION;
			}
			else
				has_error = true;
			if( has_error )
			{
				error = "The datamode option specifies the type of data to be output\n"
						"by the flock of birds for a particular sensor.\n"
						"Syntax: serviceOption name datamode sensorID mode\n"
						"Example: serviceOption i";
				error += getName();
				error += " datamode 2 positionangles\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set and\n"
						"mode is one of the following values: \"angles\", \"matrix\"\n"
						"\"quaternion\", \"position\", \"positionangles\",\n"
						"\"positionmatrix\", or \"positionquaternion\"\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "angleunits" )
		{
			if( config[i].size() != 3 )
				has_error = true;
			else if( ( tolower( config[i][2] ) != "degrees" &&
					config[i][2] != "radians" ) )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The angleUnits option specifies whether the angle data\n"
						"is output as degrees or radians.\n"
						"Syntax: serviceOption name angles sensorID ( degrees | radians )\n"
						"Example: serviceOption ";
				error += getName();
				error += " angles 2 radians\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set, and\n"
						"the unit type is set to either \"degrees\" or \"radians\".\n"
						"The default value for angle units is \"degrees\"\n";
				error_strings.push_back( error );
			}
			else if( config[i][2] == "degrees" )
				device.birds[sensor_id].angle_degrees = true;
			else
				device.birds[sensor_id].angle_degrees = false;
		}
		else if( config[i][0] == "dtkshm" )
		{
			if( config[i].size() != 3 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The dtkShm option specifies the name of the dtkSharedMem\n"
						"segment to be created and used for data output.\n"
						"Syntax: serviceOption name dtkShm sensorID shmName"
						"Example: serviceOption ";
				error += getName();
				error += "dtkShm 2 head"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set and\n"
						"shmName is a valid name for a file.\n";
				error_strings.push_back( error );
			}
			else
				device.birds[sensor_id].dtkshmname = config[i][2];
		}
		else if( config[i][0] == "srtscaleexpand" )
		{
			if( config[i].size() != 2 )
				has_error = true;
			else if( tolower( config[i][1] ) != "on" && config[i][1] != "off" &&
					config[i][1] != "yes" && config[i][1] != "no" &&
					config[i][1] != "true" && config[i][1] != "false" )
				has_error = true;
			if( has_error )
			{
				error = "The srtScaleExpand option specifies whether to expand the scale\n"
						"factor used to compute the position of the sensor relative\n"
						"to the transmitter for short range transmitters (SRT).\n"
						"This option is ignored when using an extended range\n"
						"transmitter (ERT).\n"
						"Syntax: serviceOption name srtScaleExpand boolean\n"
						"Example: serviceOption ";
				error += getName();
				error += " srtScaleExpand on\n"
						"where name was defined with the serviceType definition,\n"
						"and boolean is one of the following values:\n"
						"\"on\", \"off\", \"yes\", \"no\", \"true\", or \"false\".\n";
				error_strings.push_back( error );
			}
			else
			{
				if( config[i][1] == "on" || config[i][1] == "yes" ||
						config[i][1] == "true" )
				{
					device.position_scaling = true;
					device.position_scale = 72.0f;
				}
				else if( config[i][1] == "off" || config[i][1] == "no" ||
						config[i][1] == "false" )
				{
					device.position_scaling = true;
					device.position_scale = 36.0f;
				}
			}
		}
		else if( config[i][0] == "transmitter" )
		{
			if( config[i].size() != 2 && config[i].size() != 3 )
				has_error = true;
			else if( convertStringToNumber( sensor_id, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The sensorID parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The sensorID should be an integer representing the address\n"
						"of the bird.\n";
				error_strings.push_back( error );
			}
			else if( sensor_id < 0 || sensor_id >= addr_size )
			{
				has_error = true;
				error = "The sensorID is not within the valid range of 0 to ";
				error += addr_size;
				error += ".\n"
						"Verify that all birds are using the correct addressing mode\n"
						"and that the referenced bird has the correct address. Birds\n"
						"should have contiguous addresses starting with address 1.\n";
				error_strings.push_back( error );
			}
			int value = 0;
			if( config[i].size() == 3 )
			{
				if( convertStringToNumber( value, config[i][2].c_str() ) ||
						( value != 0 && value != 1 && value != 2 && value != 3 ) )
				{
					has_error = true;
					string error = "The transmitterAddress ";
					error += config[i][2];
					error += " is not a valid value.\n"
							"The transmitterAddress should be an integer representing the\n"
							"address of the transmitter and must be between 0 and 3.\n";
					error_strings.push_back( error );
				}
			}
			if( has_error )
			{
				error = "The transmitter option specifies which transmitter to use.\n"
						"Syntax serviceOption name transmitter sensorID transmitterNumber\n"
						"Example: serviceOption ";
				error += getName();
				error += " transmitter 1 0\n"
						"where name was defined with the serviceType definition,\n"
						"sensorID is the bird address of the sensor to set, and\n"
						"transmitterNumber is the transmitter number of the\n"
						"transmitter on the bird. For an ERC the transmitter number\n"
						"can be in the range 0 - 3. For a standard range transmitter\n"
						"attached to a bird, the transmitter number must be 0. If the\n"
						"transmitter number is omitted, the transmitter number will\n"
						"be set to 0\n";
				error_strings.push_back( error );
			}
			else
			{
				device.transmitter_address = sensor_id;
				device.transmitter_number = value;
			}
		}
		else if( config[i][0] == "axismap" )
		{
			if( config[i].size() != 7 )
				has_error = true;
			else
			{
				for( int j=0;j<6;j++ )
				{
					if( tolower( config[i][j+1] ) == "-x" )
					{
						device.axis_map[j] = 0;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+x" )
					{
						device.axis_map[j] = 0;
						device.axis_sign[j] = 1.0f;
					}
					else if( config[i][j+1] == "-y" )
					{
						device.axis_map[j] = 1;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+y" )
					{
						device.axis_map[j] = 1;
						device.axis_sign[j] = 1.0f;
					}
					else if( config[i][j+1] == "-z" )
					{
						device.axis_map[j] = 2;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+z" )
					{
						device.axis_map[j] = 2;
						device.axis_sign[j] = 1.0f;
					}
					else if( config[i][j+1] == "-h" )
					{
						device.axis_map[j] = 3;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+h" )
					{
						device.axis_map[j] = 3;
						device.axis_sign[j] = 1.0f;
					}
					else if( config[i][j+1] == "-p" )
					{
						device.axis_map[j] = 4;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+p" )
					{
						device.axis_map[j] = 4;
						device.axis_sign[j] = 1.0f;
					}
					else if( config[i][j+1] == "-r" )
					{
						device.axis_map[j] = 5;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+r" )
					{
						device.axis_map[j] = 5;
						device.axis_sign[j] = 1.0f;
					}
					else
					{
						has_error = true;
						error = "The value \"";
						error += config[i][j+1];
						error += "\" is not a valid floating\n"
								"point number in the configuration line\n";
						error_strings.push_back( error );
					}
				}
			}
			if( has_error )
			{
				error = "The axisMap option specifies the orientation of each axis.\n"
						"Syntax: serviceOption name axisMap orientX orientY orientZ\n"
						"Example: serviceOption ";
				error += getName();
				error += " axisMap -y -x +z\n"
						"where name was defined with the serviceType definition,\n"
						"and orientX, orientY, and orientZ are \"-x\", \"+x\", \"-y\",\n"
						"\"+y\", \"-z\", \"+z\"\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "diverseunit" )
		{
			float value;
			if( config[i].size() != 2 )
				has_error = true;
			else if( convertStringToNumber( value, config[i][1].c_str() ) )
			{
				has_error = true;
				error = "The units ";
				error += config[i][1];
				error += " is not a valid numeric value.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The diverseUnit option is used to specify the value\n"
						"for the DIVERSE unit in meters.\n"
						"Syntax: serviceOption name diverseUnit units\n"
						"Example: serviceOption ";
				error += getName();
				error += " diverseUnit 1.524\n"
						"where name was defined with the serviceType definition,\n"
						"and units are the diverse unit size in meters.\n";
				error_strings.push_back( error );
			}
			else
				device.diverse_unit = value;
		}
		else
		{
			has_error = true;
			string error = "The option ";
			error += config[i][0];
			error += " is not a valid configuration option\n";
			error_strings.push_back( error );
		}
		if( has_error )
		{
			any_error = true;
			dtkMsg.add( DTKMSG_ERROR, "error in line: " );
			for( unsigned int j=0;j<config[i].size();j++ )
			{
				dtkMsg.append( "%s ", config[i][j].c_str() );
			}
			dtkMsg.append( "\n" );
			for( unsigned int j=0;j<error_strings.size();j++ )
			{
				dtkMsg.append( error_strings[j].c_str() );
			}
		}
	}

	if( any_error )
		return DTKSERVICE_ERROR;

	return DTKSERVICE_CONTINUE;;
}

int flockofbirds::init()
{
	data_sizes[DATA_ANGLES] = 3;
	data_sizes[DATA_MATRIX] = 9;
	data_sizes[DATA_POSITION] = 3;
	data_sizes[DATA_POSITION_ANGLES] = 6;
	data_sizes[DATA_POSITION_MATRIX] = 12;
	data_sizes[DATA_POSITION_QUATERNION] = 7;
	data_sizes[DATA_QUATERNION] = 4;

	if( getCalibrator() )
	{
		if( getCalibrator()->initCalibration( this, getCalibratorParams() ) )
		{
			dtkMsg.add( DTKMSG_ERROR, "The dtkCalibration DSO %s had trouble with\n"
					"the parameters passed to it.\n", getCalibratorName().c_str() );
			return DTKSERVICE_ERROR;
		}
	}

	device.port = getPort();
	dtkMsg.add( DTKMSG_NOTICE, "Using port: %s\n", device.port.c_str() );
	device.baud_rate = getBaudFlag();
	dtkMsg.add( DTKMSG_NOTICE, "Using baud rate: %s\n", getBaud().c_str() );
#ifdef DTK_ARCH_WIN32_VCPP
	fd = CreateFile( device.port.c_str(), GENERIC_READ | GENERIC_WRITE,
		0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0 );
	if( fd == INVALID_HANDLE_VALUE )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
			"flockofbirds::flockofbirds()"
			" failed: CreateFile(\"%s%s%s\","
			"GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,0,0) failed.\n",
			dtkMsg.color.tur, device.port.c_str(),
			dtkMsg.color.end );
		return DTKSERVICE_ERROR;
	}
	if( !SetupComm( fd, 4096, 4096 ) )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
			"flockofbirds::flockofbirds()"
			" failed: SetupComm(%p,64,64).\n",
			fd );
		return DTKSERVICE_ERROR;
	}
	DCB commDCB;
	if( !GetCommState( fd, &commDCB ) )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
			"flockofbirds::flockofbirds()"
			" failed: GetCommState(%p,%p).\n",
			fd, &commDCB );
		return DTKSERVICE_ERROR;
	}

	commDCB.DCBlength = sizeof( DCB );
	commDCB.BaudRate = device.baud_rate;
	commDCB.fBinary = true;
	commDCB.fParity = false;
	commDCB.fOutxCtsFlow = false;
	commDCB.fOutxDsrFlow = false;
	commDCB.fDtrControl = DTR_CONTROL_ENABLE;
	commDCB.fDsrSensitivity = false;
	commDCB.fTXContinueOnXoff = false;
	commDCB.fOutX = false;
	commDCB.fInX = false;
	commDCB.fErrorChar = false;
	commDCB.fNull = false;
	commDCB.fRtsControl = RTS_CONTROL_DISABLE;
	commDCB.fAbortOnError = false;
	commDCB.XonLim = 2048;
	commDCB.XoffLim = 512;
	commDCB.ByteSize = 8;
	commDCB.Parity = NOPARITY;
	commDCB.StopBits = ONESTOPBIT;
	commDCB.XonChar = 17;
	commDCB.XoffChar = 19;
	commDCB.ErrorChar = false;
	commDCB.EofChar = false;
	commDCB.EvtChar = false;

	if( !SetCommState( fd, &commDCB ) )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
			"flockofbirds::flockofbirds()"
			" failed: SetCommState(%p,%p).\n",
			fd, &commDCB );
		return DTKSERVICE_ERROR;
	}

	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts( fd, &timeouts );
#else
	struct termios terminfo;
	fd = open( device.port.c_str(),  O_RDWR | O_NDELAY);
	if( fd < 0 )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
			"flockofbirds::flockofbirds()"
			" failed: open(\"%s%s%s\","
			"O_RDWR|O_NDELAY) failed.\n",
			dtkMsg.color.tur, device.port.c_str(),
			dtkMsg.color.end );
		return DTKSERVICE_ERROR;
	}
	terminfo.c_iflag = 0;
	terminfo.c_oflag = 0;
	terminfo.c_lflag = 0;
#  ifdef IRIX
	terminfo.c_cflag = CS8 | CREAD | CLOCAL | CNEW_RTSCTS;
	terminfo.c_ispeed = terminfo.c_ospeed = device.baud_rate;
#  else /* Linux and what */
	terminfo.c_cflag = CS8 | CREAD | CLOCAL;
	cfsetospeed( &terminfo, device.baud_rate );
	cfsetispeed( &terminfo, device.baud_rate );
#  endif

	for ( int i=0;i<NCCS;i++ )
		terminfo.c_cc[i] = 0;

	if( tcsetattr( fd, TCSANOW, &terminfo ) == -1 )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
			"FOB::FOB() failed: "
			"tcsetattr(%d,TCSANOW) failed "
			"for device file %s%s%s.",
			fd, dtkMsg.color.tur, device.port.c_str(),
			dtkMsg.color.end );
		return DTKSERVICE_ERROR;
	}

	if( tcflush( fd, TCIOFLUSH ) == -1 )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
			"FOB::FOB() failed: "
			"tcflush(%d,TCIOFLUSH) failed "
			"for device file %s%s%s.",
			fd, dtkMsg.color.tur, device.port.c_str(),
			dtkMsg.color.end );
		return DTKSERVICE_ERROR;
	}
#endif // DTK_ARCH_WIN32_VCPP

	usleep( 600000 );
	resetMaster();
	usleep( 600000 );
	sendFBBReset();
	usleep( 600000 );

	int addr_size = 0;
	char temp_buf[TEMP_BUFFER_SIZE];

	examineFBBAddressingMode();

	if( readValue( temp_buf, 1 ) )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to get a response from the master bird. Make\n"
				"sure that the bird is in fly mode and restart the service.\n" );
		return DTKSERVICE_ERROR;
	}

	device.addr_mode = (addressing_mode)temp_buf[0];
	dtkMsg.add( DTKMSG_NOTICE, "Addressing mode: " );
	switch( device.addr_mode )
	{
		case ADDR_NORMAL:
		{
			dtkMsg.append( "NORMAL\n" );
			addr_size = 14;
			break;
		}
		case ADDR_EXPANDED:
		{
			dtkMsg.append( "EXPANDED\n" );
			addr_size = 30;
			break;
		}
		case ADDR_SUPER_EXPANDED:
		{
			dtkMsg.append( "SUPER_EXPANDED\n" );
			addr_size = 126;
			break;
		}
		default:
		{
			dtkMsg.add( DTKMSG_ERROR, "The value %d returned for examineFBBAddressingMode is invalid\n", device.addr_mode );
			return DTKSERVICE_ERROR;
		}
	}

	dtkMsg.add( DTKMSG_NOTICE, "Addressable size: %d\n", addr_size );

	int error;
	if( ( error = configureFlock( addr_size ) ) )
		return error;
	else
		dtkMsg.append( "PASSED\n" );

#ifndef DTK_ARCH_WIN32_VCPP
	if( device.trackd )
		VRCO = new dtkVRCOSharedMem;
#endif

	int last_bird = 0;
	map<char, bird_state>::iterator itr = device.birds.begin();
	for( ;itr!=device.birds.end();itr++ )
	{
		if( itr->first > last_bird )
			last_bird = itr->first;
		itr->second.used_by_service = true;
	}

	examineFBBAddress();
	if( readValue( temp_buf, 1 ) )
		return DTKSERVICE_ERROR;
	if( temp_buf[0] < 0 || temp_buf[0] > 1 )
	{
		dtkMsg.add( DTKMSG_ERROR, "The FBB address of the bird attached through the serial\n"
				"cable to the computer should be address 0 if standalone\n"
				"or address 1 for a flock of birds.\n" );
		return DTKSERVICE_ERROR;
	}
	else if( temp_buf[0] == 0 )
		device.standalone = true;

	if( device.standalone && last_bird > 0 )
	{
		dtkMsg.add( DTKMSG_ERROR, "The FBB address of the bird attached to the\n"
				"computer through the serial cable should be address 0 only\n"
				"if the flock is being used in standalone mode. If you are\n"
				"using standlone mode, check your configuration file to verify\n"
				"that the sensorID for any options are set only for address 0.\n" );
		return DTKSERVICE_ERROR;
	}

	examineFlockSystemStatus();
	if( readValue( temp_buf, addr_size ) )
		return DTKSERVICE_ERROR;

	char stations = 0;
	unsigned char test;
	bool val;
	bool has_error = false;
	dtkMsg.add( DTKMSG_NOTICE, "Flock System Status\n" );
	for( int i=0;i<addr_size;i++ )
	{
		int index = i + ( device.standalone ? 0 : 1 );
		if( index > last_bird )
			break;
		dtkMsg.append( "BIRD ADDRESS: %d\n", index );
		test = 0x80;
		for( int j=0;j<8;j++ )
		{
			val = temp_buf[i] & test;
			dtkMsg.append( "%s  ", val ? "YES" : "NO " );
			switch( j )
			{
				case 0:
				{
					device.birds[index].accessible = val;
					if( val )
						stations++;
					else if( last_bird < index )
					{
						has_error = true;
						dtkMsg.add( DTKMSG_ERROR, "The bird with address %d is not accessible. This\n"
								"implies that the bird is either not attached to the\n"
								"flock or that it is not in fly mode. Verify that the\n"
								"bird is in fly mode and not in standby mode. All birds\n"
								"starting with address 1 up to the last address used\n"
								"in the configuration file must be in fly mode for this\n"
								"service to function.\n", index );
					}
					dtkMsg.append( "Accessible" );
					break;
				}
				case 1:
				{
					device.birds[index].running = val;
					dtkMsg.append( "Running" );
					break;
				}
				case 2:
				{
					device.birds[index].has_sensor = val;
					dtkMsg.append( "Has sensor" );
					break;
				}
				case 3:
				{
					device.birds[index].has_ert = val;
					dtkMsg.append( "Has ERT" );
					break;
				}
				case 4:
				{
					device.birds[index].has_ERT3 = val;
					dtkMsg.append( "ERT#3 present" );
					break;
				}
				case 5:
				{
					device.birds[index].has_ERT2 = val;
					dtkMsg.append( "ERT#2 present" );
					break;
				}
				case 6:
				{
					device.birds[index].has_ERT1 = val;
					dtkMsg.append( "ERT#1 present" );
					break;
				}
				case 7:
				{
					device.birds[index].has_ERT0 = val;
					dtkMsg.append( "ERT#0 or standard range transmitter present" );
					break;
				}
			}
			dtkMsg.append( "\n" );
			test = test >> 1;
		}
		dtkMsg.append( "\n" );
	}
	if( has_error )
		return DTKSERVICE_ERROR;

	checkError( "examineFlockSystemStatus", temp_buf, 1 );

	if( stations != last_bird + ( device.standalone ? 1 : 0 ) ||
			( stations != 1 && device.standalone ) )
	{
		dtkMsg.add( DTKMSG_ERROR, "The number of birds implied by the configuration\n"
				"file %d does not match the number of birds %d found.\n", last_bird,
				stations );
		return DTKSERVICE_ERROR;
	}

	if( device.transmitter_address == -1 )
	{
		dtkMsg.add( DTKMSG_ERROR, "The transmitter must be set. Please add\n"
				"the transmitter option to your configuration file\n"
				"for this service.\n" );
		return DTKSERVICE_ERROR;
	}
	else
	{
		bool has_error = false;
		switch( device.transmitter_number )
		{
			case 0:
			{
				if( !device.birds[device.transmitter_address].has_ERT0 )
					has_error = true;
				break;
			}
			case 1:
			{
				if( !device.birds[device.transmitter_address].has_ert )
					has_error = true;
				else if( !device.birds[device.transmitter_address].has_ERT1 )
					has_error = true;
				break;
			}
			case 2:
			{
				if( !device.birds[device.transmitter_address].has_ert )
					has_error = true;
				else if( !device.birds[device.transmitter_address].has_ERT2 )
					has_error = true;
				break;
			}
			case 3:
			{
				if( !device.birds[device.transmitter_address].has_ert )
					has_error = true;
				else if( !device.birds[device.transmitter_address].has_ERT3 )
					has_error = true;
				break;
			}
			default:
				has_error = true;
		}
		if( has_error )
		{
			dtkMsg.add( DTKMSG_ERROR, "The transmitter option specified the\n"
					"transmitter address %d and the transmitter number %d\n"
					"which are not available on the specified bird.\n"
					"Please verify that there is a transmitter attached to\n"
					"the bird, that you have the correct values, and update\n"
					"your configuration file. For a transmitter attached\n"
					"to a standalone bird, the transmitter number should\n"
					"be 0.\n", device.transmitter_address, device.transmitter_number );
			return DTKSERVICE_ERROR;
		}
		else
		{
			sendNextTransmitter( device.transmitter_address, device.transmitter_number );
			usleep( 10000 );
		}
	}

	if( device.birds[device.transmitter_address].has_ert )
	{
		device.position_scale = 144.0f;
	}
/*
	usleep( 100000 );
	changeGroupMode( true );
	checkError( "changeGroupMode", temp_buf, 1 );
*/
	usleep( 600000 );
	changeFBBAutoConfigure( stations );
	usleep( 600000 );

	readValue( temp_buf, 100000 );
	checkError( "changeFBBAutoConfigure", temp_buf, 1 );

	itr = device.birds.begin();
	for( ;itr!=device.birds.end();itr++ )
	{
		dtkMsg.add( DTKMSG_NOTICE, "Bird: %d\n", itr->first );
		examineSystemModelIdentification( itr->first );
		if( readValue( temp_buf, 10 ) )
			return DTKSERVICE_ERROR;
		temp_buf[10] = '\0';
		dtkMsg.add( DTKMSG_NOTICE, "SystemModelIdentification: %s\n", temp_buf );
		checkError( "examineSystemModelIdentification", temp_buf, itr->first );

		examineSoftwareRevisionNumber( itr->first );
		if( readValue( temp_buf, 2 ) )
			return DTKSERVICE_ERROR;
		dtkMsg.add( DTKMSG_NOTICE, "SoftwareRevisionNumber: %d.%d\n",
				(char)temp_buf[0], (char)temp_buf[1] );
		checkError( "examineSoftwareRevisionNumber", temp_buf, itr->first );

		examineBirdComputerCrystalSpeed( itr->first );
		if( readValue( temp_buf, 2 ) )
			return DTKSERVICE_ERROR;
		dtkMsg.add( DTKMSG_NOTICE, "BirdComputerCrystalSpeed: %d\n", (char)temp_buf[0] );
		checkError( "examineBirdComputerCrystalSpeed", temp_buf, itr->first );

		examineBirdMeasurementRate( itr->first );
		if( readValue( temp_buf, 2 ) )
			return DTKSERVICE_ERROR;
		dtkMsg.add( DTKMSG_NOTICE, "BirdMeasurementRate: %f\n",
				(float)( *(short*)temp_buf ) / 256.0f );
		checkError( "examineBirdMeasurementRate", temp_buf, itr->first );

		examineBirdSerialNumber( itr->first );
		if( readValue( temp_buf, 2 ) )
			return DTKSERVICE_ERROR;
		dtkMsg.add( DTKMSG_NOTICE, "BirdSerialNumber: %d\n", *(short*)temp_buf );
		checkError( "examineBirdSerialNumber", temp_buf, itr->first );

/*		if( itr->second.has_sensor )
		{
			examineSensorSerialNumber( itr->first );
			if( readValue( temp_buf, 2 ) )
				dtkMsg.add( DTKMSG_ERROR, "examineSensorSerialNumber: No data available\n" );
			else
			dtkMsg.add( DTKMSG_NOTICE, "SensorSerialNumber: %d\n", *(short*)temp_buf );
			checkError( "examineSensorSerialNumber", temp_buf, itr->first );
		}

		if( itr->second.has_ert )
		{
			examineTransmitterSerialNumber( itr->first );
			if( readValue( temp_buf, 2 ) )
				dtkMsg.add( DTKMSG_ERROR, "examineTransmitterSerialNumber: No data available\n" );
			else
			{
				dtkMsg.add( DTKMSG_NOTICE, "TransmitterSerialNumber: %d\n",
						*(short*)temp_buf );
				checkError( "examineTransmitterSerialNumber", temp_buf, itr->first );
			}
		}*/

		if( itr->second.has_sensor )
		{
			if( device.hemisphere.size() )
			{
				sendHemisphere( device.hemisphere, itr->first );
				usleep( 10000 );
				checkError( "sendHemisphere", temp_buf, itr->first );
			}

			if( device.position_scale != 144 )
			{
				changePositionScaling( device.position_scaling, itr->first );
				usleep( 10000 );
				checkError( "changePositionScaling", temp_buf, itr->first );
			}

/*			sendReferenceFrame2( device.transmitter_rotation[0],
					device.transmitter_rotation[1],
					device.transmitter_rotation[2], itr->first );
			usleep( 10000 );
			checkError( "sendReferenceFrame2", temp_buf, itr->first );

			changeXYZReferenceFrame( true, itr->first );
			usleep( 10000 );
			checkError( "changeXYZReferenceFrame", temp_buf, itr->first );

			sendOffset( itr->second.sensor_offset[0], itr->second.sensor_offset[1],
					itr->second.sensor_offset[2], itr->first );
			usleep( 10000 );
			checkError( "sendOffset", temp_buf, itr->first );

			sendAngleAlign2( itr->second.sensor_rotation[0],
					itr->second.sensor_rotation[1],
					itr->second.sensor_rotation[2], itr->first );
			usleep( 10000 );
			checkError( "sendAngleAlign2", temp_buf, itr->first );
*/
			if( itr->second.dtkshmname.size() )
			{
				itr->second.dtkshm = new dtkSharedMem( 4 * data_sizes[itr->second.data_mode],
						itr->second.dtkshmname.c_str() );
				if( !itr->second.dtkshm )
				{
					dtkMsg.add( DTKMSG_ERROR, "Unable to create dtkSharedMem %s "
							"of size %d.\n",
							itr->second.dtkshmname.c_str(),
							4 * data_sizes[itr->second.data_mode] );
					return DTKSERVICE_ERROR;
				}
			}
		}

		usleep( 10000 );
		examineBirdStatus( itr->first );
		if( readValue( temp_buf, 2 ) )
			return DTKSERVICE_ERROR;

		unsigned short test;
		bool val;
		bool expectError = false;
		char mode = 0;
		bird_state unit_status;

		dtkMsg.add( DTKMSG_NOTICE, "Bird %d status\n", itr->first );
		test = 0x8000;
		for( int i=0;i<16;i++ )
		{
			val = *((short*)temp_buf) & test;
			if( i < 11 || i > 14 )
				dtkMsg.append( "%s  ", val ? "YES" : "NO " );
			else if( i == 11 )
				dtkMsg.append( "MODE " );
			switch( i )
			{
				case 0:
				{
					unit_status.master = val;
					dtkMsg.append( "Master" );
					break;
				}
				case 1:
				{
					unit_status.inited = val;
					dtkMsg.append( "Initialized" );
					break;
				}
				case 2:
				{
					unit_status.error_detected = val;
					dtkMsg.append( "Error detected" );
					if( val )
					{
						expectError = true;
					}
					break;
				}
				case 3:
				{
					unit_status.running = val;
					dtkMsg.append( "Running" );
					break;
				}
				case 4:
				{
					unit_status.host_sync = val;
					dtkMsg.append( "Host sync mode" );
					break;
				}
				case 5:
				{
					unit_status.addr_expanded = val;
					dtkMsg.append( "Expanded address mode" );
					break;
				}
				case 6:
				{
					unit_status.crt_sync = val;
					dtkMsg.append( "CRT sync" );
					break;
				}
				case 7:
				{
					unit_status.has_sync = val;
					dtkMsg.append( "No sync modes" );
					break;
				}
				case 8:
				{
					dtkMsg.append( "Factory test" );
					unit_status.factory_test = val;
					break;
				}
				case 9:
				{
					dtkMsg.append( "XOFF" );
					unit_status.xoff = val;
					break;
				}
				case 10:
				{
					dtkMsg.append( "Sleep mode" );
					unit_status.sleep = val;
					break;
				}
				case 11:
				{
					if( val )
						mode |= 0x08;
					break;
				}
				case 12:
				{
					if( val )
						mode |= 0x04;
					break;
				}
				case 13:
				{
					if( val )
						mode |= 0x02;
					break;
				}
				case 14:
				{
					if( val )
						mode |= 0x01;
					switch( mode )
					{
						case 0x01:
						{
							dtkMsg.append( "DATA_POSITION" );
							unit_status.data_mode = DATA_POSITION;
							break;
						}
						case 0x02:
						{
							dtkMsg.append( "DATA_ANGLES" );
							unit_status.data_mode = DATA_ANGLES;
							break;
						}
						case 0x03:
						{
							dtkMsg.append( "DATA_MATRIX" );
							unit_status.data_mode = DATA_MATRIX;
							break;
						}
						case 0x04:
						{
							dtkMsg.append( "DATA_POSITION_ANGLES" );
							unit_status.data_mode = DATA_POSITION_ANGLES;
							break;
						}
						case 0x05:
						{
							dtkMsg.append( "DATA_POSITION_MATRIX" );
							unit_status.data_mode = DATA_POSITION_MATRIX;
							break;
						}
						case 0x06:
						{
							dtkMsg.append( "DATA_FACTORY_USE_ONLY" );
							unit_status.data_mode = DATA_FACTORY_USE_ONLY;
							break;
						}
						case 0x07:
						{
							dtkMsg.append( "DATA_QUATERNION" );
							unit_status.data_mode = DATA_QUATERNION;
							break;
						}
						case 0x08:
						{
							dtkMsg.append( "DATA_POSITION_QUATERNION" );
							unit_status.data_mode = DATA_POSITION_QUATERNION;
							break;
						}
						default:
						{
							dtkMsg.append( "DATA_NONE" );
							unit_status.data_mode = DATA_NONE;
							break;
						}
					}
					break;
				}
				case 15:
				{
					dtkMsg.append( "Stream" );
					unit_status.stream = val;
					break;
				}
			}
			if( i < 11 || i > 13 )
				dtkMsg.append( "\n" );
			test = test >> 1;
		}
		dtkMsg.append( "\n" );
		if( expectError )
		{
			usleep( 10000 );
			checkError( "examineBirdStatus", temp_buf, itr->first );
		}
	}

	itr = device.birds.begin();
	device.data_size = 0;
	for( ;itr!=device.birds.end();itr++ )
	{
		if( 2 * data_sizes[itr->second.data_mode] > device.max_data_size )
			device.max_data_size = 2 * data_sizes[itr->second.data_mode];
		device.data_size += 2 * data_sizes[itr->second.data_mode];
	}

	device.buffer_size = 128 * stations;
	if( !( device.buffer = new char[device.buffer_size] ) )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to allocate device buffer of size %d\n",
			device.buffer_size );
		return DTKSERVICE_ERROR;
	}
	device.cur_ptr = device.begin_ptr = device.end_ptr = device.buffer;

	usleep( 100000 );
	changeGroupMode( true );
	checkError( "changeGroupMode", temp_buf, 1 );

	usleep( 600000 );
	changeFBBAutoConfigure( stations );
	usleep( 600000 );

	sendPoint();
	usleep( 100000 );
	readValue( device.buffer, 1000000, 10 );
	checkError( "sendPoint", temp_buf, 1 );

	sendRun();
	usleep( 600000 );
	checkError( "sendRun", temp_buf, 1 );

	for( itr = device.birds.begin();itr!=device.birds.end();itr++ )
	{
		checkError( "Check all birds for errors", temp_buf, itr->first, 1000 );
		usleep( 100000 );
	}

	sendStream();
	usleep( 100000 );
//	sendReportRate( FOB_RATE_32 );
//	changeReportRate( 127 );
	usleep( 100000 );

	return DTKSERVICE_CONTINUE;
}

flockofbirds::~flockofbirds()
{
	clean_up();
}

void flockofbirds::clean_up()
{
#ifdef DTK_ARCH_WIN32_VCPP
	if( fd != NULL )
#else
	if( fd != -1 )
#endif
	{
		readValue( device.buffer, 1000000, 10 );
		sendPoint();
		dtkMsg.add( DTKMSG_NOTICE, "Total %d bytes read after stopping stream\n",
				1000000 - readValue( device.buffer, 1000000 ) );
		checkError( "sendPoint", device.buffer );
		usleep( 200000 );
		sendSleep();
		usleep( 200000 );
		checkError( "sendSleep", device.buffer );
		usleep( 200000 );
		changeGroupMode( false );
		usleep( 200000 );
		checkError( "changeGroupMode", device.buffer );
		usleep( 600000 );
		resetMaster();
		usleep( 600000 );
		sendFBBReset();
		usleep( 200000 );
		checkError( "sendFBBReset", device.buffer );
	}

	if( device.buffer )
	{
		delete [] device.buffer;
		device.cur_ptr = device.begin_ptr = device.end_ptr = device.buffer = NULL;
	}

#ifndef DTK_ARCH_WIN32_VCPP
	if( VRCO )
		delete VRCO;
#endif

	for( unsigned int i=0;i<device.birds.size();i++ )
	{
		if( device.birds[i].dtkshm )
			delete device.birds[i].dtkshm;
	}

#ifdef DTK_ARCH_WIN32_VCPP
	CancelIo( fd );
	fd = NULL;
#else
	close( fd );
	fd = -1;
#endif
}

int flockofbirds::serve()
{
	int bytes_avail = device.buffer_size - (int)( device.end_ptr - device.buffer );

#ifdef DTK_ARCH_WIN32_VCPP
	DWORD read_size;
	ReadFile( fd, device.end_ptr, device.max_data_size, NULL, &io_overlapped );
	GetOverlappedResult( fd, &io_overlapped, &read_size, true );
	int bytes_read = (int)read_size;
#else
	int bytes_read = read( fd, device.end_ptr, bytes_avail );
#endif

	device.end_ptr += bytes_read;
	int cur_size = (int)( device.end_ptr - device.begin_ptr );
	if( bytes_avail < 64 )
	{
		memcpy( device.buffer, device.begin_ptr, cur_size );
		device.end_ptr = device.buffer + cur_size;
		device.cur_ptr = device.begin_ptr = device.buffer;
	}
	if( device.run )
	{
		if( cur_size > device.max_data_size + 1 )
		{
		}
		int current_bird = -1;
		while( cur_size > device.max_data_size + 1 )
		{
			bool skipping = false;
			while( !( *device.cur_ptr & 128 ) && cur_size )
			{
				if( !skipping )
				{
					dtkMsg.add( DTKMSG_ERROR, "skipped: " );
					skipping = true;
				}
				dtkMsg.append( "%x  ", *device.cur_ptr );
				device.cur_ptr++;
				cur_size--;
			}
			if( skipping )
				dtkMsg.append( "\n" );
			if( cur_size <= device.max_data_size )
				break;
			current_bird = -1;
			map<char, bird_state>::iterator itr = device.birds.begin();
			for( ;itr!=device.birds.end();itr++ )
			{
				if( itr->second.has_sensor )
				{
					if( device.cur_ptr[2*data_sizes[itr->second.data_mode]] == itr->first )
						current_bird = itr->first;
				}
			}
			if( current_bird == -1 )
			{
				dtkMsg.add( DTKMSG_WARNING, "Skipped flock data record - !\n" );
				for( int i=0;i<cur_size;i++ )
					dtkMsg.append( "%x  ", device.cur_ptr[i] );
				dtkMsg.append( "\n" );
				device.cur_ptr++;
				cur_size--;
				continue;
			}
			else
			{
				for( int i=0;i<data_sizes[device.birds[current_bird].data_mode];i++ )
				{
					short val = convertData( device.cur_ptr );
					switch( device.birds[current_bird].data_mode )
					{
/*						case DATA_POSITION:
						{
							device.birds[current_bird].tracker_data[i] = (float)val *
									device.position_scale / 32768.0f;
							break;
						}*/
						case DATA_POSITION_ANGLES:
						{
							if( i < 3 )
								device.birds[current_bird].tracker_data[i] = (float)val *
										device.position_scale / 32768.0f / 12.0f;
							else if( device.birds[current_bird].angle_degrees )
								device.birds[current_bird].tracker_data[i] = (float)val *
										180.0f / 32768.0f;
							else
								device.birds[current_bird].tracker_data[i] = (float)val *
										M_PI_2 / 32768.0f;
							break;
						}
/*						case DATA_POSITION_MATRIX:
						{
							if( i < 3 )
								device.birds[current_bird].tracker_data[i] = (float)val *
										device.position_scale / 32768.0f;
							else
								device.birds[current_bird].tracker_data[i] = (float)val / 32768.0f;
							break;
						}
						case DATA_POSITION_QUATERNION:
						{
							if( i < 3 )
								device.birds[current_bird].tracker_data[i] = (float)val *
										device.position_scale / 32768.0f;
							else
								device.birds[current_bird].tracker_data[i] = (float)val / 32768.0f;
							break;
						}
						case DATA_ANGLES:
						{
							if( device.birds[current_bird].angle_degrees )
								device.birds[current_bird].tracker_data[i] = (float)val *
										180.0f / 32768.0f;
							else
								device.birds[current_bird].tracker_data[i] = (float)val *
										M_PI_2 / 32768.0f;
							break;
						}
						case DATA_MATRIX:
						{
							device.birds[current_bird].tracker_data[i] = (float)val / 32768.0f;
							break;
						}
						case DATA_QUATERNION:
						{
							device.birds[current_bird].tracker_data[i] = (float)val / 32768.0f;
							break;
						}*/
						default:
						{
							dtkMsg.add( DTKMSG_ERROR, "Invalid data mode\n" );
							return DTKSERVICE_ERROR;
						}
					}
					device.cur_ptr += 2;
					cur_size -= 2;
				}
				device.cur_ptr++;
				cur_size--;
static bool error_rawCalibrate = false;
				if( getCalibrator() )
				{
					if( getCalibrator()->rawCalibrate( device.birds[current_bird].tracker_data,
							(void*)&device.birds[current_bird].metal_error_value ) )
					{
						if( !error_rawCalibrate )
						{
							error_rawCalibrate = true;
							dtkMsg.add( DTKMSG_WARNING, "The rawCalibrate function is "
									"experiencing an error. Continuing.\n" );
						}
					}
					else if( error_rawCalibrate )
					{
						error_rawCalibrate = false;
						dtkMsg.add( DTKMSG_NOTICE, "The rawCalibration function is "
								"functioning normally.\n" );
					}
				}

				for( int i=0;i<data_sizes[device.birds[current_bird].data_mode];i++ )
				{
						device.axis_tracker_data[i] = device.axis_sign[i] *
								device.birds[current_bird].tracker_data[device.axis_map[i]];
				}

				rawOffsetAndOrientation( device.birds[current_bird] );

static bool error_calibrate = false;
				if( getCalibrator() )
				{
					if( getCalibrator()->calibrate( device.birds[current_bird].tracker_data,
							(void*)&device.birds[current_bird].metal_error_value ) )
					{
						if( !error_calibrate )
						{
							error_calibrate = true;
							dtkMsg.add( DTKMSG_WARNING, "The calibrate function is "
									"experiencing an error. Continuing.\n" );
						}
					}
					else if( error_calibrate )
					{
						error_calibrate = false;
						dtkMsg.add( DTKMSG_NOTICE, "The calibration function is "
								"functioning normally.\n" );
					}
				}

#ifndef DTK_ARCH_WIN32_VCPP
				if( device.trackd )
				{
					if( device.birds[current_bird].trackd_shm_type == TRACKD_TRACKER )
						VRCO->writeTracker( device.axis_tracker_data, 0 );
					else if( device.birds[current_bird].trackd_shm_type == TRACKD_CONTROLLER )
						VRCO->writeTracker( device.axis_tracker_data, 1 );
				}
#endif
				for( int i=0;i<3;i++ )
					device.axis_tracker_data[i] *=
							12.0f * 2.54f / ( device.diverse_unit * 100.0f );
				device.birds[current_bird].dtkshm->write( device.axis_tracker_data );
			}
			device.begin_ptr = device.cur_ptr;
		}
	}
	device.begin_ptr = device.cur_ptr;

	return DTKSERVICE_CONTINUE;
}

int flockofbirds::rawOffsetAndOrientation( bird_state& bird )
{
	switch( bird.data_mode )
	{
		case DATA_POSITION:
		{
			break;
		}
		case DATA_POSITION_ANGLES:
		{
dtkMatrix mat;
dtkVec3 loc_xyz( device.axis_tracker_data[0], device.axis_tracker_data[1],
		device.axis_tracker_data[2] );
dtkVec3 loc_hpr( device.axis_tracker_data[3], device.axis_tracker_data[4],
		device.axis_tracker_data[5] );

dtkVec3 xmtr_xyz( device.transmitter_offset );
dtkVec3 xmtr_hpr( device.transmitter_rotation );

dtkVec3 snsr_xyz( bird.sensor_offset );
dtkVec3 snsr_hpr( bird.sensor_rotation );
/*
mat.rotateHPR( snsr_hpr );
mat.translate( snsr_xyz );
mat.rotateHPR( loc_hpr );
mat.translate( loc_xyz );
mat.rotateHPR( xmtr_hpr );
mat.translate( xmtr_xyz );
*/
mat.translate( snsr_xyz );
mat.rotateHPR( snsr_hpr );
mat.rotateHPR( loc_hpr );
mat.translate( loc_xyz );
mat.rotateHPR( xmtr_hpr );
mat.translate( xmtr_xyz );

			dtkCoord output;
			mat.coord( &output );
			for( int i=0;i<6;i++ )
				device.axis_tracker_data[i] = output.d[i];
			break;
		}
		case DATA_POSITION_MATRIX:
		{
			break;
		}
		case DATA_POSITION_QUATERNION:
		{
			break;
		}
		case DATA_ANGLES:
		{
			break;
		}
		case DATA_MATRIX:
		{
			break;
		}
		case DATA_QUATERNION:
		{
			break;
		}
		default:
		{
			dtkMsg.add( DTKMSG_ERROR, "Invalid data mode\n" );
			return -1;
		}
	}
	return 0;
}

short flockofbirds::convertData( const char *data )
{
	short val = *(short*)data;
	unsigned char *swap = (unsigned char*)&val;
	swap[0] <<= 1;
	val <<= 1;
	return val;
}

void flockofbirds::resetMaster()
{
#ifdef DTK_ARCH_WIN32_VCPP
	EscapeCommFunction( fd, SETRTS );
	usleep( 100000 );
	EscapeCommFunction( fd, CLRRTS );
	usleep( 100000 );
#else
	int parm;
	ioctl( fd, TIOCMGET, &parm);
	parm ^= TIOCM_RTS;
	ioctl( fd, TIOCMSET, &parm );
	usleep( 100000 );
	parm ^= TIOCM_RTS;
	ioctl( fd, TIOCMSET, &parm );
	usleep( 100000 );
#endif
}

int flockofbirds::sendCommand( const string& cmd, const char& unit )
{
	string cur_cmd = cmd;
	if( unit > 1 )
	{
		cur_cmd = sendRS232toFBB( cmd, unit );
	}
#ifdef DTK_ARCH_WIN32_VCPP
	DWORD write_size;
	WriteFile( fd, cur_cmd.c_str(), cur_cmd.size(), NULL, &io_overlapped );
	GetOverlappedResult( fd, &io_overlapped, &write_size, true );
	size_t bytes = (size_t)write_size;
	if( bytes != cur_cmd.size() )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
		"flockofbirds::sendCommand() failed: error sending command string to tracker:\n"
		"WriteFile(fd=%d,cmd=%s,size=%d,size_wrote=%d) to device file %s%s%s failed.", fd,
		cur_cmd.c_str(), cur_cmd.size(), bytes,
		dtkMsg.color.tur, device.port.c_str(), dtkMsg.color.end );
		return -1;
	}
#else
	size_t bytes = write( fd, cur_cmd.c_str(), cur_cmd.size() );
	if( bytes != cur_cmd.size() )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
		"flockofbirds::sendCommand() failed: error sending command string to tracker:\n"
		"write(fd=%d,cmd=%s,size=%d) to device file %s%s%s failed.", fd,
		cur_cmd.c_str(), cur_cmd.size(),
		dtkMsg.color.tur, device.port.c_str(), dtkMsg.color.end );
		return -1;
	}
#endif
	return 0;
}

int flockofbirds::changeValue( const string& change, const char& unit )
{
	string temp( 1, (char)0x50 );
	string cmd = temp + change;
	return sendCommand( cmd, unit );
}

int flockofbirds::examineValue( const string& examine, const char& unit )
{
	string temp( 1, (char)0x4F );
	string cmd = temp + examine;
	return sendCommand( cmd, unit );
}

string flockofbirds::sendRS232toFBB( const string& cur_cmd, const char& unit )
{
	string temp;
	if( device.addr_mode == ADDR_NORMAL )
	{
		temp.append( 1, (char)( 0xF0 + unit ) );
	} 
	else if( device.addr_mode == ADDR_EXPANDED )
	{
		if( unit < 16 )
			temp.append( 1, (char)( 0xF0 + unit ) );
		else
			temp.append( 1, (char)( 0xE0 + unit - 16 ) );
	} 
	else if( device.addr_mode == ADDR_SUPER_EXPANDED )
	{
		temp.append( 1, (char)0xA0 );
		temp.append( 1, (char)unit );
	}
	string cmd = temp + cur_cmd;
	return cmd;
}

int flockofbirds::sendAngles( const char& unit )
{
	device.birds[unit].data_mode = DATA_ANGLES;
	string temp( 1, (char)0x57 );
	return sendCommand( temp, unit );
}

int flockofbirds::sendAngleAlign1( float sinA, float cosA, float sinE, float cosE,
	float sinR, float cosR, const char& unit )
{
	string temp( 1, (char)0x4A );
	short value;
	char* val = (char*)&value;
	value = (short)( sinA * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( cosA * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( sinE * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( cosA * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( sinR * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( cosA * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	return sendCommand( temp, unit );
}

int flockofbirds::sendAngleAlign2( float A, float E, float R, const char& unit )
{
	string temp( 1, (char)0x71 );
	short value;
	char* val = (char*)&value;
	value = (short)( ( A / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( E / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( R / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	return sendCommand( temp, unit );
}

int flockofbirds::sendBoresight( const char& unit )
{
	string temp( 1, (char)0x75 );
	return sendCommand( temp, unit );
}

int flockofbirds::sendBoresightRemove( const char& unit )
{
	string temp( 1, (char)0x76 );
	return sendCommand( temp, unit );
}

int flockofbirds::sendButtonMode( bool mode, const char& unit )
{
	string temp( 1, (char)0x4D );
	return sendCommand( temp, unit );
}

int flockofbirds::sendButtonRead( const char& unit )
{
	string temp( 1, (char)0x4E );
	return sendCommand( temp, unit );
}

int flockofbirds::sendFBBReset()
{
	string temp( 1, (char)0x2F );
	return sendCommand( temp );
}

int flockofbirds::sendHemisphere( const string& hemisphere, const char& unit )
{
	string temp( 1, (char)0x4C );
	if( hemisphere == "forward" )
	{
		temp.append( 1, (char)0x00 );
		temp.append( 1, (char)0x00 );
	}
	else if( hemisphere == "rear" )
	{
		temp.append( 1, (char)0x00 );
		temp.append( 1, (char)0x01 );
	}
	else if( hemisphere == "upper" )
	{
		temp.append( 1, (char)0x0C );
		temp.append( 1, (char)0x00 );
	}
	else if( hemisphere == "lower" )
	{
		temp.append( 1, (char)0x0C );
		temp.append( 1, (char)0x01 );
	}
	else if( hemisphere == "left" )
	{
		temp.append( 1, (char)0x06 );
		temp.append( 1, (char)0x00 );
	}
	else if( hemisphere == "right" )
	{
		temp.append( 1, (char)0x06 );
		temp.append( 1, (char)0x01 );
	}
	else
	{
		dtkMsg.add( DTKMSG_ERROR, "The hemisphere option does not accept %s"
				"as a valid parameter\n", hemisphere.c_str() );
	}
	return sendCommand( temp, unit );
}

int flockofbirds::sendMatrix( const char& unit )
{
	string temp( 1, (char)0x58 );
	device.birds[unit].data_mode = DATA_MATRIX;
	return sendCommand( temp, unit );
}

int flockofbirds::sendMetal( bool enable, const char& unit )
{
	if( enable )
		return sendMetal( METAL_ON, 0, unit );
	else
		return sendMetal( METAL_OFF, 0, unit );
}

int flockofbirds::sendMetalSensitivity( char sensitivity, const char& unit )
{
	if( sensitivity < 0 )
	{
		dtkMsg.add( DTKMSG_ERROR, "Metal sensitivity value of %d is not"
				"between 0 and 127\n", sensitivity );
		return -1;
	}
	device.birds[unit].metal_sensitivity = sensitivity;
	return sendMetal( METAL_ON_SENSITIVITY, sensitivity, unit );
}

int flockofbirds::sendMetalOffset( char offset, const char& unit )
{
	device.birds[unit].metal_offset = offset;
	return sendMetal( METAL_ON_OFFSET, offset, unit );
}

int flockofbirds::sendMetalSlope( char slope, const char& unit )
{
	device.birds[unit].metal_slope = slope;
	return sendMetal( METAL_ON_SLOPE, slope, unit );
}

int flockofbirds::sendMetalAlpha( char alpha, const char& unit )
{
	if( alpha < 0 )
	{
		dtkMsg.add( DTKMSG_ERROR, "Metal alpha value of %d is not"
				"between 0 and 127\n", alpha );
		return -1;
	}
	device.birds[unit].metal_alpha = alpha;
	return sendMetal( METAL_ON_ALPHA, alpha, unit );
}

int flockofbirds::sendMetal( metal_flag flag, char data, const char& unit )
{
	string temp( 1, (char)0x73 );
	temp.append( 1, (char)flag );
	temp.append( 1, data );
	return sendCommand( temp, unit );
}

int flockofbirds::sendMetalError( const char& unit )
{
	string temp( 1, (char)0x74 );
	return sendCommand( temp, unit );
}

int flockofbirds::sendNextTransmitter( char address, char num )
{
	char value = address;
	value <<= 4;
	value |= num;
	string temp( 1, (char)0x30 );
	temp.append( 1, value );
	return sendCommand( temp );
}

int flockofbirds::sendOffset( float x, float y, float z, const char& unit )
{
	string temp( 1, (char)0x4B );
	short value;
	char* val = (char*)&value;
	value = (short)( ( x / device.position_scale ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( y / device.position_scale ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( z / device.position_scale ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	return sendCommand( temp, unit );
}

int flockofbirds::sendPoint( const char& unit )
{
	string temp( 1, (char)0x42 );
	return sendCommand( temp, unit );
}

int flockofbirds::sendPosition( const char& unit )
{
	string temp( 1, (char)0x56 );
	device.birds[unit].data_mode = DATA_POSITION;
	return sendCommand( temp, unit );
}

int flockofbirds::sendPositionAngles( const char& unit )
{
	string temp( 1, (char)0x59 );
	device.birds[unit].data_mode = DATA_POSITION_ANGLES;
	return sendCommand( temp, unit );
}

int flockofbirds::sendPositionMatrix( const char& unit )
{
	string temp( 1, (char)0x5A );
	device.birds[unit].data_mode = DATA_POSITION_MATRIX;
	return sendCommand( temp, unit );
}

int flockofbirds::sendPositionQuaternion( const char& unit )
{
	string temp( 1, (char)0x5D );
	device.birds[unit].data_mode = DATA_POSITION_QUATERNION;
	return sendCommand( temp, unit );
}

int flockofbirds::sendQuaternion( const char& unit )
{
	string temp( 1, (char)0x5C );
	device.birds[unit].data_mode = DATA_QUATERNION;
	return sendCommand( temp, unit );
}

int flockofbirds::sendReferenceFrame1( float sinA, float cosA, float sinE, float cosE,
	float sinR, float cosR, const char& unit )
{
	string temp( 1, (char)0x48 );
	short value;
	char* val = (char*)&value;
	value = (short)( sinA * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( cosA * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( sinE * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( cosA * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( sinR * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( cosA * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	return sendCommand( temp, unit );
}

int flockofbirds::sendReferenceFrame2( float A, float E, float R, const char& unit )
{
	string temp( 1, (char)0x72 );
	short value;
	char* val = (char*)&value;
	value = (short)( ( A / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( E / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( R / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	return sendCommand( temp, unit );
}

int flockofbirds::sendReportRate( report_rate rate, const char& unit )
{
	string temp;
	switch( rate )
	{
		case FOB_RATE_1:
		{
			temp.append( 1, (char)0x51 );
			break;
		}
		case FOB_RATE_2:
		{
			temp.append( 1, (char)0x52 );
			break;
		}
		case FOB_RATE_8:
		{
			temp.append( 1, (char)0x53 );
			break;
		}
		case FOB_RATE_32:
		{
			temp.append( 1, (char)0x54 );
			break;
		}
	}
	return sendCommand( temp, unit );
}

int flockofbirds::sendRun()
{
	string temp( 1, (char)0x46 );
	device.run = true;
	return sendCommand( temp );
}

int flockofbirds::sendSleep()
{
	string temp( 1, (char)0x47 );
	device.run = false;
	return sendCommand( temp );
}

int flockofbirds::sendStream( const char& unit )
{
	string temp( 1, (char)0x40 );
	device.stream = true;
	return sendCommand( temp, unit );
}

int flockofbirds::sendStreamStop( const char& unit )
{
	string temp( 1, (char)0x3F );
	device.stream = false;
	return sendCommand( temp, unit );
}

int flockofbirds::sendSync( sync_type type, const char& unit )
{
	string temp( 1, (char)0x41 );
	return sendCommand( temp, unit );
}

int flockofbirds::sendXOff( const char& unit )
{
	device.birds[unit].xoff = true;
	string temp( 1, (char)0x13 );
	return sendCommand( temp, unit );
}

int flockofbirds::sendXOn( const char& unit )
{
	device.birds[unit].xoff = false;
	string temp( 1, (char)0x11 );
	return sendCommand( temp, unit );
}

int flockofbirds::examineBirdStatus( const char& unit )
{
	string temp( 1, (char)0x00 );
	return examineValue( temp, unit );
}

int flockofbirds::examineSoftwareRevisionNumber( const char& unit )
{
	string temp( 1, (char)0x01 );
	return examineValue( temp, unit );
}

int flockofbirds::examineBirdComputerCrystalSpeed( const char& unit )
{
	string temp( 1, (char)0x02 );
	return examineValue( temp, unit );
}

int flockofbirds::examinePositionScaling( const char& unit )
{
	string temp( 1, (char)0x03 );
	return examineValue( temp, unit );
}

int flockofbirds::changePositionScaling( bool enable, const char& unit )
{
	string temp( 1, (char)0x03 );
	if( enable )
		temp.append( 1, (char)0x01 );
	else
		temp.append( 1, (char)0x00 );
	return changeValue( temp, unit );
}

int flockofbirds::examineFilterStatus( const char& unit )
{
	string temp( 1, (char)0x04 );
	return examineValue( temp, unit );
}

int flockofbirds::changeFilterStatus( bool ac_narrow, bool ac_wide, bool dc_filter,
	const char& unit )
{
	string temp( 1, (char)0x04 );
	return changeValue( temp, unit );
}

int flockofbirds::examineDCFilterAlphaMin( const char& unit )
{
	string temp( 1, (char)0x05 );
	return examineValue( temp, unit );
}

int flockofbirds::changeDCFilterAlphaMin( short alpha[7], const char& unit )
{
	string temp( 1, (char)0x05 );
	return changeValue( temp, unit );
}

int flockofbirds::examineBirdMeasurementRateCount( const char& unit )
{
	string temp( 1, (char)0x06 );
	return examineValue( temp, unit );
}

int flockofbirds::changeBirdMeasurementRateCount( unsigned char count, const char& unit )
{
	string temp( 1, (char)0x06 );
	return changeValue( temp, unit );
}

int flockofbirds::examineBirdMeasurementRate( const char& unit )
{
	string temp( 1, (char)0x07 );
	return examineValue( temp, unit );
}

int flockofbirds::changeBirdMeasurementRate( unsigned char rate, const char& unit )
{
	string temp( 1, (char)0x07 );
	return changeValue( temp, unit );
}

int flockofbirds::examineDataReadyOutput( const char& unit )
{
	string temp( 1, (char)0x08 );
	return examineValue( temp, unit );
}

int flockofbirds::changeDataReadyOutput( bool enable, const char& unit )
{
	string temp( 1, (char)0x08 );
	return changeValue( temp, unit );
}

int flockofbirds::examineDataReadyCharacter( const char& unit )
{
	string temp( 1, (char)0x09 );
	return examineValue( temp, unit );
}

int flockofbirds::changeDataReadyCharacter( char character, const char& unit )
{
	string temp( 1, (char)0x09 );
	return changeValue( temp, unit );
}

int flockofbirds::examineErrorCode( const char& unit )
{
	string temp( 1, (char)0x0A );
	return examineValue( temp, unit );
}

int flockofbirds::examineErrorDetectMask( const char& unit )
{
	string temp( 1, (char)0x0B );
	return examineValue( temp, unit );
}

int flockofbirds::changeErrorDetectMask( error_mask mask, const char& unit )
{
	string temp( 1, (char)0x0B );
	return changeValue( temp, unit );
}

int flockofbirds::examineDCFilterVm( const char& unit )
{
	string temp( 1, (char)0x0C );
	return examineValue( temp, unit );
}

int flockofbirds::changeDCFilterVm( short levels[7], const char& unit )
{
	string temp( 1, (char)0x0C );
	return changeValue( temp, unit );
}

int flockofbirds::examineDCFilterAlphaMax( const char& unit )
{
	string temp( 1, (char)0x0D );
	return examineValue( temp, unit );
}

int flockofbirds::changeDCFilterAlphaMax( short alpha[7], const char& unit )
{
	string temp( 1, (char)0x0D );
	return changeValue( temp, unit );
}

int flockofbirds::examineSuddenOutputChangeLock( const char& unit )
{
	string temp( 1, (char)0x0E );
	return examineValue( temp, unit );
}

int flockofbirds::changeSuddenOutputChangeLock( bool enable, const char& unit )
{
	string temp( 1, (char)0x0E );
	return changeValue( temp, unit );
}

int flockofbirds::examineSystemModelIdentification( const char& unit )
{
	string temp( 1, (char)0x0F );
	return examineValue( temp, unit );
}

int flockofbirds::examineExpandedErrorCode( const char& unit )
{
	string temp( 1, (char)0x10 );
	return examineValue( temp, unit );
}

int flockofbirds::examineXYZReferenceFrame( const char& unit )
{
	string temp( 1, (char)0x11 );
	return examineValue( temp, unit );
}

int flockofbirds::changeXYZReferenceFrame( bool enable, const char& unit )
{
	string temp( 1, (char)0x11 );
	if( enable )
		temp.append( 1, (char)0x01 );
	else
		temp.append( 1, (char)0x00 );
	return changeValue( temp, unit );
}

int flockofbirds::examineTransmitterOperationMode( const char& unit )
{
	string temp( 1, (char)0x12 );
	return examineValue( temp, unit );
}

int flockofbirds::changeTransmitterOperationMode( transmitter_mode mode, const char& unit )
{
	string temp( 1, (char)0x12 );
	return changeValue( temp, unit );
}

int flockofbirds::examineFBBAddressingMode( const char& unit )
{
	string temp( 1, (char)0x13 );
	return examineValue( temp, unit );
}

int flockofbirds::examineFilterLineFrequency( const char& unit )
{
	string temp( 1, (char)0x14 );
	return examineValue( temp, unit );
}

int flockofbirds::changeFilterLineFrequency( const char& unit )
{
	string temp( 1, (char)0x14 );
	return changeValue( temp, unit );
}

int flockofbirds::examineFBBAddress( const char& unit )
{
	string temp( 1, (char)0x15 );
	return examineValue( temp, unit );
}

int flockofbirds::examineHemisphere( const char& unit )
{
	string temp( 1, (char)0x16 );
	return examineValue( temp, unit );
}

int flockofbirds::changeHemisphere( const char* hemisphere, const char& unit )
{
	string temp( 1, (char)0x16 );
	return changeValue( temp, unit );
}

int flockofbirds::examineAngleAlign2( const char& unit )
{
	string temp( 1, (char)0x17 );
	return examineValue( temp, unit );
}

int flockofbirds::changeAngleAlign2( float A, float E, float R, const char& unit )
{
	string temp( 1, (char)0x17 );
	short value;
	char* val = (char*)&value;
	value = (short)( ( A / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( E / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( R / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	return changeValue( temp, unit );
}

int flockofbirds::examineReferenceFrame2( const char& unit )
{
	string temp( 1, (char)0x18 );
	return examineValue( temp, unit );
}

int flockofbirds::changeReferenceFrame2( float A, float E, float R, const char& unit )
{
	string temp( 1, (char)0x18 );
	short value;
	char* val = (char*)&value;
	value = (short)( ( A / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( E / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	value = (short)( ( R / 180.0f ) * 32768.0f );
	temp.append( 1, val[1] );
	temp.append( 1, val[0] );
	return changeValue( temp, unit );
}

int flockofbirds::examineBirdSerialNumber( const char& unit )
{
	string temp( 1, (char)0x19 );
	return examineValue( temp, unit );
}

int flockofbirds::examineSensorSerialNumber( const char& unit )
{
	string temp( 1, (char)0x1A );
	return examineValue( temp, unit );
}

int flockofbirds::examineTransmitterSerialNumber( const char& unit )
{
	string temp( 1, (char)0x1B );
	return examineValue( temp, unit );
}

int flockofbirds::examineMetalDetection( const char& unit )
{
	string temp( 1, (char)0x1C );
	return examineValue( temp, unit );
}

int flockofbirds::changeMetalDetection( short metal[5], const char& unit )
{
	string temp( 1, (char)0x1C );
	return changeValue( temp, unit );
}

int flockofbirds::examineReportRate( const char& unit )
{
	string temp( 1, (char)0x1D );
	return examineValue( temp, unit );
}

int flockofbirds::changeReportRate( char rate, const char& unit )
{
	string temp( 1, (char)0x1D );
	temp.append( 1, rate );
	return changeValue( temp, unit );
}

int flockofbirds::examineFBBHostResponseDelay( const char& unit )
{
	string temp( 1, (char)0x20 );
	return examineValue( temp, unit );
}

int flockofbirds::changeFBBHostResponseDelay( short delay, const char& unit )
{
	string temp( 1, (char)0x20 );
	return changeValue( temp, unit );
}

int flockofbirds::examineGroupMode( const char& unit )
{
	string temp( 1, (char)0x23 );
	return examineValue( temp, unit );
}

int flockofbirds::changeGroupMode( bool enable, const char& unit )
{
	string temp( 1, (char)0x23 );
	if( enable )
		temp.append( 1, (char)0x01 );
	else
		temp.append( 1, (char)0x00 );
	device.group = enable;
	return changeValue( temp, unit );
}

int flockofbirds::examineFlockSystemStatus( const char& unit )
{
	string temp( 1, (char)0x24 );
	return examineValue( temp, unit );
}

int flockofbirds::examineFBBAutoConfigure( const char& unit )
{
	string temp( 1, (char)0x32 );
	return examineValue( temp, unit );
}

int flockofbirds::changeFBBAutoConfigure( char num, const char& unit )
{
	if( num < 0 )
	{
		dtkMsg.add( DTKMSG_ERROR, "The number of units to configure must be greater than 0\n" );
		return -1;
	}
	string temp( 1, (char)0x32 );
	temp.append( 1, num );
	return changeValue( temp, unit );
}

// Loader and unloader for DSO
static dtkService *dtkDSO_loader( const char* arg )
{
	return new flockofbirds( arg );
}

static int dtkDSO_unloader( dtkService *flockofbirds )
{
	delete flockofbirds;
	return DTKDSO_UNLOAD_CONTINUE;
}

