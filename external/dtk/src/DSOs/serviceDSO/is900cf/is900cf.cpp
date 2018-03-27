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

// Written by Patrick Shinpaugh 12/2008

#include <dtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifndef DTK_ARCH_WIN32_VCPP
#  include <unistd.h>
#  include <termios.h>
#  include <sys/ioctl.h>
#endif
#include <fcntl.h>
#include <dtk/dtkService.h>
#include <vector>
#include <string>
//#include <list>
#include <map>
#include <algorithm>
#include <fstream>
#include <errno.h>

#define TEMP_BUFFER_SIZE 4096

#define STATION_MIN 1
#define STATION_MAX 32

#ifdef DTK_ARCH_WIN32_VCPP
#  define SEPARATOR ";"
#else
#  define SEPARATOR ":"
#endif

using namespace std;

class IS900 : public dtkConfigService
{
public:
	IS900( const char* arg );
	virtual ~IS900();

	// Initialize the service - called by the dtkServer
	virtual int init();
	int serve();

	enum trackd_type
	{
		TRACKD_NONE,
		TRACKD_TRACKER,
		TRACKD_CONTROLLER
	};

	enum parity_type
	{
		TEST_PARITY_NONE = 'N',
		TEST_PARITY_ODD = 'O',
		TEST_PARITY_EVEN = 'E'
	};

	enum bits_type
	{
		BITS_7 = 7,
		BITS_8 = 8
	};

	enum output_record_type
	{
		OUTPUT_RECORD_SEPARATOR_SPACE = 0,
		OUTPUT_RECORD_SEPARATOR_CR_LF = 1,
		OUTPUT_RECORD_POSITION_XYZ = 2,
		OUTPUT_RECORD_ORIENTATION_HPR = 4,
		OUTPUT_RECORD_ORIENTATION_COSINES_X = 5,
		OUTPUT_RECORD_ORIENTATION_COSINES_Y = 6,
		OUTPUT_RECORD_ORIENTATION_COSINES_Z = 7,
		OUTPUT_RECORD_ORIENTATION_QUATERNION = 11,
		OUTPUT_RECORD_POSITION_XYZ_16BIT = 18,
		OUTPUT_RECORD_ORIENTATION_HPR_16BIT = 19,
		OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT = 20,
		OUTPUT_RECORD_TIMESTAMP = 21,
		OUTPUT_RECORD_BUTTONS = 22,
		OUTPUT_RECORD_JOYSTICK = 23,
		OUTPUT_RECORD_TRACKING_STATUS = 40
	};

	enum genlock_sync_type
	{
		GENLOCK_SYNC_OFF = 0,
		GENLOCK_SYNC_EXTERNAL = 2,
		GENLOCK_SYNC_INTERNAL = 3
	};

	enum sync_source_type
	{
		GENLOCK_SYNC_SOURCE_TTL = 1,
		GENLOCK_SYNC_SOURCE_NTSC = 2
	};

	enum config_lock_type
	{
		CONFIG_LOCK_OFF,
		CONFIG_LOCK_SAVED,
		CONFIG_LOCK_SAVED_AND_SESSION
	};

	enum beacon_scheduler_type
	{
		BEACON_SCHEDULE_DISTANCE = 1,
		BEACON_SCHEDULE_DISTANCE_AND_ORIENTATION = 2
	};

	enum perceptual_enhancement_type
	{
		PERCEPTUAL_ENHANCEMENT_DRIFT,
		PERCEPTUAL_ENHANCEMENT_DRIFT_JITTER,
		PERCEPTUAL_ENHANCEMENT_IMMERSIVE
	};

	enum compass_heading_correction_type
	{
		COMPASS_CORRECTION_OFF,
		COMPASS_CORRECTION_PARTIAL,
		COMPASS_CORRECTION_FULL
	};

	enum rotational_sensitivity_type
	{
		ROTATIONAL_SENSITIVITY_LOW = 1,
		ROTATIONAL_SENSITIVITY_MEDIUM = 2,
		ROTATIONAL_SENSITIVITY_HIGH = 3,
		ROTATIONAL_SENSITIVITY_HIGHEST = 4
	};

	enum station_tracking_state
	{
		TRACKING_STATE_LOST = 'L',
		TRACKING_STATE_TRACKING = 'T',
		TRACKING_STATE_INVALID = 'X'
	};

	struct system_status_record
	{
		bool ascii;
		bool inches;
		bool polled;
		char bit_error[3];
		char blank[6];
		string firmware_version_id;
		string system_identification;
	};

	struct output_list_record
	{
		char station_id;
		vector<output_record_type> parameters;
	};

	struct station_status_record
	{
		char station_id;
		bool state;
	};

	struct alignment_reference_frame_record
	{
		char station_id;
		float origin[3];
		float axis_x[3];
		float axis_y[3];
	};

	struct boresight_reference_angles_record
	{
		char station_id;
		float hpr[3];
	};

	struct hemisphere_record
	{
		float vector[3];
	};

	struct tip_offset_record
	{
		char station_id;
		float offset[3];
	};

	struct position_operational_envelope_record
	{
		char station_id;
		float max[3];
		float min[3];
	};

	struct intersense_system_status_record
	{
		bool fastrak;
		bool milliseconds;
		bool led_control;
	};

	struct intersense_station_record
	{
		char station_id;
		perceptual_enhancement_type enhancement_level;
		compass_heading_correction_type compass_mode;
	};

	struct prediction_interval_record
	{
		char station_id;
		short interval;
	};

	struct sensitivity_level_record
	{
		char station_id;
		int sensitivity_level;
	};

	struct genlock_synchronization_record
	{
		int state;
		float rate;
		int cycles;
	};

	struct ultrasonic_timeout_record
	{
		short ultrasonic_timeout;
	};

	struct ultrasonic_sensitivity_record
	{
		int sensitivity_level;
	};

	struct fixed_pse_record
	{
		int fixed_pse_number;
		float position[3];
		float normal[3];
		string hardware_id;
	};

	struct station_tracking_state_record
	{
		station_tracking_state station_state;
		int range_measurement_received;
		int range_measurement_rejected;
	};

	struct tracking_status_record
	{
		vector<station_tracking_state_record> tracking_state;
		int update_rate;
		char genlock_state;
	};

	struct station_state
	{
		station_state()
		{
			enabled = true;
			has_position = false;
			has_angles = false;
			has_cosinesx = false;
			has_cosinesy = false;
			has_cosinesz = false;
			has_quaternion = false;
			angle_degrees = true;
			data_size = 0;
			buttons = 0;
			mapped_buttons = 0;
			dtkshm = NULL;
			for( int i=0;i<2;i++ )
			{
				joystick[i] = 0.0f;
				mapped_joystick[i] = 0.0f;
				joystick_map[i] = i;
				joystick_sign[i] = 1.0f;
			}
			for( int i=0;i<3;i++ )
			{
				sensor_offset[i] = 0.0f;
				sensor_rotation[i] = 0.0f;
				position[i] = 0.0f;
				orientation_hpr[i] = 0.0f;
				cosines_x[i] = 0.0f;
				cosines_y[i] = 0.0f;
				cosines_z[i] = 0.0f;
				mapped_position[i] = 0.0f;
				mapped_orientation_hpr[i] = 0.0f;
				mapped_cosines_x[i] = 0.0f;
				mapped_cosines_y[i] = 0.0f;
				mapped_cosines_z[i] = 0.0f;
				sensor_alignment_reference_frame[i] = 0.0f;
				sensor_alignment_reference_frame[i+3] = 0.0f;
				sensor_alignment_reference_frame[i+6] = 0.0f;
			}
			sensor_alignment_reference_frame[3] = 1.0f;
			sensor_alignment_reference_frame[7] = 1.0f;
			for( int i=0;i<4;i++ )
			{
				quaternion[i] = 0.0f;
				mapped_quaternion[i] = 0.0f;
				for( int j=0;j<3;j++ )
					tracker_data[4*j+i] = 0.0f;
			}
			for( int i=0;i<8;i++ )
				button_map[i] = i;
		}
		bool enabled;
		bool has_position;
		bool has_angles;
		bool has_cosinesx;
		bool has_cosinesy;
		bool has_cosinesz;
		bool has_quaternion;
		bool angle_degrees;
		int data_size;
		trackd_type trackd_shm_type;
		int trackd_key;
		string dtkshmname;
		dtkSharedMem* dtkshm;
		float sensor_alignment_reference_frame[9];
		float sensor_offset[3];
		float sensor_rotation[3];
		float tracker_data[12];
		vector<output_record_type> output_record_list;
		map<string, vector<output_record_type> > dtk_shm_map;
		map<string, dtkSharedMem*> dtk_shm;

		float position[3];
		float orientation_hpr[3];
		float cosines_x[3];
		float cosines_y[3];
		float cosines_z[3];
		float quaternion[4];
		float timestamp;
		unsigned char buttons;
		float joystick[2];
		char status;

		float mapped_position[3];
		float mapped_orientation_hpr[3];
		float mapped_cosines_x[3];
		float mapped_cosines_y[3];
		float mapped_cosines_z[3];
		float mapped_quaternion[4];
		unsigned char mapped_buttons;
		float mapped_joystick[2];

		int button_map[8];
		int joystick_map[2];
		float joystick_sign[2];
		vector<float> joystick_poly[2];
	};

	struct device_state
	{
		device_state()
		{
			polled = true;
			fastrak_compatibility = false;
			inches = true;
			ascii = true;
			milliseconds = true;
			baud_rate = 0;
			eth_port = 5001;
			eth_state = false;
			ultrasonic_timeout_interval = 0;
			ultrasonic_receiver_sensitivity = 0;
			genlock_sync_state = GENLOCK_SYNC_OFF;
			genlock_sync_rate = 0;
			genlock_sync_source = GENLOCK_SYNC_SOURCE_TTL;
			configuration_lock = CONFIG_LOCK_OFF;
			led_control = true;
			only_reload_constellation_file = false;
			beacon_scheduler = BEACON_SCHEDULE_DISTANCE_AND_ORIENTATION;
			error_reporting = false;
			command_logging = false;
			diverse_unit = 1.524f;
			for( int i=0;i<3;i++ )
			{
				transmitter_offset[i] = 0.0f;
				transmitter_rotation[i] = 0.0f;
				axis_map[i] = i;
				axis_map[i+3] = i;
			}
			for( int i=0;i<6;i++ )
			{
				axis_sign[i] = 1.0f;
			}
			buffer = cur_ptr = begin_ptr = end_ptr = NULL;
			max_data_size = 0;
		}
		int max_data_size;
		bool polled;
		bool fastrak_compatibility;
		bool inches;
		bool ascii;
		bool milliseconds;
		int baud_rate;
		string baud;
		string port;
		bool eth_state;
		string eth_address;
		int eth_port;
		string hemisphere;
		map<int, station_state> stations;
		int ultrasonic_timeout_interval;
		int ultrasonic_receiver_sensitivity;
		genlock_sync_type genlock_sync_state;
		int genlock_sync_rate;
		string genlock_phase;
		sync_source_type genlock_sync_source;
		config_lock_type configuration_lock;
		bool led_control;
		bool only_reload_constellation_file;
		beacon_scheduler_type beacon_scheduler;
		bool error_reporting;
		bool command_logging;
		float diverse_unit;
		float transmitter_offset[3];
		float transmitter_rotation[3];
		int axis_map[6];
		float axis_sign[6];
		int buffer_size;
		string constellation_file;
		char* buffer;
		char* cur_ptr;
		char* begin_ptr;
		char* end_ptr;
	};

private:
	device_state device;
	map<output_record_type, int> data_sizes;
	map<output_record_type, int> data_sizes_ascii;
	map<output_record_type, int> data_sizes_dtk_shm;

#ifdef DTK_ARCH_WIN32_VCPP
	OVERLAPPED io_overlapped;
#endif

#ifndef DTK_ARCH_WIN32_VCPP
	dtkVRCOSharedMem* VRCO;
#endif

	// Calculate the data sizes for each station
	int calcStationDataSize( const char& unit );

	// take raw input and add offset and orientation for both
	// transmitter and sensor
	int rawOffsetAndOrientation( station_state& station );

	// Clean up service before exiting
	void cleanUp();

	// Get the constellation path which includes DTK_SERVICE_PATH,
	// DTK_SERVICE_CONFIG_PATH, and the default service path.
	string getConstellationPath();

	// Determine what record needs to be interpreted and
	// pass the data to the appropriate function
	int interpretRecord( char* data_ptr, int size );
	// Convert data record into its components
	int interpretDataRecord( char* data_ptr, int size );

	// Records for all devices (Fastrak, IS900, etc)
	int interpretSystemStatusRecord( char* data_ptr, int size );
	int interpretOutputListRecord( char* data_ptr, int size );
	int interpretStationStatusRecord( char* data_ptr, int size );
	int interpretAlignmentReferenceFrameRecord( char* data_ptr, int size );
	int interpretBoresightReferenceAnglesRecord( char* data_ptr, int size );
	int interpretHemisphereRecord( char* data_ptr, int size );
	int interpretTipOffsetRecord( char* data_ptr, int size );
	int interpretOperationalEnvelopeRecord( char* data_ptr, int size );

	// Records to Intersense devices
	int interpretIntersenseSystemStatusRecord( char* data_ptr, int size );
	int interpretIntersenseStationRecord( char* data_ptr, int size );
	int interpretPredictionIntervalRecord( char* data_ptr, int size );
	int interpretSensitivityLevelRecord( char* data_ptr, int size );
	int interpretGenlockSynchronizationRecord( char* data_ptr, int size );
	int interpretGenlockPhaseRecord( char* data_ptr, int size );
	int interpretGenlockSyncSourceRecord( char* data_ptr, int size );
	int interpretConfigurationLockRecord( char* data_ptr, int size );
	int interpretEthernetAddressRecord( char* data_ptr, int size );
	int interpretEthernetStateRecord( char* data_ptr, int size );
	int interpretEthernetPortRecord( char* data_ptr, int size );

	// Records specific to IS900  models
	int interpretUltrasonicTimeoutRecord( char* data_ptr, int size );
	int interpretUltrasonicSensitivityRecord( char* data_ptr, int size );
	int interpretFixedPSERecord( char* data_ptr, int size );
	int interpretTrackingStatusRecord( char* data_ptr, int size );	
	int interpretBeaconSchedulerRecord( char* data_ptr, int size );

	int interpretCommandLoggingStateRecord( char* data_ptr, int size );
	int interpretCommandLogRecord( char* data_ptr, int size );
	int interpretErrorReportRecord( char* data_ptr, int size );

	// Convert numeric station number to alphanumeric station ID
	char getStationID( int station_num );
	// Convert alphanumeric station ID to numeric station number
	int getStationNum( char station_id );

	// Reads configuration file and sets appropriate options
	int configureIS900();
	// Read waiting data into buffer
	int readValue( char* buffer, int milli_seconds = 1000, bool show_errors = false );
	// Send the specified command
	int sendCommand( const string& cmd );

	int requestDataRecord();
	int sendOutputMode( bool polled );
	int requestAlignmentReferenceFrame( const char& unit );
	int sendAlignmentReferenceFrame( const char& unit, float Ox, float Oy, float Oz,
			float Xx, float Xy, float Xz, float Yx, float Yy, float Yz );
	int sendResetAlignmentReferenceFrame( const char& unit );
	int requestBoresightReferenceAngles( const char& unit );
	int sendBoresightReferenceAngles( const char& unit, float heading, float pitch,
			float roll );
	int sendBoresightCompatibilityMode( bool fastrak );
	int sendBoresight( const char& unit );
	int sendUnboresight( const char& unit );
	int sendHeadingBoresight( const char& unit );
	int sendHeadingUnboresight( const char& unit );
	int sendSetSerialCommunicationParameters( int rate, parity_type parity,
			bits_type bits, bool handshake );
	int requestSystemRecord();
	int requestStationStatus( const char& unit );
	int sendStationStatus( const char& unit, bool enabled );
	// If inches, then output units are in inches, else output units are in centimeters.
	int sendOutputUnitsControl( bool inches );
	int sendSaveCurrentSettings();
	int sendRestoreSettingsFactoryDefault();
	int sendFirmwareRestart();
	int sendSuspendDataTransmission();
	int sendResumeDataTransmission();
	// If ascii is true then output is in ascii format, wlse output will be in binary format.
	int sendOutputRecordMode( bool ascii );
	int requestOutputRecordList( const char& unit );
	int sendOutputRecordList( const char& unit,
			vector<output_record_type> output_record_list );
	int requestDefineTipOffsets( const char& unit );
	int sendDefineTipOffsets( const char& unit, float Ox, float Oy, float Oz );
	int requestPositionOperationalEnvelope( const char& unit );
	int sendPositionOperationalEnvelope( const char& unit, float Xmax, float Ymax,
			float Zmax, float Xmin, float Ymin, float Zmin );
	int requestHemisphere( const char& unit );
	int sendHemisphere( const char& unit, float p1, float p2, float p3 );
	// If milliseconds is true then the time stamp uses millisecond units, else the time
	// stamp uses microsecond units.
	int sendTimeUnits( bool milliseconds );
	int sendSetCurrentTimeZero();
	int requestEthernetAddress();
	int sendEthernetAddress( string address );
	int requestEthernetState();
	int sendEthernetState( bool enabled );
	int requestEthernetPort();
	int sendEthernetPort( int port );
	int requestIntersenseStatusRecord();
	int requestTrackingStatusRecord();
	// No effect on IS900
	int requestUltrasonicTimeoutInterval();
	// No effect on IS900
//	int sendUltrasonicTimeoutInterval( int interval );
	// No effect on IS900
	int requestUltrasonicReceiverSensitivity();
	// No effect on IS900
//	int sendUltrasonicReceiverSensitivity( int level );
	int requestGenlockSynchronization();
	int sendGenlockSynchronization( genlock_sync_type state, int rate );
	int requestGenlockPhase();
	int sendGenlockPhase( string percent );
	int requestGenlockSyncSource();
	int sendGenlockSyncSource( sync_source_type source );
	int requestConfigurationLock();
	int sendConfigurationLock( config_lock_type mode );
	int sendSonistripLEDControl( bool enabled );
	int requestBeaconScheduler();
	int sendBeaconScheduler( beacon_scheduler_type algorithm );
	int requestErrorReport();
	int sendClearErrorReport();
	int sendErrorReporting( bool enabled );
	int sendCommandLogging( bool enabled );
	int sendClearCommandLog();
	int requestCommandLogState();
	int requestCommandLog();
	int requestIntersenseStationStatusRecord( const char& unit );
	int requestPredictionInterval( const char& unit );
	int sendPredictionInterval( const char& unit, int interval );
	int sendPerceptualEnhancementLevel( const char& unit,
			perceptual_enhancement_type mode );
	int sendCompassHeadingCorrection( const char& unit,
			compass_heading_correction_type mode );
	int sendResetCompassHeadingCorrection( const char& unit );
	int requestRotationalSensitivity( const char& unit );
	int sendRotationalSensitivity( const char& unit,
			rotational_sensitivity_type level );
	int requestAssociateFixedPSEWithConstellation( const char& unit = 0 );
	int sendAssociateFixedPSEWithConstellation( const char& unit, float Px,
			float Py, float Pz, float Nx, float Ny, float Nz, int hardware_id );
	int sendDisassociateFixedPSEFromConstellation( const char& unit, int hardware_id );
	int sendClearAllFixedPSEsFromConstellation();
	int sendApplyConstellation();
	int sendCancelConstellationConfiguration();

	int sendConstellationFile( string& filename );

};

IS900::IS900( const char* arg )
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

IS900::~IS900()
{
	cleanUp();
}

void IS900::cleanUp()
{
#ifdef DTK_ARCH_WIN32_VCPP
	if( fd != NULL )
#else
	if( fd != -1 )
#endif
	{
		sendOutputMode( true );		
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

	map<int, station_state>::iterator station_itr = device.stations.begin();
	for( station_itr;station_itr!=device.stations.end();station_itr++ )
	{
		map<string, dtkSharedMem*>::const_iterator itr = station_itr->second.dtk_shm.begin();
		for( itr;itr!=station_itr->second.dtk_shm.end();itr++ )
		{
			if( itr->second )
				delete itr->second;
		}
	}

#ifdef DTK_ARCH_WIN32_VCPP
	CancelIo( fd );
	fd = NULL;
#else
	close( fd );
	fd = -1;
#endif
}

string IS900::getConstellationPath()
{
	char *config_env = getenv("DTK_SERVICE_CONFIG_PATH");
	char *svc_env = getenv("DTK_SERVICE_PATH");

	string constellation_path;
	if( config_env )
		constellation_path += string( config_env );
	if( svc_env )
	{
		if( constellation_path.size() )
			constellation_path += string( SEPARATOR );
		constellation_path += string( svc_env );
	}

	if( constellation_path.size() )
		constellation_path += string( SEPARATOR );
	constellation_path += ".";

	string service_dso = dtkConfig.getString( dtkConfigure::SERVICE_DSO_DIR );
	if( service_dso.size() )
	{
		if( constellation_path.size() )
			constellation_path += string( SEPARATOR );
		constellation_path += service_dso;
	}

	return constellation_path;
}

int IS900::calcStationDataSize( const char& unit )
{
	device.stations[unit].data_size = 3;
	for( unsigned int i=0;i<device.stations[unit].output_record_list.size();i++ )
	{
		switch( device.stations[unit].output_record_list[i] )
		{
			case OUTPUT_RECORD_SEPARATOR_SPACE:
			{
				device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_SEPARATOR_SPACE];
				break;
			}
			case OUTPUT_RECORD_SEPARATOR_CR_LF:
			{
				device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_SEPARATOR_CR_LF];
				break;
			}
			case OUTPUT_RECORD_POSITION_XYZ:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_POSITION_XYZ];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_POSITION_XYZ];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_HPR:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_HPR];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_ORIENTATION_HPR];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_X:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_X];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_X];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_Y:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_Y];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_Y];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_Z:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_Z];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_Z];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_QUATERNION:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_QUATERNION];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_ORIENTATION_QUATERNION];
				}
				break;
			}
			case OUTPUT_RECORD_POSITION_XYZ_16BIT:
			{
				device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_POSITION_XYZ_16BIT];
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_HPR_16BIT:
			{
				device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_ORIENTATION_HPR_16BIT];
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT:
			{
				device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT];
				break;
			}
			case OUTPUT_RECORD_TIMESTAMP:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_TIMESTAMP];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_TIMESTAMP];
				}
				break;
			}
			case OUTPUT_RECORD_BUTTONS:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_BUTTONS];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_BUTTONS];
				}
				break;
			}
			case OUTPUT_RECORD_JOYSTICK:
			{
				if( device.ascii )
				{
					device.stations[unit].data_size += data_sizes_ascii[OUTPUT_RECORD_JOYSTICK];
				}
				else
				{
					device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_JOYSTICK];
				}
				break;
			}
			case OUTPUT_RECORD_TRACKING_STATUS:
			{
				device.stations[unit].data_size += data_sizes[OUTPUT_RECORD_TRACKING_STATUS];
				break;
			}
		}
	}
	if( device.stations[unit].data_size > device.max_data_size )
		device.max_data_size = device.stations[unit].data_size;

	return 0;
}

int IS900::interpretRecord( char* data_ptr, int size )
{
	if( !strncmp( data_ptr, "NODATA", 6 ) )
		return 0;
	if( *data_ptr == '0' )
	{
		return interpretDataRecord( data_ptr, size );
	}
	else if( *data_ptr == '2' )
	{
		switch( *( data_ptr + 2 ) )
		{
			case 'S':
			{
				return interpretSystemStatusRecord( data_ptr, size );
				break;
			}
			case 'O':
			{
				return interpretOutputListRecord( data_ptr, size );
				break;
			}
			case 'I':
			{
				return interpretStationStatusRecord( data_ptr, size );
				break;
			}
			case 'A':
			{
				return interpretAlignmentReferenceFrameRecord( data_ptr, size );
				break;
			}
			case 'G':
			{
				return interpretBoresightReferenceAnglesRecord( data_ptr, size );
				break;
			}
			case 'H':
			{
				return interpretHemisphereRecord( data_ptr, size );
				break;
			}
			case 'N':
			{
				return interpretTipOffsetRecord( data_ptr, size );
				break;
			}
			case 'V':
			{
				return interpretOperationalEnvelopeRecord( data_ptr, size );
				break;
			}
			case 'E':
			{
				return interpretErrorReportRecord( data_ptr, size );
				break;
			}
			default:
			{
				dtkMsg.add( DTKMSG_ERROR, "IS900::interpretRecord error - the Fastrak status record\n"
					"\treceived has an invalid sub-record type \"%c\".\n",
					*( data_ptr + 2 ) );
				return -1;
				break;
			}
		}
	}
	else if( *data_ptr == '3' )
	{
		switch( *( data_ptr + 2 ) )
		{
			case 'G':
			{
				if( !strncmp( data_ptr + 2, "GP", 2 ) )
				{
					return interpretGenlockPhaseRecord( data_ptr, size );
					break;
				}
				else if( !strncmp( data_ptr + 2, "GS", 2 ) )
				{
					return interpretGenlockSyncSourceRecord( data_ptr, size );
					break;
				}
				else if( !strncmp( data_ptr + 2, "G", 1 ) )
				{
					return interpretGenlockSynchronizationRecord( data_ptr, size );
					break;
				}
			}
			case 'S':
			{
				if( !strncmp( data_ptr + 2, "SchAlg", 6 ) )
				{
					return interpretBeaconSchedulerRecord( data_ptr, size );
					break;
				}
				else if( !strncmp( data_ptr + 2, "S", 1 ) )
				{
					return interpretIntersenseSystemStatusRecord( data_ptr, size );
					break;
				}
			}
			case 's':
			{
				return interpretIntersenseStationRecord( data_ptr, size );
				break;
			}
			case 'p':
			{
				return interpretPredictionIntervalRecord( data_ptr, size );
				break;
			}
			case 'Q':
			{
				return interpretSensitivityLevelRecord( data_ptr, size );
				break;
			}
			case 'U':
			{
				return interpretUltrasonicTimeoutRecord( data_ptr, size );
				break;
			}
			case 'g':
			{
				return interpretUltrasonicSensitivityRecord( data_ptr, size );
				break;
			}
			case 'F':
			{
				return interpretFixedPSERecord( data_ptr, size );
				break;
			}
			case 'P':
			{
				return interpretTrackingStatusRecord( data_ptr, size );
				break;
			}
			case 'L':
			{
				if( !strncmp( data_ptr + 2, "LS", 2 ) )
				{
					return interpretCommandLoggingStateRecord( data_ptr, size );
					break;
				}
				else if( !strncmp( data_ptr + 2, "LF", 2 ) )
				{
					return interpretCommandLogRecord( data_ptr, size );
					break;
				}
			}
			case 'E':
			{
				if( !strncmp( data_ptr + 2, "EI", 2 ) )
				{
					return interpretEthernetAddressRecord( data_ptr, size );
					break;
				}
				else if( !strncmp( data_ptr + 2, "EUP", 3 ) )
				{
					return interpretEthernetPortRecord( data_ptr, size );
					break;
				}
				else if( !strncmp( data_ptr + 2, "EU", 2 ) )
				{
					return interpretEthernetStateRecord( data_ptr, size );
					break;
				}
				else if( !strncmp( data_ptr + 2, "E", 1 ) )
				{
					return interpretErrorReportRecord( data_ptr, size );
					break;
				}
			}
			case 'C':
			{
				if( !strncmp( data_ptr + 2, "CLM", 3 ) )
				{
					break;
				}
			}
			default:
			{
				dtkMsg.add( DTKMSG_ERROR, "IS900::interpretRecord error - the Intersense manufacturer-specific\n"
					"\tstatus record is invalid \"%s\".\n",
					data_ptr );
				return -1;
				break;
			}
		}
	}
	else
	{
		char temp[TEMP_BUFFER_SIZE];
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretRecord error - unrecognized record header.\n" );
		for( int i=-64;i<0;i++ )
		{
			sprintf( temp + 4 * ( i + 64 ), "%4d", (unsigned char)*( data_ptr + i ) );
		}
		dtkMsg.add( DTKMSG_ERROR, "BEFORE device.begin_ptr - Data: %s\n", temp );
		for( int i=0;i<size;i++ )
		{
			sprintf( temp + 4 * i, "%4d", (unsigned char)*( data_ptr + i ) );
		}
		dtkMsg.add( DTKMSG_ERROR, "Pointer: %p    Size: %d    Data: %s\n", data_ptr, size, temp );
		if( device.begin_ptr == data_ptr )
			device.begin_ptr += size;

		return -3;
/*
		char* cur_ptr = data_ptr;
		int index = 0;
		while( *cur_ptr != '\r' && *( cur_ptr + 1 ) != '\n' &&
				cur_ptr < device.end_ptr )
		{
			sprintf( temp + index, "%4d", (unsigned char)*cur_ptr );
			cur_ptr++;
			index += 4;
		}
		temp[index] = '\0';
		dtkMsg.append( "size: %d    device.begin_ptr: %p    device.cur_ptr: %p    device.end_ptr: %p\n",
				size, device.begin_ptr, device.cur_ptr, device.end_ptr );
		dtkMsg.append( "Pointer: %p    Data: %s\n", data_ptr, temp );
		dtkMsg.append( "device.begin_ptr: %p    device.cur_ptr: %p    device.end_ptr: %p\n",
				device.begin_ptr, device.cur_ptr, device.end_ptr );
*/
	}

	return 0;
}

int IS900::interpretDataRecord( char* data_ptr, int size )
{
	char temp[TEMP_BUFFER_SIZE];
	char* cur_ptr = data_ptr + 1;
	int station_num = getStationNum( *cur_ptr );

	if( device.stations.find( station_num ) == device.stations.end() )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretDataRecord invalid station_num %d\n",
				station_num );
	}
	else
	{
		if( size < device.stations[station_num].data_size )
		{
			return -2;
		}
	}

	cur_ptr++;

	if( *cur_ptr != ' ' )
	{
		dtkMsg.add( DTKMSG_WARNING, "The data record status byte is \"%c\" instead of a space.\n", *cur_ptr );
	}
	cur_ptr++;

	for( unsigned int i=0;i<device.stations[station_num].output_record_list.size();i++ )
	{
		switch( device.stations[station_num].output_record_list[i] )
		{
			case OUTPUT_RECORD_SEPARATOR_SPACE:
			{
				if( *cur_ptr != ' ' )
				{
					dtkMsg.add( DTKMSG_WARNING, "IS900::interpretDataRecord - expected OUTPUT_RECORD_SEPARATOR_SPACE \" \" but got \"%c\".\n", *cur_ptr );
					return -1;
				}
				cur_ptr += data_sizes[OUTPUT_RECORD_SEPARATOR_SPACE];
				break;
			}
			case OUTPUT_RECORD_SEPARATOR_CR_LF:
			{
				if( *cur_ptr != '\r' && *( cur_ptr + 1 ) != '\n' )
				{
					dtkMsg.add( DTKMSG_WARNING, "IS900::interpretDataRecord - expected OUTPUT_RECORD_SEPARATOR_CR_LF \"\\r\\n\" but got \"%c%c\".\n", *cur_ptr, *( cur_ptr + 1 ) );
					return -1;
				}
				cur_ptr += data_sizes[OUTPUT_RECORD_SEPARATOR_CR_LF];
				break;
			}
			case OUTPUT_RECORD_POSITION_XYZ:
			{
				for( int j=0;j<3;j++ )
				{
					if( device.ascii )
					{
						strncpy( temp, cur_ptr, 7 );
						temp[7] = '\0';
						sscanf( temp, "%f", &device.stations[station_num].position[j] );
						cur_ptr += data_sizes_ascii[OUTPUT_RECORD_POSITION_XYZ] / 3;
					}
					else
					{
						device.stations[station_num].position[j] = *(float*)cur_ptr;
						cur_ptr += data_sizes[OUTPUT_RECORD_POSITION_XYZ] / 3;
					}
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_HPR:
			{
				for( int j=0;j<3;j++ )
				{
					if( device.ascii )
					{
						strncpy( temp, cur_ptr, 7 );
						temp[7] = '\0';
						sscanf( temp, "%f", &device.stations[station_num].orientation_hpr[j] );
						cur_ptr += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_HPR] / 3;
					}
					else
					{
						device.stations[station_num].orientation_hpr[j] = *(float*)cur_ptr;
						cur_ptr += data_sizes[OUTPUT_RECORD_ORIENTATION_HPR] / 3;
					}
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_X:
			{
				for( int j=0;j<3;j++ )
				{
					if( device.ascii )
					{
						strncpy( temp, cur_ptr, 7 );
						temp[7] = '\0';
						sscanf( temp, "%f", &device.stations[station_num].cosines_x[j] );
						cur_ptr += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_X] / 3;
					}
					else
					{
						device.stations[station_num].cosines_x[j] = *(float*)cur_ptr;
						cur_ptr += data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_X] / 3;
					}
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_Y:
			{
				for( int j=0;j<3;j++ )
				{
					if( device.ascii )
					{
						strncpy( temp, cur_ptr, 7 );
						temp[7] = '\0';
						sscanf( temp, "%f", &device.stations[station_num].cosines_y[j] );
						cur_ptr += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_Y] / 3;
					}
					else
					{
						device.stations[station_num].cosines_y[3+j] = *(float*)cur_ptr;
						cur_ptr += data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_Y] / 3;
					}
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_Z:
			{
				for( int j=0;j<3;j++ )
				{
					if( device.ascii )
					{
						strncpy( temp, cur_ptr, 7 );
						temp[7] = '\0';
						sscanf( temp, "%f", &device.stations[station_num].cosines_z[j] );
						cur_ptr += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_Z] / 3;
					}
					else
					{
						device.stations[station_num].cosines_z[6+j] = *(float*)cur_ptr;
						cur_ptr += data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_Z] / 3;
					}
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_QUATERNION:
			{
				for( int j=0;j<4;j++ )
				{
					if( device.ascii )
					{
						strncpy( temp, cur_ptr, 7 );
						temp[7] = '\0';
						sscanf( temp, "%f", &device.stations[station_num].quaternion[j] );
						cur_ptr += data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_QUATERNION] / 4;
					}
					else
					{
						device.stations[station_num].quaternion[j] = *(float*)cur_ptr;
						cur_ptr += data_sizes[OUTPUT_RECORD_ORIENTATION_QUATERNION] / 4;
					}
				}
				break;
			}
			case OUTPUT_RECORD_POSITION_XYZ_16BIT:
			{
				for( int j=0;j<3;j++ )
				{
					char lo = *cur_ptr & 0x007f;
					char hi = *( cur_ptr + 1 ) & 0x007f;
					short value = ( lo << 2 ) | ( hi << 9 );
					if( device.inches )
						device.stations[station_num].position[j] = value * 300.0f / 32768.0f;
					else
						device.stations[station_num].position[j] = value * 118.11f / 32768.0f;
					cur_ptr += data_sizes[OUTPUT_RECORD_POSITION_XYZ_16BIT] / 3;
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_HPR_16BIT:
			{
				for( int j=0;j<3;j++ )
				{
					char lo = *cur_ptr & 0x007f;
					char hi = *( cur_ptr + 1 ) & 0x007f;
					short value = ( lo << 2 ) | ( hi << 9 );
					if( device.stations[station_num].angle_degrees )
						device.stations[station_num].orientation_hpr[j] = value * 180.0f / 32768.0f;
					else
						device.stations[station_num].orientation_hpr[j] = value * (float)M_PI_2 / 32768.0f;
					cur_ptr += data_sizes[OUTPUT_RECORD_ORIENTATION_HPR_16BIT] / 3;
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT:
			{
				for( int j=0;j<4;j++ )
				{
					char lo = *cur_ptr & 0x007f;
					char hi = *( cur_ptr + 1 ) & 0x007f;
					short value = ( lo << 2 ) | ( hi << 9 );
					device.stations[station_num].quaternion[j] = value * 1.0f / 32768.0f;
					cur_ptr += data_sizes[OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT] / 4;
				}
				break;
			}
			case OUTPUT_RECORD_TIMESTAMP:
			{
				if( device.ascii )
				{
					strncpy( temp, cur_ptr, 14 );
					temp[14] = '\0';
					sscanf( temp, "%f", &device.stations[station_num].timestamp );
					cur_ptr += data_sizes_ascii[OUTPUT_RECORD_TIMESTAMP];
				}
				else
				{
					device.stations[station_num].timestamp = *(float*)cur_ptr;
					cur_ptr += data_sizes[OUTPUT_RECORD_TIMESTAMP];
				}
				break;
			}
			case OUTPUT_RECORD_BUTTONS:
			{
				if( device.ascii )
				{
					strncpy( temp, cur_ptr, 5 );
					temp[5] = '\0';
					sscanf( temp, "%d", &device.stations[station_num].buttons );
					cur_ptr += data_sizes_ascii[OUTPUT_RECORD_BUTTONS];
				}
				else
				{
					device.stations[station_num].buttons = *cur_ptr;
					cur_ptr += data_sizes[OUTPUT_RECORD_BUTTONS];
				}
				break;
			}
			case OUTPUT_RECORD_JOYSTICK:
			{
				unsigned char js[2];
				for( int j=0;j<2;j++ )
				{
					if( device.ascii )
					{
						strncpy( temp, cur_ptr, 4 );
						temp[4] = '\0';
						sscanf( temp, "%d", &js[j] );
						cur_ptr += data_sizes_ascii[OUTPUT_RECORD_JOYSTICK] / 2;
					}
					else
					{
						js[j] = *cur_ptr;
						cur_ptr += data_sizes[OUTPUT_RECORD_JOYSTICK] / 2;
					}
					if( js[j] < 127 )
					{
						device.stations[station_num].joystick[j] = ( (float)js[j] - 127.0f ) / 127.0f;
					}
					else
					{
						device.stations[station_num].joystick[j] = ( (float)js[j] - 127.0f ) / 128.0f;
					}
				}
				break;
			}
			case OUTPUT_RECORD_TRACKING_STATUS:
			{
				device.stations[station_num].status = *cur_ptr;
				cur_ptr += data_sizes[OUTPUT_RECORD_TRACKING_STATUS];
				break;
			}
		}
	}

	for( unsigned int i=0;i<device.stations[station_num].output_record_list.size();i++ )
	{
		switch( device.stations[station_num].output_record_list[i] )
		{
			case OUTPUT_RECORD_SEPARATOR_SPACE:
			case OUTPUT_RECORD_SEPARATOR_CR_LF:
			case OUTPUT_RECORD_TIMESTAMP:
			case OUTPUT_RECORD_TRACKING_STATUS:
			{
				break;
			}
			case OUTPUT_RECORD_POSITION_XYZ:
			case OUTPUT_RECORD_POSITION_XYZ_16BIT:
			{
				for( int j=0;j<3;j++ )
				{
					device.stations[station_num].mapped_position[j] =
							device.axis_sign[j] *
							device.stations[station_num].position[device.axis_map[j]];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_HPR:
			case OUTPUT_RECORD_ORIENTATION_HPR_16BIT:
			{
				for( int j=0;j<3;j++ )
				{
					device.stations[station_num].mapped_orientation_hpr[j] =
							device.axis_sign[j+3] *
							device.stations[station_num].orientation_hpr[device.axis_map[j+3]];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_X:
			{
				for( int j=0;j<3;j++ )
				{
					device.stations[station_num].mapped_cosines_x[j] =
							device.stations[station_num].cosines_x[j];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_Y:
			{
				for( int j=0;j<3;j++ )
				{
					device.stations[station_num].mapped_cosines_y[j] =
							device.stations[station_num].cosines_y[j];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_Z:
			{
				for( int j=0;j<3;j++ )
				{
					device.stations[station_num].mapped_cosines_y[j] =
							device.stations[station_num].cosines_y[j];
				}
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_QUATERNION:
			case OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT:
			{
				for( int j=0;j<4;j++ )
				{
					device.stations[station_num].mapped_quaternion[j] =
							device.stations[station_num].quaternion[j];
				}
				break;
			}
			case OUTPUT_RECORD_BUTTONS:
			{
				int cur_bit = 1;
				device.stations[station_num].mapped_buttons = 0;
				for( int j=0;j<8;j++ )
				{
					int mapped_bit = 1;
					int cur_button = device.stations[station_num].button_map[j];
					for( int k=1;k<cur_button;k++ )
						mapped_bit *= 2;
					if( cur_bit & device.stations[station_num].buttons )
						device.stations[station_num].mapped_buttons |= mapped_bit;
					cur_bit *= 2;
				}
				break;
			}
			case OUTPUT_RECORD_JOYSTICK:
			{
				for( int j=0;j<2;j++ )
				{
					device.stations[station_num].mapped_joystick[device.stations[station_num].joystick_map[j]] =
							device.stations[station_num].joystick_sign[j] *
							device.stations[station_num].joystick[j];
				}
				break;
			}
		}
	}

	rawOffsetAndOrientation( device.stations[station_num] );

	map<string, vector<output_record_type> >::iterator itr =
			device.stations[station_num].dtk_shm_map.begin();
	for( itr;itr!=device.stations[station_num].dtk_shm_map.end();itr++ )
	{
		char* temp_ptr = temp;
		
		for( unsigned int j=0;j<itr->second.size();j++ )
		{
			switch( itr->second[j] )
			{
				case OUTPUT_RECORD_SEPARATOR_SPACE:
				case OUTPUT_RECORD_SEPARATOR_CR_LF:
					break;
				case OUTPUT_RECORD_POSITION_XYZ:
				case OUTPUT_RECORD_POSITION_XYZ_16BIT:
				{
					for( int k=0;k<3;k++ )
					{
						*(float*)temp_ptr = device.stations[station_num].mapped_position[k] /
								( device.diverse_unit * 100.0f );
						temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_POSITION_XYZ] / 3;
					}
					break;
				}
				case OUTPUT_RECORD_ORIENTATION_HPR:
				case OUTPUT_RECORD_ORIENTATION_HPR_16BIT:
				{
					for( int k=0;k<3;k++ )
					{
						*(float*)temp_ptr = device.stations[station_num].mapped_orientation_hpr[k];
						temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_HPR] / 3;
					}
					break;
				}
				case OUTPUT_RECORD_ORIENTATION_COSINES_X:
				{
					for( int k=0;k<3;k++ )
					{
						*(float*)temp_ptr = device.stations[station_num].mapped_cosines_x[k];
						temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_COSINES_X] / 3;
					}
					break;
				}
				case OUTPUT_RECORD_ORIENTATION_COSINES_Y:
				{
					for( int k=0;k<3;k++ )
					{
						*(float*)temp_ptr = device.stations[station_num].mapped_cosines_y[k];
						temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_COSINES_Y] / 3;
					}
					break;
				}
				case OUTPUT_RECORD_ORIENTATION_COSINES_Z:
				{
					for( int k=0;k<3;k++ )
					{
						*(float*)temp_ptr = device.stations[station_num].mapped_cosines_z[k];
						temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_COSINES_Z] / 3;
					}
					break;
				}
				case OUTPUT_RECORD_ORIENTATION_QUATERNION:
				case OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT:
				{
					for( int k=0;k<4;k++ )
					{
						*(float*)temp_ptr = device.stations[station_num].mapped_quaternion[k];
						temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_QUATERNION] / 4;
					}
					break;
				}
				case OUTPUT_RECORD_TIMESTAMP:
				{
					*(float*)temp_ptr = device.stations[station_num].timestamp;
					temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_TIMESTAMP];
					break;
				}
				case OUTPUT_RECORD_BUTTONS:
				{
					*temp_ptr = device.stations[station_num].mapped_buttons;
					temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_BUTTONS];
					break;
				}
				case OUTPUT_RECORD_JOYSTICK:
				{
					for( int k=0;k<2;k++ )
					{
						*(float*)temp_ptr = device.stations[station_num].mapped_joystick[k];
						temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_JOYSTICK] / 2;
					}
					break;
				}
				case OUTPUT_RECORD_TRACKING_STATUS:
				{
					*temp_ptr = device.stations[station_num].status;
					temp_ptr += data_sizes_dtk_shm[OUTPUT_RECORD_TRACKING_STATUS];
					break;
				}
			}
		}

		device.stations[station_num].dtk_shm[itr->first]->write( temp );
	}

	return 0;
}

int IS900::interpretSystemStatusRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr;

	struct system_status_record record;
	cur_ptr += 5;
	record.ascii = !( *cur_ptr && 0x01 );
	record.inches = !( *cur_ptr && 0x02 );
	record.polled = !( *cur_ptr && 0x04 );
	cur_ptr++;

	for( int i=0;i<3;i++ )
	{
		record.bit_error[i] = *( cur_ptr + i );
	}
	cur_ptr += 3;

	for( int i=0;i<6;i++ )
	{
		record.blank[i] = *( cur_ptr + i );
	}
	cur_ptr += 6;

	char* temp = new char[33];
	strncpy( temp, cur_ptr, 6 );
	temp[6] = '\0';
	record.firmware_version_id = temp;
	cur_ptr += 6;

	strncpy( temp, cur_ptr, 32 );
	temp[32] = '\0';
	record.system_identification = temp;
	delete [] temp;
	cur_ptr += 32;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid system_status_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "System Status Record\n"
			"\toutput format (ascii or binary): %s\n"
			"\toutput units (inches or centimeters): %s\n"
			"\ttransmit mode (polled or continuous): %s\n"
			"\tbit error: %c%c%c\n"
			"\tfirmware version id: %s\n"
			"\tsystem identification: %s\n",
			record.ascii ? "ascii" : "binary",
			record.inches ? "inches" : "centimeters",
			record.polled ? "polled" : "continuous",
			record.bit_error[0], record.bit_error[1], record.bit_error[2],
			record.firmware_version_id.c_str(),
			record.system_identification.c_str() );

	return 0;
}

int IS900::interpretOutputListRecord( char* data_ptr, int size )
{
	struct output_list_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;

	output_record_type parameter;
	while( *cur_ptr != '\r' && *( cur_ptr + 1 ) != '\n' )
	{
		string value_str;
		value_str.append( cur_ptr, 0, 2 );
		sscanf( value_str.c_str(), "%2d", &parameter );
		record.parameters.push_back( parameter );
		cur_ptr += 2;
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid system_status_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Output List Record\n"
			"Station: %d\n",
			getStationNum( record.station_id ) );
	for( unsigned int i=0;i<record.parameters.size();i++ )
	{
		switch( record.parameters[i] )
		{
			case OUTPUT_RECORD_SEPARATOR_SPACE:
			{
				dtkMsg.append( "\tSEPARATOR_SPACE\n" );
				break;
			}
			case OUTPUT_RECORD_SEPARATOR_CR_LF:
			{
				dtkMsg.append( "\tSEPARATOR_CR_LF\n" );
				break;
			}
			case OUTPUT_RECORD_POSITION_XYZ:
			{
				dtkMsg.append( "\tPOSITION_XYZ\n" );
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_HPR:
			{
				dtkMsg.append( "\tORIENTATION_HPR\n" );
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_X:
			{
				dtkMsg.append( "\tORIENTATION_COSINES_X\n" );
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_Y:
			{
				dtkMsg.append( "\tORIENTATION_COSINES_Y\n" );
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_COSINES_Z:
			{
				dtkMsg.append( "\tORIENTATION_COSINES_Z\n" );
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_QUATERNION:
			{
				dtkMsg.append( "\tORIENTATION_QUATERNION\n" );
				break;
			}
			case OUTPUT_RECORD_POSITION_XYZ_16BIT:
			{
				dtkMsg.append( "\tPOSITION_XYZ_16BIT\n" );
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_HPR_16BIT:
			{
				dtkMsg.append( "\tORIENTATION_HPR_16BIT\n" );
				break;
			}
			case OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT:
			{
				dtkMsg.append( "\tORIENTATION_QUATERNION_16BIT\n" );
				break;
			}
			case OUTPUT_RECORD_TIMESTAMP:
			{
				dtkMsg.append( "\tTIMESTAMP\n" );
				break;
			}
			case OUTPUT_RECORD_BUTTONS:
			{
				dtkMsg.append( "\tBUTTONS\n" );
				break;
			}
			case OUTPUT_RECORD_JOYSTICK:
			{
				dtkMsg.append( "\tJOYSTICK\n" );
				break;
			}
			case OUTPUT_RECORD_TRACKING_STATUS:
			{
				dtkMsg.append( "\tTRACKING_STATUS\n" );
				break;
			}
			default:
			{
				dtkMsg.append( "\tUKNOWN OUTPUT PARAMETER\n" );
				break;
			}
		}
	}

	return 0;
}

int IS900::interpretStationStatusRecord( char* data_ptr, int size )
{
	struct station_status_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;

	sscanf( cur_ptr, "%1d", &record.state );
	cur_ptr += 4;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid station_status_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Station Status Record\n"
			"\tStation %d\n"
			"\tStatus: %s",
			getStationNum( record.station_id ),
			record.state ? "on" : "off" );

	return 0;
}

int IS900::interpretAlignmentReferenceFrameRecord( char* data_ptr, int size )
{
	struct alignment_reference_frame_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;
	string value_str;

	for( int i=0;i<3;i++ )
	{
		value_str.clear();
		value_str.append( cur_ptr, 0, 7 );
		sscanf( value_str.c_str(), "%f", &record.origin[i] );
		cur_ptr += 7;
	}

	for( int i=0;i<3;i++ )
	{
		value_str.clear();
		value_str.append( cur_ptr, 0, 7 );
		sscanf( value_str.c_str(), "%f", &record.axis_x[i] );
		cur_ptr += 7;
	}

	for( int i=0;i<3;i++ )
	{
		value_str.clear();
		value_str.append( cur_ptr, 0, 7 );
		sscanf( value_str.c_str(), "%f", &record.axis_y[i] );
		cur_ptr += 7;
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid alignment_reference_frame_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Alignment Reference Frame Record\n"
			"\tStation: %d\n"
			"\tOrigin: %7.2f    %7.2f    %7.2f\n"
			"\tAxis X: %7.2f    %7.2f    %7.2f\n"
			"\tAxis Y: %7.2f    %7.2f    %7.2f\n",
			getStationNum( record.station_id ),
			record.origin[0], record.origin[1], record.origin[2],
			record.axis_x[0], record.axis_x[1], record.axis_x[2],
			record.axis_y[0], record.axis_y[1], record.axis_y[2] );

	return 0;
}

int IS900::interpretBoresightReferenceAnglesRecord( char* data_ptr, int size )
{
	struct boresight_reference_angles_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;

	for( int i=0;i<3;i++ )
	{
		sscanf( cur_ptr, "%f", &record.hpr[i] );
		cur_ptr += 7;
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid boresight_reference_angles_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Boresight Reference Angles Record\n"
			"\tStation: %d"
			"\tHPR: %7.2f    %7.2f    %7.2f\n",
			getStationNum( record.station_id ),
			record.hpr[0], record.hpr[1], record.hpr[2] );

	return 0;
}

int IS900::interpretHemisphereRecord( char* data_ptr, int size )
{
	struct hemisphere_record record;
	char* cur_ptr = data_ptr + 3;

	for( int i=0;i<3;i++ )
	{
		sscanf( cur_ptr, "%f", &record.vector[i] );
		cur_ptr += 7;
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid hemisphere_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Hemisphere Record\n"
			"\tVector: %7.2f    %7.2f    %7.2f\n",
			record.vector[0], record.vector[1], record.vector[2] );

	return 0;
}

int IS900::interpretTipOffsetRecord( char* data_ptr, int size )
{
	struct tip_offset_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;

	for( int i=0;i<3;i++ )
	{
		sscanf( cur_ptr, "%f", &record.offset[i] );
		cur_ptr += 7;
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid tip_offset_record.\n" );
		return -1;
	}

	dtkMsg.add( DTKMSG_NOTICE, "Tip Offset Record\n"
			"\tStation: %d\n"
			"\tTip offset: %7.2f    %7.2f    %7.2f\n",
			getStationNum( record.station_id ),
			record.offset[0], record.offset[1], record.offset[2] );

	return 0;
}

int IS900::interpretOperationalEnvelopeRecord( char* data_ptr, int size )
{
	struct position_operational_envelope_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;

	for( int i=0;i<3;i++ )
	{
		sscanf( cur_ptr, "%f", &record.max[i] );
		cur_ptr += 7;
	}

	for( int i=0;i<3;i++ )
	{
		sscanf( cur_ptr, "%f", &record.min[i] );
		cur_ptr += 7;
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid position_operational_envelope_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Position Operational Envelope Record\n"
			"\tStation: %d\n"
			"\tMin: %7.2f    %7.2f    %7.2f\n"
			"\tMax: %7.2f    %7.2f    %7.2f\n",
			getStationNum( record.station_id ),
			record.min[0], record.min[1], record.min[2],
			record.max[0], record.max[1], record.max[2] );

	return 0;
}

int IS900::interpretIntersenseSystemStatusRecord( char* data_ptr, int size )
{
	struct intersense_system_status_record record;
	char* cur_ptr = data_ptr + 3;

	char config;
	sscanf( cur_ptr + 2, "%c", &config );
	record.fastrak = config & 0x02;
	record.milliseconds = config & 0x04;
	record.led_control = config & 0x08;
	cur_ptr += 3;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid intersense_system_status_record.\n" );
		return -1;
	}

	dtkMsg.add( DTKMSG_NOTICE, "Intersense System Status Record\n"
			"\tFastrak Compatibility: %s\n"
			"\tTime units: %s\n"
			"\tReceiver Pod LEDs: %s\n",
			record.fastrak ? "FASTRAK" : "Firmware Version 2.x",
			record.milliseconds ? "milliseconds" : "microseconds",
			record.led_control ? "on" : "off" );

	return 0;
}

int IS900::interpretIntersenseStationRecord( char* data_ptr, int size )
{
	struct intersense_station_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;

	char config;
	sscanf( cur_ptr + 1, "%c", &config );
	record.enhancement_level = (perceptual_enhancement_type)( config & 0x03 );
	string perceptual_enhancement_level;
	switch( record.enhancement_level )
	{
		case PERCEPTUAL_ENHANCEMENT_DRIFT:
		{
			perceptual_enhancement_level = "drift";
			break;
		}
		case PERCEPTUAL_ENHANCEMENT_DRIFT_JITTER:
		{
			perceptual_enhancement_level = "drift and jitter";
			break;
		}
		case PERCEPTUAL_ENHANCEMENT_IMMERSIVE:
		{
			perceptual_enhancement_level = "immersive";
			break;
		}
	}

	sscanf( cur_ptr + 2, "%c", &config );
	record.compass_mode = (compass_heading_correction_type)( config & 0x06 );
	string compass_heading_correction;
	switch( record.compass_mode )
	{
		case COMPASS_CORRECTION_OFF:
		{
			compass_heading_correction = "off";
			break;
		}
		case COMPASS_CORRECTION_PARTIAL:
		{
			compass_heading_correction = "partial";
			break;
		}
		case COMPASS_CORRECTION_FULL:
		{
			compass_heading_correction = "full";
			break;
		}
	}
	cur_ptr += 3;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid intersense_station_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Intersense Station Record\n"
			"\tStation: %d\n"
			"\tPerceptual Enhancement Level: %s\n"
			"\tCompass Mode: %s\n",
			getStationNum( record.station_id ),
			perceptual_enhancement_level.c_str(),
			compass_heading_correction.c_str() );

	return 0;
}

int IS900::interpretPredictionIntervalRecord( char* data_ptr, int size )
{
	struct prediction_interval_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;

	sscanf( cur_ptr, "%2d", &record.interval );
	cur_ptr++;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid prediction_interval_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Prediction Interval Record\n"
			"\tStation: %d\n"
			"\tPrediction Interval: %d\n",
			getStationNum( record.station_id ),
			record.interval );

	return 0;
}

int IS900::interpretSensitivityLevelRecord( char* data_ptr, int size )
{
	struct sensitivity_level_record record;
	char* cur_ptr = data_ptr + 1;
	sscanf( cur_ptr, "%c", &record.station_id );
	cur_ptr += 2;

	sscanf( cur_ptr, "%1d", &record.sensitivity_level );
	cur_ptr++;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid sensitivity_level_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Sensitivity Level Record\n"
			"\tStation: %d\n"
			"\tSensitivity Level: %d\n",
			getStationNum( record.station_id ),
			record.sensitivity_level );

	return 0;
}

int IS900::interpretGenlockSynchronizationRecord( char* data_ptr, int size )
{
	struct genlock_synchronization_record record;
	char* cur_ptr = data_ptr + 3;

	sscanf( cur_ptr, "%1d", &record.state );
	cur_ptr++;

	sscanf( cur_ptr, "%f", &record.rate );
	cur_ptr += 7;

	sscanf( cur_ptr, "%3d", &record.cycles );
	cur_ptr += 3;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid genlock_synchronization_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	string state;
	switch( record.state )
	{
		case GENLOCK_SYNC_OFF:
		{
			state = "off";
			break;
		}
		case GENLOCK_SYNC_EXTERNAL:
		{
			state = "external sync";
			break;
		}
		case GENLOCK_SYNC_INTERNAL:
		{
			state = "internal sync";
			break;
		}
		default:
		{
			state = "invalid state";
		}
	}

	dtkMsg.add( DTKMSG_NOTICE, "Genlock Synchronization Record\n"
			"\tState: %s\n"
			"\tRate: %7.2f\n"
			"\tStrobe rate: %d\n",
			state.c_str(), record.rate, record.cycles );

	return 0;
}

int IS900::interpretGenlockPhaseRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 4;

	int phase;
	sscanf( cur_ptr, "%7d", &phase );
	cur_ptr += 7;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid ethernet address record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Genlock Phase Record\n"
			"\tphase: %d\n",
			phase );

	return 0;
}

int IS900::interpretGenlockSyncSourceRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 4;

	int sync_source;
	sscanf( cur_ptr, "%1d", &sync_source );
	cur_ptr++;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid ethernet address record.\n" );
		return -1;
	}
	cur_ptr += 2;

	string source;
	if( sync_source == 1 )
		source = "TTL";
	else if( sync_source == 2 )
		source = "NTSC";
	else
		source = "Unknown";

	dtkMsg.add( DTKMSG_NOTICE, "Genlock Sync Source Record\n"
			"\tsync source: %s\n",
			source.c_str() );

	return 0;
}

int IS900::interpretConfigurationLockRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 5;

	int lock_mode;
	sscanf( cur_ptr, "%1d", &lock_mode );
	cur_ptr++;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid ethernet address record.\n" );
		return -1;
	}
	cur_ptr += 2;

	string mode;
	if( lock_mode == 0 )
		mode = "lock off";
	else if( lock_mode == 1 )
		mode = "lock saved settings";
	else if( lock_mode == 2 )
		mode = "lock saved and session settings";
	else
		mode = "Unknown";

	dtkMsg.add( DTKMSG_NOTICE, "Configuration Lock Record\n"
			"\tmode: %s\n",
			mode.c_str() );

	return 0;
}

int IS900::interpretEthernetAddressRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 4;

	string ip_address;
	while( *( cur_ptr ) != '\r' && *( cur_ptr + 1 ) != '\n' )
	{
		ip_address.append( 1, *cur_ptr );
		cur_ptr++;
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid ethernet address record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Ethernet Address Record\n"
			"\tIP address: %s\n",
			ip_address.c_str() );

	return 0;
}

int IS900::interpretEthernetStateRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 4;

	int state;
	sscanf( cur_ptr, "%1d", &state );
	cur_ptr++;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid ethernet state record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Ethernet State Record\n"
			"\tState: %s\n",
			state ? "enabled" : "disabled" );

	return 0;
}

int IS900::interpretEthernetPortRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 5;

	string port;
	while( *( cur_ptr ) != '\r' && *( cur_ptr + 1 ) != '\n' )
	{
		port.append( 1, *cur_ptr );
		cur_ptr++;
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid ethernet port record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Ethernet Port Record\n"
			"\tport: %s\n",
			port.c_str() );

	return 0;
}

int IS900::interpretUltrasonicTimeoutRecord( char* data_ptr, int size )
{
	struct ultrasonic_timeout_record record;
	char* cur_ptr = data_ptr + 3;

	sscanf( cur_ptr, "%2d", &record.ultrasonic_timeout );
	cur_ptr += 2;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid ultrasonic_timeout_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Ultrasonic Timeout Record\n"
			"\tUltrasonic Timeout: %d\n",
			record.ultrasonic_timeout );

	return 0;
}

int IS900::interpretUltrasonicSensitivityRecord( char* data_ptr, int size )
{
	struct ultrasonic_sensitivity_record record;
	char* cur_ptr = data_ptr + 3;

	sscanf( cur_ptr, "%1d", &record.sensitivity_level );
	cur_ptr++;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid ultrasonic_sensitivity_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Ultrasonic Sensitivity Record\n"
			"\tUltrasonic Sensitivity: %d\n",
			record.sensitivity_level );

	return 0;
}

int IS900::interpretFixedPSERecord( char* data_ptr, int size )
{
	vector<struct fixed_pse_record> records;
	char* cur_ptr = data_ptr;
	while( cur_ptr != '\0' )
	{
		if( strncmp( cur_ptr, "31F", 3 ) )
		{
			break;
		}
		cur_ptr += 3;

		struct fixed_pse_record record;

		sscanf( cur_ptr, "%7d", &record.fixed_pse_number );
		cur_ptr += 7;

		for( int i=0;i<3;i++ )
		{
			sscanf( cur_ptr, "%f", &record.position[i] );
			cur_ptr += 10;
		}

		for( int i=0;i<3;i++ )
		{
			sscanf( cur_ptr, "%f", &record.normal[i] );
			cur_ptr += 7;
		}

		record.hardware_id.append( cur_ptr, 0, 7 );
		cur_ptr += 7;

		if( *( cur_ptr ) != '\r' && *( cur_ptr + 1 ) != '\n' )
		{
			dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid fixed_pse_record.\n" );
			continue;
		}
		cur_ptr += 2;

		records.push_back( record );
	}

	for( unsigned int i=0;i<records.size();i++ )
	{
		dtkMsg.add( DTKMSG_NOTICE, "Fixed PSE Record\n"
				"\tFixed PSE Number: %d\n"
				"\tPosition: %10.4f    %10.4f    %10.4f\n"
				"\tNormal: %7.2f    %7.2f    %7.2f\n"
				"\tHardware ID: %s\n",
				records[i].fixed_pse_number,
				records[i].position[0], records[i].position[1], records[i].position[2],
				records[i].normal[0], records[i].normal[1], records[i].normal[2],
				records[i].hardware_id.c_str() );
	}

	return 0;
}

int IS900::interpretTrackingStatusRecord( char* data_ptr, int size )
{
	struct tracking_status_record record;
	char* cur_ptr = data_ptr + 3;

	for( int i=0;i<12;i++ )
	{
		struct station_tracking_state_record station_record;
		sscanf( cur_ptr, "%c", &station_record.station_state );
		cur_ptr++;
		sscanf( cur_ptr, "%1x", &station_record.range_measurement_received );
		cur_ptr++;
		sscanf( cur_ptr, "%1x", &station_record.range_measurement_rejected );
		cur_ptr++;
		record.tracking_state.push_back( station_record );
	}

	sscanf( cur_ptr, "%5d", &record.update_rate );
	cur_ptr += 5;

	// Skip unused byte
	cur_ptr++;

	sscanf( cur_ptr, "%c", &record.genlock_state );
	cur_ptr++;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid tracking_status_record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Tracking Status Record\n" );
	for( unsigned int i=0;i<record.tracking_state.size();i++ )
	{
		dtkMsg.append( "\tStation: %d\n"
			"\t\tTracking state: %c\n"
			"\t\tRange measurements received: %d\n"
			"\t\tRange measurements rejected: %d\n",
			i, record.tracking_state[i].station_state,
			record.tracking_state[i].range_measurement_received,
			record.tracking_state[i].range_measurement_rejected );
	}
	dtkMsg.append( "\tGenlock identifier: %c\n",
			record.genlock_state );

	return 0;
}

int IS900::interpretBeaconSchedulerRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 8;

	int algorithm;
	sscanf( cur_ptr, "%1d", &algorithm );
	cur_ptr++;

	string algorithm_string;
	switch( algorithm )
	{
		case 1:
		{
			algorithm_string = "Distance only";
			break;
		}
		case 2:
		{
			algorithm_string = "Distance and orientation";
			break;
		}
		default:
		{
			algorithm_string = "Unknown";
		}
	}

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid beacon scheduler record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Beacon Scheduler Record\n"
			"\talgorithm: %s\n",
			algorithm_string.c_str() );

	return 0;
}

int IS900::interpretCommandLoggingStateRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 4;

	bool log_state;
	sscanf( cur_ptr, "%1d", &log_state );

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid command logging state record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Command Logging State Record\n"
			"\tLog state: %s\n",
			log_state ? "on" : "off" );

	return 0;
}

int IS900::interpretCommandLogRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 4;

	int i = 0;
	while( *( cur_ptr + i ) != ':' )
	{
		i++;
	}

	int timestamp;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "%%%dd", i );
	sscanf( cur_ptr, temp, &timestamp );

	cur_ptr += i + 1;

	i = 0;
	while( *( cur_ptr + i ) != '\r' )
	{
		i++;
	}

	sprintf( temp, "%%%ss", i );
	temp[i] = '\0';
	string command = temp;

	if( *( cur_ptr ) != '\r' || *( cur_ptr + 1 ) != '\n' )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid command log record.\n" );
		return -1;
	}
	cur_ptr += 2;

	dtkMsg.add( DTKMSG_NOTICE, "Command Log Record\n"
			"\tTimestamp: %14d    "
			"Command: %s\n",
			timestamp, command.c_str() );

	return 0;
}

int IS900::interpretErrorReportRecord( char* data_ptr, int size )
{
	char* cur_ptr = data_ptr + 3;
	if( strncmp( cur_ptr, "*ERROR*", 7 ) )
	{
		dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid error report record.\n" );
		return -1;
	}
	cur_ptr += 7;

	string error;
	int error_code;
	char* end_ptr = strstr( cur_ptr, "*ERROR*" );
	if( end_ptr == NULL )
	{
		end_ptr = strstr( cur_ptr, "EC" );
		if( end_ptr == NULL )
		{
			dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid error report record.\n" );
			return -1;
		}
		else
		{
			cur_ptr = end_ptr + 2;
			sscanf( cur_ptr, "%d", &error_code );
			end_ptr = strstr( cur_ptr, "\r\n" );
			if( end_ptr == NULL )
			{
				dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid error report record.\n" );
				return -1;
			}
			else
			{
				*end_ptr = '\0';
				error = cur_ptr;
			}
		}
	}
	else
	{
		error = "Invalid command: \"";
		error.append( cur_ptr, 0, end_ptr - cur_ptr );
		error.append( 1, '\"' );
		cur_ptr = end_ptr + 7;
		end_ptr = strstr( cur_ptr, "EC" );
		if( end_ptr == NULL )
		{
			dtkMsg.add( DTKMSG_WARNING, "IS900::interpretSystemStatusRecord failed - not a valid error report record.\n" );
			return -1;
		}
		else
		{
			cur_ptr = end_ptr + 2;
			sscanf( cur_ptr, "%d", &error_code );
		}
	}

	dtkMsg.add( DTKMSG_NOTICE, "Error Report Record\n"
			"\tError code: %d\n"
			"\tError: %s\n",
			error_code, error.c_str() );

	return 0;
}

char IS900::getStationID( int station_num )
{
	char station_id = -1;
	if( station_num >= STATION_MIN && station_num <= 9 )
	{
		station_id = '0' + (char)station_num;
	}
	else if( station_num >= 10 && station_num <= STATION_MAX )
	{
		station_id = 'A' + (char)station_num - 10;
	}
	else
	{
		dtkMsg.add( DTKMSG_ERROR, "IS900::getStationID - Invalid station_num %d.\n"
				"The station_num must be a value between %i and %i inclusive.",
				station_num, STATION_MIN, STATION_MAX );
		return 0;
	}

	return station_id;
}

int IS900::getStationNum( char station_id )
{
	int station_num = -1;
	if( station_id > '0' && station_id <= '9' )
	{
		station_num = station_id - '0';
	}
	else if( station_id >= 'A' && station_id <= 'Z' )
	{
		station_num = station_id - 'A' + 1;
	}
	else if( station_id >= 'a' && station_id <= 'z' )
	{
		station_num = station_id - 'a' + 1;
	}
	else
	{
		dtkMsg.add( DTKMSG_ERROR, "IS900::getStationNum - Invalid station_id '%c'.\n"
				"The station_id must be a character between '1' and '9', or 'A' through 'V'.\n",
				station_num );
		return -1;
	}

	return station_num;
}

int IS900::init()
{
	data_sizes[OUTPUT_RECORD_SEPARATOR_SPACE] = 1;
	data_sizes[OUTPUT_RECORD_SEPARATOR_CR_LF] = 2;
	data_sizes[OUTPUT_RECORD_POSITION_XYZ] = 12;
	data_sizes[OUTPUT_RECORD_ORIENTATION_HPR] = 12;
	data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_X] = 12;
	data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_Y] = 12;
	data_sizes[OUTPUT_RECORD_ORIENTATION_COSINES_Z] = 12;
	data_sizes[OUTPUT_RECORD_ORIENTATION_QUATERNION] = 16;
	data_sizes[OUTPUT_RECORD_POSITION_XYZ_16BIT] = 6;
	data_sizes[OUTPUT_RECORD_ORIENTATION_HPR_16BIT] = 6;
	data_sizes[OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT] = 8;
	data_sizes[OUTPUT_RECORD_TIMESTAMP] = 4;
	data_sizes[OUTPUT_RECORD_BUTTONS] = 1;
	data_sizes[OUTPUT_RECORD_JOYSTICK] = 2;
	data_sizes[OUTPUT_RECORD_TRACKING_STATUS] = 1;

	data_sizes_ascii[OUTPUT_RECORD_SEPARATOR_SPACE] = 1;
	data_sizes_ascii[OUTPUT_RECORD_SEPARATOR_CR_LF] = 2;
	data_sizes_ascii[OUTPUT_RECORD_POSITION_XYZ] = 21;
	data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_HPR] = 21;
	data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_X] = 21;
	data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_Y] = 21;
	data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_COSINES_Z] = 21;
	data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_QUATERNION] = 28;
	data_sizes_ascii[OUTPUT_RECORD_POSITION_XYZ_16BIT] = 6;
	data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_HPR_16BIT] = 6;
	data_sizes_ascii[OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT] = 8;
	data_sizes_ascii[OUTPUT_RECORD_TIMESTAMP] = 14;
	data_sizes_ascii[OUTPUT_RECORD_BUTTONS] = 5;
	data_sizes_ascii[OUTPUT_RECORD_JOYSTICK] = 8;
	data_sizes_ascii[OUTPUT_RECORD_TRACKING_STATUS] = 1;

	data_sizes_dtk_shm[OUTPUT_RECORD_SEPARATOR_SPACE] = 0;
	data_sizes_dtk_shm[OUTPUT_RECORD_SEPARATOR_CR_LF] = 0;
	data_sizes_dtk_shm[OUTPUT_RECORD_POSITION_XYZ] = 12;
	data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_HPR] = 12;
	data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_COSINES_X] = 12;
	data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_COSINES_Y] = 12;
	data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_COSINES_Z] = 12;
	data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_QUATERNION] = 16;
	data_sizes_dtk_shm[OUTPUT_RECORD_POSITION_XYZ_16BIT] = 12;
	data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_HPR_16BIT] = 12;
	data_sizes_dtk_shm[OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT] = 16;
	data_sizes_dtk_shm[OUTPUT_RECORD_TIMESTAMP] = 4;
	data_sizes_dtk_shm[OUTPUT_RECORD_BUTTONS] = 1;
	data_sizes_dtk_shm[OUTPUT_RECORD_JOYSTICK] = 8;
	data_sizes_dtk_shm[OUTPUT_RECORD_TRACKING_STATUS] = 1;

	if( getCalibrator() )
	{
		if( getCalibrator()->initCalibration( this, getCalibratorParams() ) )
		{
			dtkMsg.add( DTKMSG_ERROR, "The dtkCalibration DSO %s had trouble with\n"
					"the parameters passed to it.\n", getCalibratorName().c_str() );
			return DTKSERVICE_ERROR;
		}
	}

	int error = configureIS900();
	if( error )
	{
		return DTKSERVICE_ERROR;
	}

	if( device.eth_state )
	{
		dtkMsg.add( DTKMSG_ERROR, "IS900::init() failed - ethernet not currently implemented.\n" );
		return -1;
	}
	else
	{
		device.port = getPort();
		dtkMsg.add( DTKMSG_NOTICE, "Using port: %s\n", device.port.c_str() );
		device.baud = getBaud();
		device.baud_rate = getBaudFlag();
		dtkMsg.add( DTKMSG_NOTICE, "Using baud rate: %s\n", getBaud().c_str() );

#ifdef DTK_ARCH_WIN32_VCPP
		fd = CreateFile( device.port.c_str(), GENERIC_READ | GENERIC_WRITE,
			0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0 );
		if( fd == INVALID_HANDLE_VALUE )
		{
			dtkMsg.add( DTKMSG_ERROR, 1,
				"IS900::IS900()"
				" failed: CreateFile(\"%s%s%s\","
				"GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,0,0) failed.\n",
				dtkMsg.color.tur, device.port.c_str(),
				dtkMsg.color.end );
			return DTKSERVICE_ERROR;
		}
		if( !SetupComm( fd, 4096, 4096 ) )
		{
			dtkMsg.add( DTKMSG_ERROR, 1,
				"IS900::IS900()"
				" failed: SetupComm(%p,64,64).\n",
				fd );
			return DTKSERVICE_ERROR;
		}
		DCB commDCB;
		if( !GetCommState( fd, &commDCB ) )
		{
			dtkMsg.add( DTKMSG_ERROR, 1,
				"IS900::IS900()"
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
				"IS900::IS900()"
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
				"IS900::IS900()"
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
				"IS900::init() failed: "
				"tcsetattr(%d,TCSANOW) failed "
				"for device file %s%s%s.",
				fd, dtkMsg.color.tur, device.port.c_str(),
				dtkMsg.color.end );
			return DTKSERVICE_ERROR;
		}

		if( tcflush( fd, TCIOFLUSH ) == -1 )
		{
			dtkMsg.add( DTKMSG_ERROR, 1,
				"IS900::init() failed: "
				"tcflush(%d,TCIOFLUSH) failed "
				"for device file %s%s%s.",
				fd, dtkMsg.color.tur, device.port.c_str(),
				dtkMsg.color.end );
			return DTKSERVICE_ERROR;
		}
#endif // DTK_ARCH_WIN32_VCPP
	}

	char temp_buf[TEMP_BUFFER_SIZE];
	int error_bytes_read = 0;

	// Change output mode to polled in case it is already continuous
	sendOutputMode( true );
	usleep( 100 );

	// Clear serial buffer if data was still spewing
	readValue( temp_buf );

	// Set to binary mode
	sendOutputRecordMode( false );

	// Set error reporting to true
	sendErrorReporting( true );

	requestErrorReport();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	// Give visual feedback (possibly temporary) that the sonistrips are working
	sendSonistripLEDControl( true );

	if( device.constellation_file.size() )
		sendConstellationFile( device.constellation_file );
	if( device.only_reload_constellation_file )
	{
		cleanUp();
		return DTKSERVICE_UNLOAD;
	}

	map<int, station_state>::iterator station_itr = device.stations.begin();
	for( station_itr;station_itr!=device.stations.end();station_itr++ )
	{
		sendStationStatus( station_itr->first, station_itr->second.enabled );

		if( station_itr->second.output_record_list.size() )
		{
			calcStationDataSize( station_itr->first );

			sendOutputRecordList( station_itr->first, station_itr->second.output_record_list );

			sendResetAlignmentReferenceFrame( station_itr->first );

			sendAlignmentReferenceFrame( station_itr->first,
					station_itr->second.sensor_alignment_reference_frame[0],
					station_itr->second.sensor_alignment_reference_frame[1],
					station_itr->second.sensor_alignment_reference_frame[2],
					station_itr->second.sensor_alignment_reference_frame[3],
					station_itr->second.sensor_alignment_reference_frame[4],
					station_itr->second.sensor_alignment_reference_frame[5],
					station_itr->second.sensor_alignment_reference_frame[6],
					station_itr->second.sensor_alignment_reference_frame[7],
					station_itr->second.sensor_alignment_reference_frame[8] );

			sendBoresightReferenceAngles( station_itr->first, 0.0f, 0.0f, 0.0f );
		}

		map<string, vector<output_record_type> >::const_iterator itr =
				station_itr->second.dtk_shm_map.begin();
		for( itr;itr!=station_itr->second.dtk_shm_map.end();itr++ )
		{
			int shm_size = 0;
			for( unsigned int j=0;j<itr->second.size();j++ )
			{
				shm_size += data_sizes_dtk_shm[itr->second[j]];
			}
			dtkSharedMem* shm = new dtkSharedMem( shm_size, itr->first.c_str() );
			if( !shm )
			{
				dtkMsg.add( DTKMSG_ERROR, "IS900::init() failed - unable to create dtkSharedMem( %s, %d ).\n",
						itr->first.c_str(), shm_size );
				return DTKSERVICE_ERROR;
			}
			else
				station_itr->second.dtk_shm[itr->first] = shm;
		}
	}

	requestSystemRecord();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestEthernetAddress();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestEthernetState();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestEthernetPort();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestIntersenseStatusRecord();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestTrackingStatusRecord();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestUltrasonicTimeoutInterval();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestUltrasonicReceiverSensitivity();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestGenlockSynchronization();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestGenlockPhase();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestGenlockSyncSource();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestConfigurationLock();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestBeaconScheduler();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	requestAssociateFixedPSEWithConstellation();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	map<int, station_state>::iterator itr = device.stations.begin();
	for( itr;itr!=device.stations.end();itr++ )
	{
		if( itr->second.enabled )
		{
			requestAlignmentReferenceFrame( itr->first );
			error_bytes_read = readValue( temp_buf );
			interpretRecord( temp_buf, 1024 );

			requestBoresightReferenceAngles( itr->first );
			error_bytes_read = readValue( temp_buf );
			interpretRecord( temp_buf, 1024 );

			requestOutputRecordList( itr->first );
			error_bytes_read = readValue( temp_buf );
			interpretRecord( temp_buf, 1024 );

			requestDefineTipOffsets( itr->first );
			error_bytes_read = readValue( temp_buf );
			interpretRecord( temp_buf, 1024 );

			requestPositionOperationalEnvelope( itr->first );
			error_bytes_read = readValue( temp_buf );
			interpretRecord( temp_buf, 1024 );

			requestIntersenseStationStatusRecord( itr->first );
			error_bytes_read = readValue( temp_buf );
			interpretRecord( temp_buf, 1024 );

			requestPredictionInterval( itr->first );
			error_bytes_read = readValue( temp_buf );
			interpretRecord( temp_buf, 1024 );

			requestRotationalSensitivity( itr->first );
			error_bytes_read = readValue( temp_buf );
			interpretRecord( temp_buf, 1024 );
		}
	}

	sendSonistripLEDControl( device.led_control );

	device.buffer_size = TEMP_BUFFER_SIZE;
	if( !( device.buffer = new char[device.buffer_size] ) )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to allocate device buffer of size %d\n",
			device.buffer_size );
		return DTKSERVICE_ERROR;
	}
	device.cur_ptr = device.begin_ptr = device.end_ptr = device.buffer;

	requestErrorReport();
	error_bytes_read = readValue( temp_buf );
	interpretRecord( temp_buf, 1024 );

	// Set to continuous data record mode
	sendOutputMode( false );

	return DTKSERVICE_CONTINUE;
}

int IS900::configureIS900()
{
	int station_num;
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
		if( config[i][0] == "ethenable" )
		{
			if( config[i].size() != 2 )
				has_error = true;
			else if( tolower( config[i][1] ) != "on" && config[i][1] != "off" &&
					config[i][1] != "yes" && config[i][1] != "no" &&
					config[i][1] != "true" && config[i][1] != "false" )
				has_error = true;
			if( has_error )
			{
				error = "The ethEnable parameter specifies whether to enable network communication\n"
						"or serial communication. If ethEnable is true then network communication\n"
						"will be used. By default, serial communication is used.\n"
						"Syntax: serviceOption name ethEnable boolean\n"
						"Example: serviceOption ";
				error += getName();
				error += " ethEnable on\n"
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
					device.eth_state = true;
				}
				else if( config[i][1] == "off" || config[i][1] == "no" ||
						config[i][1] == "false" )
				{
					device.eth_state = false;
				}
			}
		}
		else if( config[i][0] == "ethaddress" )
		{
			if( config[i].size() != 2 )
				has_error = true;
			else
			{
				int ip_parts[4];
				sscanf( config[i][1].c_str(), "%d.%d.%d.%d",
						&ip_parts[0], &ip_parts[1], &ip_parts[2], &ip_parts[3] );
				for( int j=0;j<4;j++ )
				{
					if( ip_parts[j] < 0 || ip_parts[j] > 255 )
					{
						has_error = true;
						error = "Invalid IP address \"";
						error += config[i][1];
						error += "\". The IP address is 4 numbers\n"
								"ranging from 0 to 255 separated by periods (\".\").\n";
						error_strings.push_back( error );
					}
				}
			}
			if( has_error )
			{
				error = "The ethAddress parameter specifes the IP address to use for netowrk\n"
						"communication. It must be an IPV4 address using standard dot notation\n"
						"(i.e. 123.45.67.89).\n";
				error += "Syntax: serviceOption name ethAddress ipAddress\n";
						"Example: serviceOption ";
				error += getName();
				error += " ethAddress 123.45.67.89\n"
						"where name was defined with the serviceType definition,\n"
						"and ipAddress is the IP address the IS900 will be set to use.\n";
				error_strings.push_back( error );
			}
			else
				device.eth_address = config[i][1];
		}
		else if( config[i][0] == "ethport" )
		{
			int port;
			if( config[i].size() != 2 )
				has_error = true;
			else
			{
				sscanf( config[i][1].c_str(), "%d", &port );
				if( port < 1 || port > 65535 )
				{
					has_error = true;
					error = "Invalid ethernet port \"";
					error += port;
					error += "\". The port number must be in the range 1 to 65535.\n";
					error_strings.push_back( error );
				}
			}
			if( has_error )
			{
				error = "The ethPort parameter specifies the UDP port number to use for netowrk\n"
						"communication.\n";
				error += "Syntax: serviceOption name ethPort port\n"
						"Example: serviceOption ";
				error += getName();
				error += " ethPort 5001\n"
						"where name was defined with the serviceType definition,\n"
						"and port is the ethernet port with a range from 1 to 65535.\n";
				error_strings.push_back( error );
			}
			else
				device.eth_port = port;
		}
		else if( config[i][0] == "ledcontrol" )
		{
			if( config[i].size() != 2 )
				has_error = true;
			else if( tolower( config[i][1] ) != "on" && config[i][1] != "off" &&
					config[i][1] != "yes" && config[i][1] != "no" &&
					config[i][1] != "true" && config[i][1] != "false" )
				has_error = true;
			if( has_error )
			{
				error = "The ledControl option specifies whether to enable or disable the\n"
						"LEDs on the SoniStrips.\n"
						"Syntax: serviceOption name ledControl boolean\n"
						"Example: serviceOption ";
				error += getName();
				error += " ledControl on\n"
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
					device.led_control = true;
				}
				else if( config[i][1] == "off" || config[i][1] == "no" ||
						config[i][1] == "false" )
				{
					device.led_control = false;
				}
			}
		}
		else if( config[i][0] == "onlyreloadconstellationfile" )
		{
			if( config[i].size() != 2 )
				has_error = true;
			else if( tolower( config[i][1] ) != "on" && config[i][1] != "off" &&
					config[i][1] != "yes" && config[i][1] != "no" &&
					config[i][1] != "true" && config[i][1] != "false" )
				has_error = true;
			if( has_error )
			{
				error = "The onlyreloadconstellationfile option specifies if only the constellation file\n"
						"should be reloaded and no other functionality is used.\n"
						"Syntax: serviceOption name onlyreloadconstellationfile boolean\n"
						"Example: serviceOption ";
				error += getName();
				error += " onlyreloadconstellationfile on\n"
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
					device.only_reload_constellation_file = true;
				}
				else if( config[i][1] == "off" || config[i][1] == "no" ||
						config[i][1] == "false" )
				{
					device.only_reload_constellation_file = false;
				}
			}
		}
		else if( config[i][0] == "constellationfile" )
		{
			if( config[i].size() != 2 )
				has_error = true;
			else
			{
				string path = getConstellationPath();
				char* filename = dtk_getFileFromPath( path.c_str(), config[i][1].c_str() );
				if( !filename )
				{
					error = "The constellation file \"";
					error += config[i][1].c_str();
					error += "\" is not valid.\n";
					error_strings.push_back( error );
				}
			}
			if( has_error )
			{
				error = "The constellationFile option specifies which constellation file to\n"
						"load when the service is started.\n"
						"Syntax: serviceOption name constellationFile file\n"
						"Example: serviceOption ";
				error += getName();
				error += " constellationFile VTCaveIS900.txt\n"
						"where name was defined with the serviceType definition,\n"
						"and file is an existing file containing an IS900 constellation.\n";
				error_strings.push_back( error );
			}
			else
				device.constellation_file = config[i][1];
		}
		else if( config[i][0] == "trackdtype" )
		{
			int shm_key = 0;
			if( config[i].size() != 4 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
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
						"Syntax:  serviceOption name trackdType stationNumber type shmKey\n"
						"Example: serviceOption ";
				error += getName();
				error += " trackdType 2 tracker 4126\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station address of the sensor,\n"
						"type is either \"tracker\" or \"controller\",\n"
						"and shmKey is the System V shared memory key to use.\n";
				error_strings.push_back( error );
			}
			else
			{
				if( config[i][2] == "tracker" )
					device.stations[station_num].trackd_shm_type = TRACKD_TRACKER;
				else if( config[i][2] == "controller" )
					device.stations[station_num].trackd_shm_type = TRACKD_CONTROLLER;
				device.stations[station_num].trackd_key = shm_key;
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
		else if( config[i][0] == "sensoralignment" )
		{
			if( config[i].size() != 11 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
				error_strings.push_back( error );
			}
			else
			{
				for( unsigned int j=2;j<config[i].size();j++ )
				{
					if( convertStringToNumber( device.stations[station_num].sensor_alignment_reference_frame[j-2],
							config[i][j].c_str() ) )
					{
						has_error = true;
						error = "The value ";
						error += config[i][j];
						error += " is invalid for the sensorAlignment.\n";
						error_strings.push_back( error );
					}
				}
			}
			if( has_error )
			{
				error = "The sensorAlignment option is used to specify the coordinate frame for\n"
						"the specified sensor.\n"
						"Syntax: serviceOption name sensorAlignment stationNumber Ox Oy Oz Xx Xy Xz Yx Yy Yz\n"
						"Example: serviceOption ";
				error += getName();
				error += " sensorOffset 1 0 0 0 0 -1 0 1 0 0\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station address of the sensor to set,\n"
						"Ox, Oy, and Oz are the coordinates of the new origin,\n"
						"Xx, Xy, and Xz are a point on the new x axis, and\n"
						"Yx, Yy, and Yz are a point on the new y axis.\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "sensoroffset" )
		{
			if( config[i].size() != 5 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
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
						device.stations[station_num].sensor_offset[k] = value;
				}
			}
			if( has_error )
			{
				error = "The sensorOffset option is used to specify the offset of the\n"
						"output sensor location relative to its real location.\n"
						"Syntax: serviceOption name sensorOffset stationNumber X Y Z\n"
						"Example: serviceOption ";
				error += getName();
				error += " sensorOffset 2 1.2 0.3 -0.6\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station address of the sensor to set,\n"
						"and X, Y, and Z are measured in inches.\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "sensorrotation" )
		{
			if( config[i].size() != 5 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
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
					device.stations[station_num].sensor_rotation[k] = value;
				}
			}
			if( has_error )
			{
				error = "The sensorrotation option is used to specify the rotation\n"
						"of the sensor relative to the transmitter\n"
						"Syntax: serviceOption name sensorRotation stationNumber H P R\n"
						"Example: serviceOption ";
				error += getName();
				error += " sensorRotation 3 -90.0 21.5 15.0\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station address of the sensor to set,\n"
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
		else if( config[i][0] == "station" )
		{
			if( config[i].size() != 3 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
				error_strings.push_back( error );
			}
			else if( tolower( config[i][2] ) != "on" && config[i][2] != "off" &&
					config[i][2] != "yes" && config[i][2] != "no" &&
					config[i][2] != "true" && config[i][2] != "false" )
				has_error = true;
			if( has_error )
			{
			}
			else
			{
				if( config[i][2] == "on" || config[i][2] == "yes" ||
						config[i][2] == "true" )
				{
					device.stations[station_num].enabled = true;
				}
				else if( config[i][2] == "off" || config[i][2] == "no" ||
						config[i][2] == "false" )
				{
					device.stations[station_num].enabled = false;
				}
			}
		}
		else if( config[i][0] == "datamode" )
		{
			if( config[i].size() < 3 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
				error_strings.push_back( error );
			}
			if( device.stations[station_num].output_record_list.size() )
			{
				has_error = true;
				error = "An output record list already exists for station ";
				error += station_num;
				error += ".\nOnly one output record list can be defined for each station.\n";
				error_strings.push_back( error );
			}
			bool has_separatorspace = false;
			bool has_separatorcrlf = false;
			bool has_position = false;
			bool has_angles = false;
			bool has_cosinesx = false;
			bool has_cosinesy = false;
			bool has_cosinesz = false;
			bool has_quaternion = false;
			bool has_position16bit = false;
			bool has_angles16bit = false;
			bool has_quaternion16bit = false;
			bool has_timestamp = false;
			bool has_buttons = false;
			bool has_joystick = false;
			bool has_trackingstatus = false;
			for( unsigned int j=2;j<config[i].size();j++ )
			{
				bool local_error = false;
				if( config[i][j] == "separatorspace" )
				{
					if( has_separatorspace )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_SEPARATOR_SPACE );
					has_separatorspace = true;
				}
				else if( config[i][j] == "separatorcrlf" )
				{
					if( has_separatorcrlf )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_SEPARATOR_CR_LF );
					has_separatorcrlf = true;
				}
				else if( config[i][j] == "position" )
				{
					if( has_position )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_POSITION_XYZ );
					device.stations[station_num].has_position = true;
					has_position = true;
				}
				else if( config[i][j] == "angles" )
				{
					if( has_angles )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_ORIENTATION_HPR );
					device.stations[station_num].has_angles = true;
					has_angles = true;
				}
				else if( config[i][j] == "cosinesx" )
				{
					if( has_cosinesx )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_ORIENTATION_COSINES_X );
					device.stations[station_num].has_cosinesx = true;
					has_cosinesx = true;
				}
				else if( config[i][j] == "cosinesy" )
				{
					if( has_cosinesy )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_ORIENTATION_COSINES_Y );
					device.stations[station_num].has_cosinesy = true;
					has_cosinesy = true;
				}
				else if( config[i][j] == "cosinesz" )
				{
					if( has_cosinesz )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_ORIENTATION_COSINES_Z );
					device.stations[station_num].has_cosinesz = true;
					has_cosinesz = true;
				}
				else if( config[i][j] == "quaternion" )
				{
					if( has_quaternion )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_ORIENTATION_QUATERNION );
					device.stations[station_num].has_quaternion = true;
					has_quaternion = true;
				}
				else if( config[i][j] == "position16bit" )
				{
					if( has_position16bit )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_POSITION_XYZ_16BIT );
					device.stations[station_num].has_position = true;
					has_position16bit = true;
				}
				else if( config[i][j] == "angles16bit" )
				{
					if( has_angles16bit )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_ORIENTATION_HPR_16BIT );
					device.stations[station_num].has_angles = true;
					has_angles16bit = true;
				}
				else if( config[i][j] == "quaternion16bit" )
				{
					if( has_quaternion16bit )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT );
					device.stations[station_num].has_quaternion = true;
					has_quaternion16bit = true;
				}
				else if( config[i][j] == "timestamp" )
				{
					if( has_timestamp )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_TIMESTAMP );
					has_timestamp = true;
				}
				else if( config[i][j] == "buttons" )
				{
					if( has_buttons )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_BUTTONS );
					has_buttons = true;
				}
				else if( config[i][j] == "joystick" )
				{
					if( has_joystick )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_JOYSTICK );
					has_joystick = true;
				}
				else if( config[i][j] == "trackingstatus" )
				{
					if( has_trackingstatus )
						local_error = true;
					device.stations[station_num].output_record_list.push_back( OUTPUT_RECORD_TRACKING_STATUS );
					has_trackingstatus = true;
				}
				else
				{
					has_error = true;
					error = "Unrecognized output record list type \"";
					error += config[i][j];
					error += " \".\n";
					error_strings.push_back( error );
				}
				if( local_error )
				{
					has_error = true;
					error = "Duplicate ";
					error += config[i][j];
					error += " parameter passed in the dataMode option for station ";
					error += station_num;
					error += "\n";
					error_strings.push_back( error );
				}
			}
			if( has_error )
			{
				error = "The dataMode option defines the types of data to be output\n"
						"by the specified station.\n"
						"Syntax: serviceOption name dataMode stationNumber modes\n"
						"Example: serviceOption ";
				error += getName();
				error += " dataMode 2 position angles separatorcrlf\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station address of the sensor to set and\n"
						"modes is one or more of the following values: separatorspace,\n"
						"separatorcrlf, position, angles, cosinesx, cosinesy, cosinesz,\n"
						"quaternion, position16bit, angles16bit, quaternion16bit,\n"
						"timestamp, buttons, joystick, and/or trackingstatus.\n";
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
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The angleUnits option specifies whether the angle data\n"
						"is output as degrees or radians.\n"
						"Syntax: serviceOption name angles stationNumber ( degrees | radians )\n"
						"Example: serviceOption ";
				error += getName();
				error += " angles 2 radians\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station address of the sensor to set, and\n"
						"the unit type is set to either \"degrees\" or \"radians\".\n"
						"The default value for angle units is \"degrees\"\n";
				error_strings.push_back( error );
			}
			else if( config[i][2] == "degrees" )
				device.stations[station_num].angle_degrees = true;
			else
				device.stations[station_num].angle_degrees = false;
		}
		else if( config[i][0] == "dtkshm" )
		{
			if( config[i].size() < 4 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
				error_strings.push_back( error );
			}
			string dtk_shm_name = config[i][2];
			if( device.stations[station_num].dtk_shm_map.find( dtk_shm_name ) != device.stations[station_num].dtk_shm_map.end() )
			{
				has_error = true;
				error = "There is a duplicate entry for dtkShm \"";
				error += dtk_shm_name;
				error += "\". Each dtkShm name must be unique.\n";
				error_strings.push_back( error );
			}
			bool has_separatorspace = false;
			bool has_separatorcrlf = false;
			bool has_position = false;
			bool has_angles = false;
			bool has_cosinesx = false;
			bool has_cosinesy = false;
			bool has_cosinesz = false;
			bool has_quaternion = false;
			bool has_position16bit = false;
			bool has_angles16bit = false;
			bool has_quaternion16bit = false;
			bool has_timestamp = false;
			bool has_buttons = false;
			bool has_joystick = false;
			bool has_trackingstatus = false;
			for( unsigned int j=3;j<config[i].size();j++ )
			{
				bool local_error = false;
				if( config[i][j] == "separatorspace" )
				{
					if( has_separatorspace )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_SEPARATOR_SPACE );
					has_separatorspace = true;
				}
				else if( config[i][j] == "separatorcrlf" )
				{
					if( has_separatorcrlf )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_SEPARATOR_CR_LF );
					has_separatorcrlf = true;
				}
				else if( config[i][j] == "position" )
				{
					if( has_position )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_POSITION_XYZ );
					has_position = true;
				}
				else if( config[i][j] == "angles" )
				{
					if( has_angles )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_ORIENTATION_HPR );
					has_angles = true;
				}
				else if( config[i][j] == "cosinesx" )
				{
					if( has_cosinesx )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_ORIENTATION_COSINES_X );
					has_cosinesx = true;
				}
				else if( config[i][j] == "cosinesy" )
				{
					if( has_cosinesy )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_ORIENTATION_COSINES_Y );
					has_cosinesy = true;
				}
				else if( config[i][j] == "cosinesz" )
				{
					if( has_cosinesz )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_ORIENTATION_COSINES_Z );
					has_cosinesz = true;
				}
				else if( config[i][j] == "quaternion" )
				{
					if( has_quaternion )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_ORIENTATION_QUATERNION );
					has_quaternion = true;
				}
				else if( config[i][j] == "position16bit" )
				{
					if( has_position16bit )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_POSITION_XYZ_16BIT );
					has_position16bit = true;
				}
				else if( config[i][j] == "angles16bit" )
				{
					if( has_angles16bit )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_ORIENTATION_HPR_16BIT );
					has_angles16bit = true;
				}
				else if( config[i][j] == "quaternion16bit" )
				{
					if( has_quaternion16bit )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_ORIENTATION_QUATERNION_16BIT );
					has_quaternion16bit = true;
				}
				else if( config[i][j] == "timestamp" )
				{
					if( has_timestamp )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_TIMESTAMP );
					has_timestamp = true;
				}
				else if( config[i][j] == "buttons" )
				{
					if( has_buttons )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_BUTTONS );
					has_buttons = true;
				}
				else if( config[i][j] == "joystick" )
				{
					if( has_joystick )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_JOYSTICK );
					has_joystick = true;
				}
				else if( config[i][j] == "trackingstatus" )
				{
					if( has_trackingstatus )
						local_error = true;
					device.stations[station_num].dtk_shm_map[dtk_shm_name].push_back( OUTPUT_RECORD_TRACKING_STATUS );
					has_trackingstatus = true;
				}
				else
				{
					has_error = true;
					error = "Unrecognized output record list type \"";
					error += config[i][j];
					error += " \".\n";
					error_strings.push_back( error );
				}
				if( local_error )
				{
					has_error = true;
					error = "Duplicate ";
					error += config[i][j];
					error += " parameter passed in the dtkShm option for station ";
					error += station_num;
					error += "\n";
					error_strings.push_back( error );
				}
			}
			if( has_error )
			{
				error = "The dtkShm option specifies the name of the dtkSharedMem\n"
						"segment to be created and used for data output.\n"
						"Syntax: serviceOption name dtkShm stationNumber shmName"
						"Example: serviceOption ";
				error += getName();
				error += "dtkShm 2 head"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station address of the sensor to set and\n"
						"shmName is a valid name for a file.\n";
				error_strings.push_back( error );
			}
			if( has_error )
			{
				error = "The dataMode option defines the types of data to be output\n"
						"by the specified station.\n"
						"Syntax: serviceOption name dataMode stationNumber modes\n"
						"Example: serviceOption ";
				error += getName();
				error += " dataMode 2 position angles separatorcrlf\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station address of the sensor to set and\n"
						"modes is one or more of the following values: separatorspace,\n"
						"separatorcrlf, position, angles, cosinesx, cosinesy, cosinesz,\n"
						"quaternion, position16bit, angles16bit, quaternion16bit,\n"
						"timestamp, buttons, joystick, and/or trackingstatus.\n";
				error_strings.push_back( error );
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
						device.axis_map[j] = 0;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+h" )
					{
						device.axis_map[j] = 0;
						device.axis_sign[j] = 1.0f;
					}
					else if( config[i][j+1] == "-p" )
					{
						device.axis_map[j] = 1;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+p" )
					{
						device.axis_map[j] = 1;
						device.axis_sign[j] = 1.0f;
					}
					else if( config[i][j+1] == "-r" )
					{
						device.axis_map[j] = 2;
						device.axis_sign[j] = -1.0f;
					}
					else if( config[i][j+1] == "+r" )
					{
						device.axis_map[j] = 2;
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
		else if( config[i][0] == "buttonmap" )
		{
			if( config[i].size() < 3 || config[i].size() > 10 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
				error_strings.push_back( error );
			}
			int button_num;
			for( unsigned int j=2;j<config[i].size();j++ )
			{
				if( convertStringToNumber( button_num, config[i][j].c_str() ) )
				{
					has_error = true;
					error = "The value ";
					error += config[i][j];
					error += " is not a valid numerical value.\n";
					error_strings.push_back( error );
				}
				else if( button_num < 1 || button_num > 8 )
				{
					has_error = true;
					error = "Invalid button number ";
					error += button_num;
					error += " - the button number must be a value between 1 and 8.\n";
					error_strings.push_back( error );
				}
				else
					device.stations[station_num].button_map[j-2] = button_num - 1;
			}
			if( has_error )
			{
				error = "The buttonMap option specifies the ordering of the buttons."
						"Syntax: serviceOption name buttonMap stationNumber B1 ...\n"
						"Example: serviceOption ";
				error += getName();
				error += " buttonMap 2 1 2 3 4 5 1\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station of the sensor to set,\n"
						"B1 is the mapping value of the first button, and ... are all\n"
						"successive button mappings.\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "joystickorder" )
		{
			if( config[i].size() != 4 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
				error_strings.push_back( error );
			}
			for( int j=0;j<2;j++ )
			{
				if( config[i][j+2].c_str() == "-x" )
				{
					device.stations[station_num].joystick_map[j] = 0;
					device.stations[station_num].joystick_sign[j] = -1.0f;
				}
				else if( config[i][j+2].c_str() == "+x" )
				{
					device.stations[station_num].joystick_map[j] = 0;
					device.stations[station_num].joystick_sign[j] = 1.0f;
				}
				else if( config[i][j+2].c_str() == "-y" )
				{
					device.stations[station_num].joystick_map[j] = 1;
					device.stations[station_num].joystick_sign[j] = -1.0f;
				}
				else if( config[i][j+2].c_str() == "+y" )
				{
					device.stations[station_num].joystick_map[j] = 1;
					device.stations[station_num].joystick_sign[j] = 1.0f;
				}
				else
				{
					error = "The value \"";
					error += config[i][j+2];
					error += "\" is not a valid floating\n"
							"point number in the configuration line\n";
					error_strings.push_back( error );
				}
			}
			if( has_error )
			{
				error = "The joystickOrder option specifies the sign and ordering of the\n"
						"joystick for the specified station.\n"
						"Syntax: serviceOption name joystickOrder stationNumber X Y\n"
						"Example: serviceOption ";
				error += getName();
				error += " joystickOrder 2 -x y\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station of the sensor to set,\n"
						"and X and Y are \"-x\", \"+x\", \"-y\", or \"+y\".\n";
				error_strings.push_back( error );
			}
		}
		else if( config[i][0] == "joystickpoly" )
		{
			float value;
			if( config[i].size() < 4 )
				has_error = true;
			else if( convertStringToNumber( station_num, config[i][1].c_str() ) )
			{
				has_error = true;
				string error = "The stationNumber parameter ";
				error += config[i][1];
				error += " is not valid\n"
						"The stationNumber should be an integer representing the address\n"
						"of the station.\n";
				error_strings.push_back( error );
			}
			else if( station_num < STATION_MIN || station_num > STATION_MAX )
			{
				has_error = true;
				error = "The stationNumber is not within the valid range of ";
				error += STATION_MIN;
				error += " to ";
				error += STATION_MAX;
				error += ".\n";
				error_strings.push_back( error );
			}
			int joystick_axis;
			if( config[i][2] == "x" )
				joystick_axis = 0;
			else if( config[i][2] == "y" )
				joystick_axis = 1;
			else
			{
				has_error = true;
				error = "The joystickAxis must be either \"x\" or \"y\".\n";
				error_strings.push_back( error );
			}
			for( unsigned int j=3;j<config[i].size();j++ )
			{
				if( convertStringToNumber( value, config[i][j].c_str() ) )
				{
					has_error = true;
					error = "The value ";
					error += config[i][j];
					error += " is not a valid numerical value.\n";
					error_strings.push_back( error );
				}
				else
					device.stations[station_num].joystick_poly[joystick_axis].push_back( value );
			}
			if( has_error )
			{
				error = "The joystickPoly option is used to specify a polynomial\n"
						"which is used to alter the joystick value for the specified\n"
						"joystick axis.\n"
						"Syntax: serviceOption name joystickPoly stationNum joystickAxis C0 ...\n"
						"Example: serviceOption ";
				error += getName();
				error += " joystickPoly 2 x 0.8 1.2 0.2\n"
						"where name was defined with the serviceType definition,\n"
						"stationNumber is the station of the sensor to set,\n"
						"joystickAxis is either \"x\" or \"y\" representing the axis\n"
						"of the joystick to which the polynomial should be applied,\n"
						"and C0 is the first coefficient, and ... can be all additional coefficients\n";
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

	return DTKSERVICE_CONTINUE;
}

int IS900::readValue( char* buffer, int milli_seconds, bool show_errors )
{
	int test = 0;
	int bytes_read_total = 0;
	int bytes_read_new = 0;
	int timeout = milli_seconds;

#ifdef DTK_ARCH_WIN32_VCPP
	DWORD read_size = 0;
#endif
	bool end_of_record = false;

	memset( buffer, '\0', TEMP_BUFFER_SIZE );
	while( !end_of_record )
	{
		usleep( 1000 );

#ifdef DTK_ARCH_WIN32_VCPP
		ReadFile( fd, buffer + bytes_read_total, TEMP_BUFFER_SIZE, &read_size, &io_overlapped );
		bytes_read_new = (int)read_size;
#else
		bytes_read_new = read( fd, buffer + bytes_read_total, TEMP_BUFFER_SIZE );
#endif

if( 0 )
{
	if( bytes_read_new )
	{
		for( int i=0;i<bytes_read_new;i++ )
			printf( "%c", (unsigned char)*(buffer + bytes_read_total + i ) );
		printf( "\n" );

		for( int i=0;i<bytes_read_new;i++ )
			printf( "%4d", (unsigned char)*(buffer + bytes_read_total + i ) );
		printf( "\n" );
	}
}

		bytes_read_total += bytes_read_new;
		timeout--;
		if( timeout < 0 && !bytes_read_new )
		{
			char* cur_ptr = buffer + bytes_read_total;
			if( *( cur_ptr - 2 ) == '\r' && *( cur_ptr -1 ) == '\n' )
			{
				return 0;
			}
			if( show_errors )
				dtkMsg.add( DTKMSG_ERROR, "Timed out after %d milliseconds.\n"
						"\tRead %d bytes.\n",
						milli_seconds, bytes_read_total );
			if( !bytes_read_total )
				strcpy( buffer, "NODATA" );
			return bytes_read_total;
		}
		if( *( buffer + bytes_read_total - 2 ) == '\r' && *( buffer + bytes_read_total - 1 ) == '\n' )
		{
			test++;
			if( test == 2 )
				return 0;
		}
		else
			test = 0;
	}

	return 0;
}

int IS900::serve()
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

if( 0 )
{
	for( int i=0;i<bytes_read;i++ )
		printf( "%c", *( device.end_ptr + i ) );
	printf( "\n" );

	for( int i=0;i<bytes_read;i++ )
		printf( "%4d", (int)*( device.end_ptr + i ) );
	printf( "\n" );
}

	device.end_ptr += bytes_read;
	int cur_size = (int)( device.end_ptr - device.begin_ptr );
	if( bytes_avail - bytes_read < 64 )
	{
		memcpy( device.buffer, device.begin_ptr, cur_size );
		device.end_ptr = device.buffer + cur_size;
		device.cur_ptr = device.begin_ptr = device.buffer;
	}

	char* test_ptr = device.cur_ptr;
	int record_retval = 0;
	while( test_ptr < device.end_ptr )
	{
		if( *( test_ptr ) == '\r' && *( test_ptr + 1 ) == '\n' &&
				( test_ptr + 1 ) < device.end_ptr )
		{
			test_ptr += 2;
			if( ( record_retval = interpretRecord( device.begin_ptr, test_ptr - device.begin_ptr ) ) < 0 )
			{
				// The "true" end of record was not found - continue searching
				if( record_retval == -2 )
				{
					test_ptr++;
					continue;
				}
				// The record was invalid/corrupted - device.begin_ptr was moved
				else if( record_retval == -3 )
				{
					return DTKSERVICE_CONTINUE;
				}
				else
				{
					return DTKSERVICE_ERROR;
				}
			}
			else
			{
				device.begin_ptr = device.cur_ptr = test_ptr;
			}
		}
		else
			test_ptr++;
	}

	return DTKSERVICE_CONTINUE;
}

int IS900::rawOffsetAndOrientation( station_state& station )
{
	dtkVec3 xmtr_xyz( device.transmitter_offset );
	dtkVec3 xmtr_hpr( device.transmitter_rotation );

	dtkVec3 snsr_xyz( station.sensor_offset );
	dtkVec3 snsr_hpr( station.sensor_rotation );

	dtkMatrix mat;

	mat.translate( snsr_xyz );
	mat.rotateHPR( snsr_hpr );

	dtkVec3 loc_xyz;
	if( station.has_position )
	{
		loc_xyz = dtkVec3( station.mapped_position );
	}

	if( station.has_angles )
	{
		dtkVec3 loc_hpr( station.mapped_orientation_hpr );
		mat.rotateHPR( loc_hpr );
	}
	else if( station.has_quaternion )
	{
		mat.quat( station.mapped_quaternion[0], station.mapped_quaternion[1],
				station.mapped_quaternion[2], station.mapped_quaternion[3] );
	}
	else if( station.has_cosinesx && station.has_cosinesy &&
			station.has_cosinesz )
	{
		dtkMatrix mult_mat;
		for( int i=0;i<3;i++ )
		{
			mult_mat.element( i, 1, station.mapped_cosines_x[i] );
			mult_mat.element( i, 2, station.mapped_cosines_y[i] );
			mult_mat.element( i, 3, station.mapped_cosines_z[i] );
		}
		mat.mult( &mult_mat );
	}

	mat.translate( loc_xyz );
	mat.rotateHPR( xmtr_hpr );
	mat.translate( xmtr_xyz );

	if( station.has_position || station.has_angles )
	{

		dtkCoord output;
		mat.coord( &output );
		for( int i=0;i<3;i++ )
		{
			if( station.has_position )
				station.mapped_position[i] = output.d[i];
			if( station.has_angles )
				station.mapped_orientation_hpr[i] = output.d[i+3];
		}
	}
	if( station.has_quaternion )
	{
		mat.quat( &station.mapped_quaternion[0], &station.mapped_quaternion[1],
				&station.mapped_quaternion[2], &station.mapped_quaternion[3] );
	}
	if( station.has_cosinesx && station.has_cosinesy && station.has_cosinesz )
	{
		for( int i=0;i<3;i++ )
		{
			station.mapped_cosines_x[i] = mat.element( i, 1 );
			station.mapped_cosines_y[i] = mat.element( i, 2 );
			station.mapped_cosines_z[i] = mat.element( i, 3 );
		}
	}

	return 0;
}

int IS900::sendCommand( const string& cmd )
{
	string cur_cmd = cmd;
	if( cur_cmd.size() && cur_cmd.size() > 1 )
	{
		cur_cmd += string( "\r\n" );
	}

#ifdef DTK_ARCH_WIN32_VCPP
	DWORD write_size;
	WriteFile( fd, cur_cmd.c_str(), cur_cmd.size(), NULL, &io_overlapped );
	GetOverlappedResult( fd, &io_overlapped, &write_size, true );
	size_t bytes = (size_t)write_size;
	if( bytes != cur_cmd.size() )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
				"IS900::sendCommand() failed: error sending command string to tracker:\n"
				"WriteFile(fd=%d,cmd=%s,size=%d,size_wrote=%d) to device file %s%s%s failed.",
				fd, cur_cmd.c_str(), cur_cmd.size(), bytes,
				dtkMsg.color.tur, device.port.c_str(), dtkMsg.color.end );
		return -1;
	}
#else
	size_t bytes = write( fd, cur_cmd.c_str(), cur_cmd.size() );
	if( bytes != cur_cmd.size() )
	{
		dtkMsg.add( DTKMSG_ERROR, 1,
				"IS900::sendCommand() failed: error sending command string to tracker:\n"
				"write(fd=%d,cmd=%s,size=%d) to device file %s%s%s failed.", fd,
				cur_cmd.c_str(), cur_cmd.size(),
				dtkMsg.color.tur, device.port.c_str(), dtkMsg.color.end );
		return -1;
	}
#endif
	return 0;
}

int IS900::requestDataRecord()
{
	string cmd = "P";
	return sendCommand( cmd );
}

int IS900::sendOutputMode( bool polled )
{
	device.polled = polled;
	string cmd;
	if( polled )
		cmd = "c";
	else
		cmd = "C";
	return sendCommand( cmd );
}

int IS900::requestAlignmentReferenceFrame( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "A%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendAlignmentReferenceFrame( const char& unit, float Ox, float Oy, float Oz,
		float Xx, float Xy, float Xz, float Yx, float Yy, float Yz )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "A%c,%3.2f,%3.2f,%3.2f,%3.2f,%3.2f,%3.2f,%3.2f,%3.2f,%3.2f", getStationID( unit ),
			Ox, Oy, Oz, Xx, Xy, Xz, Yx, Yy, Yz );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendResetAlignmentReferenceFrame( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "R%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestBoresightReferenceAngles( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "G%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendBoresightReferenceAngles( const char& unit, float heading, float pitch,
		float roll )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "G%c,%3.2f,%3.2f,%3.2f", getStationID( unit ),
			heading, pitch, roll );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendBoresightCompatibilityMode( bool fastrak )
{
	device.fastrak_compatibility = fastrak;
	string cmd;
	if( fastrak )
		cmd = "MBF";
	else
		cmd = "MBI";
	return sendCommand( cmd );
}

int IS900::sendBoresight( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	if( device.fastrak_compatibility )
	{
		sprintf( temp, "B%c", getStationID( unit ) );
	}
	else
	{
		sprintf( temp, "MB%c", getStationID( unit ) );
	}
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendUnboresight( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	if( device.fastrak_compatibility )
	{
		sprintf( temp, "b%c", getStationID( unit ) );
	}
	else
	{
		sprintf( temp, "Mb%c", getStationID( unit ) );
	}
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendHeadingBoresight( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	if( device.fastrak_compatibility )
	{
		sprintf( temp, "MB%c", getStationID( unit ) );
	}
	else
	{
		sprintf( temp, "B%c", getStationID( unit ) );
	}
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendHeadingUnboresight( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	if( device.fastrak_compatibility )
	{
		sprintf( temp, "Mb%c", getStationID( unit ) );
	}
	else
	{
		sprintf( temp, "b%c", getStationID( unit ) );
	}
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendSetSerialCommunicationParameters( int rate, parity_type parity,
		bits_type bits, bool handshake )
{
	int rate_num = rate / 100;
	switch( rate_num )
	{
		case 3:
		case 12:
		case 24:
		case 48:
		case 96:
		case 192:
		case 384:
		case 576:
		case 1152:
			break;
		default:
			dtkMsg.add( DTKMSG_ERROR, "IS900::sendSetSerialCommunicationParameters failed - the baud rate %d is invalid.\n",
					rate );
			return -1;
	}
	int handshake_code;
	if( handshake )
		handshake_code = 1;
	else
		handshake_code = 0;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "o%d,%d,%d,%d", rate_num, parity, bits, handshake_code );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestSystemRecord()
{
	string cmd = "S";
	return sendCommand( cmd );
}

int IS900::requestStationStatus( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "I%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendStationStatus( const char& unit, bool enabled )
{
	device.stations[unit].enabled = enabled;
	char temp[TEMP_BUFFER_SIZE];
	int status_code;
	if( enabled )
		status_code = 1;
	else
		status_code = 0;
	sprintf( temp, "I%c,%d", getStationID( unit ), status_code );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendOutputUnitsControl( bool inches )
{
	device.inches = inches;
	string cmd;
	if( inches )
		cmd = "U";
	else
		cmd = "u";
	return sendCommand( cmd );
}

int IS900::sendSaveCurrentSettings()
{
	string cmd = "^K";
	return sendCommand( cmd );
}

int IS900::sendRestoreSettingsFactoryDefault()
{
	string cmd = "W";
	return sendCommand( cmd );
}

int IS900::sendFirmwareRestart()
{
	string cmd = "^Y";
	return sendCommand( cmd );
}

int IS900::sendSuspendDataTransmission()
{
	string cmd = "^S";
	return sendCommand( cmd );
}

int IS900::sendResumeDataTransmission()
{
	string cmd = "^Q";
	return sendCommand( cmd );
}

int IS900::sendOutputRecordMode( bool ascii )
{
	device.ascii = ascii;
	string cmd;
	if( ascii )
		cmd = "F";
	else
		cmd = "f";
	return sendCommand( cmd );
}

int IS900::requestOutputRecordList( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "O%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendOutputRecordList( const char& unit,
		vector<output_record_type> output_record_list )
{
	if( !output_record_list.size() )
	{
		dtkMsg.add( DTKMSG_ERROR, "IS900::sendOutputRecordList failed - "
				"the output_record_list was empty.\n" );
		return -1;
	}
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "O%c", getStationID( unit ) );
	string cmd = temp;
	for( unsigned int i=0;i<output_record_list.size();i++ )
	{
		cmd += ",";
		sprintf( temp, "%d", output_record_list[i] );
		cmd += temp;
	}
	return sendCommand( cmd );
}

int IS900::requestDefineTipOffsets( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "N%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendDefineTipOffsets( const char& unit, float Ox, float Oy, float Oz )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "N%c,%f,%f,%f", getStationID( unit ), Ox, Oy, Oz );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestPositionOperationalEnvelope( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "V%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendPositionOperationalEnvelope( const char& unit, float Xmax, float Ymax,
		float Zmax, float Xmin, float Ymin, float Zmin )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "V%c,%f,%f,%f,%f,%f,%f", getStationID( unit ), Xmax, Ymax, Zmax, Xmin, Zmin );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestHemisphere( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "H%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendHemisphere( const char& unit, float p1, float p2, float p3 )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "H%c,%f,%f,%f", getStationID( unit ), p1, p2, p3 );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendTimeUnits( bool milliseconds )
{
	device.milliseconds = milliseconds;
	string cmd;
	if( milliseconds )
		cmd = "MT";
	else
		cmd = "Mt";
	return sendCommand( cmd );
}

int IS900::sendSetCurrentTimeZero()
{
	string cmd = "MZ";
	return sendCommand( cmd );
}

int IS900::requestEthernetAddress()
{
	string cmd = "MEthIp";
	return sendCommand( cmd );
}

int IS900::sendEthernetAddress( string address )
{
	device.eth_address = address;
	int ip_address[4];
	int ret_val = sscanf( address.c_str(), "%d.%d.%d.%d", &ip_address[0], &ip_address[1], &ip_address[2], &ip_address[3] );
	if( ret_val == EOF )
	{
		dtkMsg.add( DTKMSG_ERROR, "IS900::sendEthernetAddress failed - IP address \"%s\"\n"
				"must use dot format (i.e. 192.168.1.1)\n", address.c_str() );
		return -1;
	}
	for( int i=0;i<4;i++ )
	{
		if( ip_address[i] < 0 || ip_address[i] > 255 )
		{
			dtkMsg.add( DTKMSG_ERROR, "IS900::sendEthernetAddress failed - IP address \"%s\"\n"
					"must use dot format (i.e. 192.168.1.1)\n", address.c_str() );
			return -1;
		}
	}
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MEthIp%s", address.c_str() );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestEthernetState()
{
	string cmd = "MEthUdp";
	return sendCommand( cmd );
}

int IS900::sendEthernetState( bool enabled )
{
	device.eth_state = enabled;
	int state_code;
	if( enabled )
		state_code = 1;
	else
		state_code = 2;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MEthUdp%d", state_code );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestEthernetPort()
{
	string cmd = "MEthUdpPort";
	return sendCommand( cmd );
}

int IS900::sendEthernetPort( int port )
{
	device.eth_port = port;
	if( port < 0 || port > 65535 )
	{
		dtkMsg.add( DTKMSG_ERROR, "IS900::sendEthernetPort failed - invalid port.\n"
				"The port number must be between 0 and 65535.\n" );
		return -1;
	}
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MEthUdpPort%d", port );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestIntersenseStatusRecord()
{
	string cmd = "MS";
	return sendCommand( cmd );
}

int IS900::requestTrackingStatusRecord()
{
	string cmd = "MP";
	return sendCommand( cmd );
}

int IS900::requestUltrasonicTimeoutInterval()
{
	string cmd = "MU";
	return sendCommand( cmd );
}
/*
int IS900::sendUltrasonicTimeoutInterval( int interval )
{
	device.ultrasonic_timeout_interval = interval;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MU%d", interval );
	string cmd = temp;
	return sendCommand( cmd );
}
*/
int IS900::requestUltrasonicReceiverSensitivity()
{
	string cmd = "Mg";
	return sendCommand( cmd );
}
/*
int IS900::sendUltrasonicReceiverSensitivity( int level )
{
	device.ultrasonic_receiver_sensitivity = level;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "Mg%d", level );
	string cmd = temp;
	return sendCommand( cmd );
}
*/
int IS900::requestGenlockSynchronization()
{
	string cmd = "MG";
	return sendCommand( cmd );
}

int IS900::sendGenlockSynchronization( genlock_sync_type state, int rate )
{
	device.genlock_sync_state = state;
	device.genlock_sync_rate = rate;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MG%d,%d", state, rate );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestGenlockPhase()
{
	string cmd = "MGP";
	return sendCommand( cmd );
}

int IS900::sendGenlockPhase( string percent )
{
	device.genlock_phase = percent;
	int percent_value;
	if( percent == "+" || percent == "-" )
		;
	else
	{
		int ret_val = sscanf( percent.c_str(), "%d", &percent_value );
		if( ret_val == EOF )
		{
			dtkMsg.add( DTKMSG_ERROR, "IS900::sendGenlockPhase failed - invalid percent.\n"
					"The percent value must be \"+\", \"-\", or an integer between 0 and 100.\n" );
			return -1;
		}
		char temp[TEMP_BUFFER_SIZE];
		sprintf( temp, "%d", percent_value );
		percent = temp;
	}
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MGP%d", percent.c_str() );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestGenlockSyncSource()
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MGS" );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendGenlockSyncSource( sync_source_type source )
{
	device.genlock_sync_source = source;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MGS%d", source );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestConfigurationLock()
{
	string cmd = "MConfigLockMode";
	return sendCommand( cmd );
}

int IS900::sendConfigurationLock( config_lock_type mode )
{
	device.configuration_lock = mode;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MConfigLockMode%d", mode );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendSonistripLEDControl( bool enabled )
{
	device.led_control = enabled;
	int led_state;
	if( enabled )
		led_state = 1;
	else
		led_state = 0;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "ML%d", led_state );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestBeaconScheduler()
{
	string cmd = "MSchAlg";
	return sendCommand( cmd );
}

int IS900::sendBeaconScheduler( beacon_scheduler_type algorithm )
{
	device.beacon_scheduler = algorithm;
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MSchAlg%c", algorithm );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestErrorReport()
{
	string cmd = "ME";
	return sendCommand( cmd );
}

int IS900::sendClearErrorReport()
{
	string cmd = "MEC";
	return sendCommand( cmd );
}

int IS900::sendErrorReporting( bool enabled )
{
	device.error_reporting = enabled;
	string cmd;
	if( enabled )
		cmd = "ME1";
	else
		cmd = "ME0";
	return sendCommand( cmd );
}

int IS900::sendCommandLogging( bool enabled )
{
	device.command_logging = enabled;
	string cmd;
	if( enabled )
		cmd = "MLogOpen";
	else
		cmd = "MLogClose";
	return sendCommand( cmd );
}

int IS900::sendClearCommandLog()
{
	string cmd = "MLogClear";
	return sendCommand( cmd );
}

int IS900::requestCommandLogState()
{
	string cmd = "MLogState";
	return sendCommand( cmd );
}

int IS900::requestCommandLog()
{
	string cmd = "MLogSend";
	return sendCommand( cmd );
}

int IS900::requestIntersenseStationStatusRecord( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "Ms%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestPredictionInterval( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "Mp%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendPredictionInterval( const char& unit, int interval )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "Mp%c,%d", getStationID( unit ), interval );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendPerceptualEnhancementLevel( const char& unit,
		perceptual_enhancement_type mode )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MF%c,%d", getStationID( unit ), mode );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendCompassHeadingCorrection( const char& unit,
		compass_heading_correction_type mode )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MH%c,%d", getStationID( unit ), mode );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendResetCompassHeadingCorrection( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "Mh%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestRotationalSensitivity( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MQ%c", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendRotationalSensitivity( const char& unit,
		rotational_sensitivity_type level )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MQ%c,%d", getStationID( unit ), level );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::requestAssociateFixedPSEWithConstellation( const char& unit )
{
	char temp[TEMP_BUFFER_SIZE];
	string cmd = temp;
	if( unit == 0 )
		cmd = "MCF";
	else
	{
		sprintf( temp, "MCF%c", getStationID( unit ) );
		cmd = temp;
	}
	return sendCommand( cmd );
}

int IS900::sendAssociateFixedPSEWithConstellation( const char& unit, float Px,
		float Py, float Pz, float Nx, float Ny, float Nz, int hardware_id )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MCF%c,%10.5f,%10.5f,%10.5f,%10.5f,%10.5f,%10.5f,%d", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendDisassociateFixedPSEFromConstellation( const char& unit, int hardware_id )
{
	char temp[TEMP_BUFFER_SIZE];
	sprintf( temp, "MCf%c,%d", getStationID( unit ) );
	string cmd = temp;
	return sendCommand( cmd );
}

int IS900::sendClearAllFixedPSEsFromConstellation()
{
	string cmd = "MCC";
	return sendCommand( cmd );
}

int IS900::sendApplyConstellation()
{
	string cmd = "MCe";
	return sendCommand( cmd );
}

int IS900::sendCancelConstellationConfiguration()
{
	string cmd = "MCx";
	return sendCommand( cmd );
}

int IS900::sendConstellationFile( string& filename )
{
	ifstream file;
	file.open( filename.c_str() );
	if( !file.is_open() )
	{
		dtkMsg.add( DTKMSG_ERROR, "IS900::sendConstellationFile failed - unable to open\n"
				"constellation file \"%s\".\n", filename.c_str() );
		return -1;
	}
	filebuf* fbuf = file.rdbuf();
	long size = fbuf->pubseekoff( 0, ios::end, ios::in );
	fbuf->pubseekpos( 0, ios::in );

	char* temp = new char[size+1];
	if( !temp )
	{
		dtkMsg.add( DTKMSG_ERROR, "IS900::sendConstellationFile failed - unable to allocate memory (%d bytes).\n",
				size + 1 );
		return -1;
	}
	file.read( temp, size );
	file.close();
	temp[size] = '\0';

	vector<string> lines;
	tokenize( temp, lines, "\r\n" );

	delete temp;

	dtkMsg.add( DTKMSG_NOTICE, "Uploading constellation file %s.\n",
			filename.c_str() );
	for( unsigned int i=0;i<lines.size();i++ )
	{
		dtkMsg.append( "%s\n", lines[i].c_str() );
		if( sendCommand( lines[i] ) )
		{
			return -1;
		}
	}
	return 0;
}

static dtkService *dtkDSO_loader( const char *arg )
{
	return new IS900( arg );
}

static int dtkDSO_unloader( dtkService *svc )
{
	delete svc;
	return DTKDSO_UNLOAD_CONTINUE;
}

