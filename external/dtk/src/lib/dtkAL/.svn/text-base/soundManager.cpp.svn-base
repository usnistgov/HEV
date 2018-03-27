#include <al.h>
#include <alut.h>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include "_config.h"
#include "config.h"
#include "dtkConfigure.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkVec3.h"
#include "dtkMatrix.h"
#include "dtkSoundBuffer.h"
#include "dtkSoundSource.h"
#include "dtkSoundSyncedSources.h"
#include "dtkSoundManager.h"

bool dtkSoundManager::m_is_inited = false;
dtkSoundManager* dtkSoundManager::m_sound_manager = NULL;

dtkSoundManager::dtkSoundManager()
{
	ALenum error;

	m_distance_model = (distanceModelEnum)alGetInteger( AL_DISTANCE_MODEL );
	error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundManager::dtkSoundManager() - "
				"alGetInteger( AL_DISTANCE_MODEL ) FAILED: %s\n",
				alutGetErrorString( error ) );
	}

	m_doppler_factor = alGetFloat( AL_DOPPLER_FACTOR );
	error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundManager::dtkSoundManager() - "
				"alGetFloat( AL_DOPPLER_FACTOR ) FAILED: %s\n",
				alutGetErrorString( error ) );
	}

	m_speed_of_sound = alGetFloat( AL_SPEED_OF_SOUND );
	error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundManager::dtkSoundManager() - "
				"alGetFloat( AL_SPEED_OF_SOUND ) FAILED: %s\n",
				alutGetErrorString( error ) );
	}
}

dtkSoundManager::~dtkSoundManager()
{
	std::map<std::string, dtkSoundSyncedSources*>::iterator synced_source_itr =
			m_synced_sources.begin();
	for( ;synced_source_itr!=m_synced_sources.end();synced_source_itr++ )
	{
		synced_source_itr->second->stop();
		delete synced_source_itr->second;
		m_synced_sources.erase( synced_source_itr );
	}

	std::map<std::string, dtkSoundSource*>::iterator source_itr =
			m_sources.begin();
	for( ;source_itr!=m_sources.end();source_itr++ )
	{
		source_itr->second->stop();
		delete source_itr->second;
		m_sources.erase( source_itr );
	}

	std::map<std::string, dtkSoundBuffer*>::iterator buffer_itr =
			m_buffers.begin();
	for( ;buffer_itr!=m_buffers.end();buffer_itr++ )
	{
		delete buffer_itr->second;
		m_buffers.erase( buffer_itr );
	}
}

int dtkSoundManager::init()
{
	if( !m_is_inited )
	{
		if ( !alutInit( 0, NULL ) )
		{
			ALenum error = alutGetError();
			if( error )
			{
				dtkMsg.add( DTKMSG_ERROR, "dtkSoundManager::init() - alutInit FAILED: %s\n",
						alutGetErrorString( error ) );
				return -1;
			}
		}
		else
		{
			m_sound_manager = new dtkSoundManager();
			if( m_sound_manager )
			{
				m_is_inited = true;
			}
			else
			{
				alutExit();
				ALenum error = alutGetError();
				if( error )
				{
					dtkMsg.add( DTKMSG_ERROR, "dtkSoundManager::init() - "
							"alutExit FAILED: %s\n", alutGetErrorString( error ) );
				}
				return -1;
			}
		}
	}

	return 0;
}

int dtkSoundManager::close()
{
	if( m_is_inited )
	{
		delete m_sound_manager;
		m_is_inited = false;

		alutExit();
		ALenum error = alutGetError();
		if( error )
		{
			dtkMsg.add( DTKMSG_ERROR, "dtkSoundManager::close() - "
					"alutExit FAILED: %s\n", alutGetErrorString( error ) );
			return -1;
		}
	}
	return 0;
}

dtkSoundSource* dtkSoundManager::playSound( const std::string& filename,
		bool loop )
{
	dtkSoundBuffer* buffer = createBufferFromFile( filename,
			"dtkSoundManager::loop_buffer" );

	if( !buffer )
	{
		dtkMsg.add( DTKMSG_ERROR, "dtkSoundManager::playSound - "
				"Unable to create buffer from file %s\n",
				filename.c_str() );
		return NULL;
	}

	dtkSoundSource* source = createSourceFromBuffer( buffer,
			"dtkSoundManager::loop_source" );
	if( !source )
	{
		dtkMsg.add( DTKMSG_ERROR, "dtkSoundManager::playSound - "
				"Unable to create source rom buffer\n" );
		delete buffer;
		return NULL;
	}

	source->setSourceRelativeToListener( true );
	source->setLooping( true );
	source->play();
	return source;
}

dtkSoundSource* dtkSoundManager::playSounds(
		const std::vector<std::string>& filenames, bool loop )
{
	std::vector<dtkSoundBuffer*> buffers;
	dtkSoundBuffer* buffer;
	std::string base_name = "dtkSoundManager::loop_sound";
	std::string name;
	for( unsigned int i=0;i<filenames.size();i++ )
	{
		name = base_name;
		name += i;
		buffer = createBufferFromFile( filenames[i], name );
		if( !buffer )
		{
			dtkMsg.add( DTKMSG_ERROR, "dtkSoundManager::playSounds - "
					"Unable to create buffer from file %s\n",
					filenames[i].c_str() );
			continue;
		}
		buffers.push_back( buffer );
	}

	dtkSoundSource* source = createSourceFromBuffers( buffers,
			"dtkSoundManager::loop_source" );
	if( !source )
	{
		dtkMsg.add( DTKMSG_ERROR, "dtkSoundManager::playSound - "
				"Unable to create source rom buffers\n" );
		for( unsigned int i=0;i<buffers.size();i++ )
		{
			delete buffers[i];
		}
		return NULL;
	}

	source->setSourceRelativeToListener( true );
	source->setLooping( true );
	source->play();
	return source;
}

dtkSoundBuffer* dtkSoundManager::createBufferHelloWorld( const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator itr = m_buffers.find( name );
	if( itr == m_buffers.end() )
	{
		dtkSoundBuffer* buffer = new dtkSoundBuffer( name );

		if( buffer )
		{
			if( buffer->isValid() )
			{
				m_buffers[name] = buffer;
				return buffer;
			}
			else
			{
				delete buffer;
			}
		}
	}

	return NULL;
}

dtkSoundBuffer* dtkSoundManager::createBufferFromFile( const std::string& filename,
		const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator itr = m_buffers.find( name );
	if( itr == m_buffers.end() )
	{
		dtkSoundBuffer* buffer = new dtkSoundBuffer( filename, name );

		if( buffer )
		{
			if( buffer->isValid() )
			{
				m_buffers[name] = buffer;
				return buffer;
			}
			else
			{
				delete buffer;
			}
		}
	}

	return NULL;
}

dtkSoundBuffer* dtkSoundManager::createBufferFromFileImage( const void* data,
		int length, const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator itr = m_buffers.find( name );
	if( itr == m_buffers.end() )
	{
		dtkSoundBuffer* buffer = new dtkSoundBuffer( data, length, name );

		if( buffer )
		{
			if( buffer->isValid() )
			{
				m_buffers[name] = buffer;
				return buffer;
			}
			else
			{
				delete buffer;
			}
		}
	}

	return NULL;
}

dtkSoundBuffer* dtkSoundManager::createBufferWaveform( const dtkSoundBuffer::waveformEnum& waveshape,
		const float& frequency, const float& phase, const float& duration,
		const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator itr = m_buffers.find( name );
	if( itr == m_buffers.end() )
	{
		dtkSoundBuffer* buffer = new dtkSoundBuffer( waveshape, frequency, phase,
				duration, name );

		if( buffer )
		{
			if( buffer->isValid() )
			{
				m_buffers[name] = buffer;
				return buffer;
			}
			else
			{
				delete buffer;
			}
		}
	}

	return NULL;
}

dtkSoundBuffer* dtkSoundManager::getBuffer( const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator itr = m_buffers.find( name );
	if( itr != m_buffers.end() )
	{
		return itr->second;
	}

	return NULL;
}

bool dtkSoundManager::destroyBuffer( const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator itr = m_buffers.find( name );
	if( itr != m_buffers.end() )
	{
		delete itr->second;
		m_buffers.erase( itr );

		return true;
	}

	return false;
}

bool dtkSoundManager::destroyBuffer( dtkSoundBuffer* buffer )
{
	std::string name;
	if( buffer )
	{
		name = buffer->getName();
	}
	else
	{
		return false;
	}

	std::map<std::string, dtkSoundBuffer*>::iterator itr = m_buffers.find( name );
	if( itr != m_buffers.end() )
	{
		delete itr->second;
		m_buffers.erase( itr );

		return true;
	}

	return false;
}

dtkSoundSource* dtkSoundManager::createSourceEmpty( const std::string& name )
{
	std::map<std::string, dtkSoundSource*>::iterator itr = m_sources.find( name );
	if( itr == m_sources.end() )
	{
		dtkSoundSource* source = new dtkSoundSource( name );

		if( source )
		{
			if( source->isValid() )
			{
				m_sources[name] = source;
				return source;
			}
			else
			{
				delete source;
			}
		}
	}
	else
	{
		dtkMsg.add( DTKMSG_WARNING, "A source with the name %s already exists\n",
				name.c_str() );
	}

	return NULL;
}

dtkSoundSource* dtkSoundManager::createSourceFromBuffer( dtkSoundBuffer* buffer,
		const std::string& name )
{
	if( !buffer )
	{
		dtkMsg.add( DTKMSG_WARNING, "Cannot create source from a NULL buffer\n" );
		return NULL;
	}

	std::map<std::string, dtkSoundSource*>::iterator itr = m_sources.find( name );
	if( itr == m_sources.end() )
	{
		dtkSoundSource* source = new dtkSoundSource( buffer, name );

		if( source )
		{
			if( source->isValid() )
			{
				m_sources[name] = source;
				return source;
			}
			else
			{
				delete source;
			}
		}
	}
	else
	{
		dtkMsg.add( DTKMSG_WARNING, "A source with the name %s already exists\n",
				name.c_str() );
	}

	return NULL;
}

dtkSoundSource* dtkSoundManager::createSourceFromBuffer( const std::string& buffer_name,
		const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator buffer_itr =
			m_buffers.find( buffer_name );
	if( buffer_itr == m_buffers.end() )
	{
		dtkMsg.add( DTKMSG_WARNING, "Unable to find a buffer with name \"%s\"\n",
				buffer_name.c_str() );
		return NULL;
	}

	std::map<std::string, dtkSoundSource*>::iterator itr = m_sources.find( name );
	if( itr == m_sources.end() )
	{
		dtkSoundSource* source = new dtkSoundSource( buffer_itr->second, name );

		if( source )
		{
			if( source->isValid() )
			{
				m_sources[name] = source;
				return source;
			}
			else
			{
				delete source;
			}
		}
	}
	else
	{
		dtkMsg.add( DTKMSG_WARNING, "A source with the name %s already exists\n",
				name.c_str() );
	}

	return NULL;
}

dtkSoundSource* dtkSoundManager::createSourceFromBuffers(
		const std::vector<dtkSoundBuffer*>& buffers, const std::string& name )
{
	std::map<std::string, dtkSoundSource*>::iterator itr = m_sources.find( name );
	if( itr == m_sources.end() )
	{
		dtkSoundSource* source = new dtkSoundSource( buffers, name );

		if( source )
		{
			if( source->isValid() )
			{
				m_sources[name] = source;
				return source;
			}
			else
			{
				delete source;
			}
		}
	}
	else
	{
		dtkMsg.add( DTKMSG_WARNING, "A source with the name %s already exists\n",
				name.c_str() );
	}

	return NULL;
}

dtkSoundSource* dtkSoundManager::createSourceFromBuffers(
		const std::vector<std::string>& buffer_names, const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator buffer_itr;
	std::vector<dtkSoundBuffer*> buffers;
	for( unsigned int i=0;i<buffer_names.size();i++ )
	{
		buffer_itr = m_buffers.find( buffer_names[i] );
		if( buffer_itr != m_buffers.end() )
		{
			buffers.push_back( buffer_itr->second );
		}
		else
		{
			dtkMsg.add( DTKMSG_WARNING, "The buffer named %s does not exist\n",
					buffer_names[i].c_str() );
		}
	}

	std::map<std::string, dtkSoundSource*>::iterator itr = m_sources.find( name );
	if( itr == m_sources.end() )
	{
		dtkSoundSource* source = new dtkSoundSource( buffers, name );

		if( source )
		{
			if( source->isValid() )
			{
				m_sources[name] = source;
				return source;
			}
			else
			{
				delete source;
			}
		}
	}
	else
	{
		dtkMsg.add( DTKMSG_WARNING, "A source with the name %s already exists\n",
				name.c_str() );
	}

	return NULL;
}

dtkSoundSource* dtkSoundManager::getSource( const std::string& name )
{
	std::map<std::string, dtkSoundSource*>::iterator itr = m_sources.find( name );
	if( itr != m_sources.end() )
	{
		return itr->second;
	}

	return NULL;
}

bool dtkSoundManager::destroySource( const std::string& name )
{
	std::map<std::string, dtkSoundSource*>::iterator itr = m_sources.find( name );
	if( itr != m_sources.end() )
	{
		delete itr->second;
		m_sources.erase( itr );

		return true;
	}

	return false;
}

bool dtkSoundManager::destroySource( dtkSoundSource* source )
{
	std::string name;
	if( source )
	{
		name = source->getName();
	}
	else
	{
		return false;
	}

	std::map<std::string, dtkSoundSource*>::iterator itr = m_sources.find( name );
	if( itr != m_sources.end() )
	{
		delete itr->second;
		m_sources.erase( itr );

		return true;
	}

	return false;
}

dtkSoundSyncedSources* dtkSoundManager::createSyncedSources( const std::string& name )
{
	std::map<std::string, dtkSoundSyncedSources*>::iterator itr =
			m_synced_sources.find( name );
	if( itr == m_synced_sources.end() )
	{
		dtkSoundSyncedSources* synced_source = new dtkSoundSyncedSources( name );

		if( synced_source )
		{
			m_synced_sources[name] = synced_source;
			return synced_source;
		}
	}

	return NULL;
}

dtkSoundSyncedSources* dtkSoundManager::createSyncedSources(
		const std::vector<dtkSoundSource*>& sources, const std::string& name )
{
	std::map<std::string, dtkSoundSyncedSources*>::iterator itr =
			m_synced_sources.find( name );
	if( itr == m_synced_sources.end() )
	{
		dtkSoundSyncedSources* synced_source = new dtkSoundSyncedSources( sources, name );

		if( synced_source )
		{
			m_synced_sources[name] = synced_source;
			return synced_source;
		}
	}

	return NULL;
}

dtkSoundSyncedSources* dtkSoundManager::getSyncedSources( const std::string& name )
{
	std::map<std::string, dtkSoundSyncedSources*>::iterator itr =
			m_synced_sources.find( name );
	if( itr != m_synced_sources.end() )
	{
		return itr->second;
	}

	return NULL;
}

bool dtkSoundManager::destroySyncedSources( const std::string& name )
{
	std::map<std::string, dtkSoundSyncedSources*>::iterator itr =
			m_synced_sources.find( name );
	if( itr != m_synced_sources.end() )
	{
		delete itr->second;
		m_synced_sources.erase( itr );

		return true;
	}

	return false;
}

bool dtkSoundManager::destroySyncedSources( dtkSoundSyncedSources* synced_sources )
{
	std::string name;
	if( synced_sources )
	{
		name = synced_sources->getName();
	}
	else
	{
		return false;
	}

	std::map<std::string, dtkSoundSyncedSources*>::iterator itr =
			m_synced_sources.find( name );
	if( itr != m_synced_sources.end() )
	{
		delete itr->second;
		m_synced_sources.erase( itr );

		return true;
	}

	return false;
}

int dtkSoundManager::setListenerPosition( const dtkVec3& position )
{
	alListenerfv( AL_POSITION, position.d );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundManager::setListenerPosition - "
				"alListenerfv( AL_POSITION, position.d ) FAILED: %S\n",
				alutGetErrorString( error ) );
		return -1;
	}

	m_listener_xyz = position;
	return 0;
}

dtkVec3 dtkSoundManager::getListenerPosition()
{
	return m_listener_xyz;
}

int dtkSoundManager::setListenerVelocity( const dtkVec3& velocity )
{
	alListenerfv( AL_VELOCITY, velocity.d );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundManager::setListenerVelocity - "
				"alListenerfv( AL_VELOCITY, velocity.d ) FAILED: %S\n",
				alutGetErrorString( error ) );
		return -1;
	}

	m_listener_velocity = velocity;
	return 0;
}

dtkVec3 dtkSoundManager::getListenerVelocity()
{
	return m_listener_velocity;
}

int dtkSoundManager::setListenerGain( const float& gain )
{
	alListenerf( AL_GAIN, gain );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundManager::setListenerGain - "
				"alListenerf( AL_GAIN, gain ) FAILED: %S\n",
				alutGetErrorString( error ) );
		return -1;
	}

	m_listener_gain = gain;
	return 0;
}

float dtkSoundManager::getListenerGain()
{
	return m_listener_gain;
}

int dtkSoundManager::setListenerOrientation( const dtkVec3 hpr )
{
	float orientation[6];
	dtkMatrix mat;
	mat.rotateHPR( hpr );

	// because diverse coordinates are different from OpenGL and OpenAL coord system
	orientation[0] = mat.element( 0, 1 );
	orientation[1] = mat.element( 1, 1 );
	orientation[2] = mat.element( 2, 1 );
	orientation[3] = mat.element( 0, 2 );
	orientation[4] = mat.element( 1, 2 );
	orientation[5] = mat.element( 2, 2 );

	alListenerfv( AL_ORIENTATION, orientation );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundManager::setListenerOrientation - "
				"alListenerfv( AL_ORIENTATION, orientation ) FAILED: %S\n",
				alutGetErrorString( error ) );
		return -1;
	}

	mat.rotateHPR( &m_listener_hpr );
	return 0;
}

int dtkSoundManager::setListenerOrientation( const dtkVec3 at, const dtkVec3 up )
{
	float orientation[6];
	for( int i=0;i<3;i++ )
	{
		orientation[i] = at.d[i];
		orientation[i+3] = up.d[i];
	}

	alListenerfv( AL_ORIENTATION, orientation );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundManager::setListenerOrientation - "
				"alListenerfv( AL_ORIENTATION, orientation ) FAILED: %S\n",
				alutGetErrorString( error ) );
		return -1;
	}

	dtkVec3 right = at * up;

	dtkMatrix mat;
	for( int j=0;j<3;j++ )
	{
		mat.element( j, 0, right.d[j] );
		mat.element( j, 1, at.d[j] );
		mat.element( j, 2, up.d[j] );
	}

	mat.rotateHPR( &m_listener_hpr );
	return 0;
}

dtkVec3 dtkSoundManager::getListenerOrientation()
{
	return m_listener_hpr;
}

void dtkSoundManager::setDistanceModel( const distanceModelEnum& distance_model )
{
	alDistanceModel( distance_model );

	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING,
				"dtkSoundManager::setDistanceModel( %s ) failed\n%s\n",
				getDistanceModel( distance_model ).c_str(), alutGetErrorString( error ) );
	}
	else
		m_distance_model = distance_model;
}

std::string dtkSoundManager::getDistanceModel( const distanceModelEnum& distance_model )
{
	switch( distance_model )
	{
		case AL_NONE:
		{
			return "NONE";
			break;
		}
		case AL_INVERSE_DISTANCE:
		{
			return "INVERSE_DISTANCE";
			break;
		}
		case AL_INVERSE_DISTANCE_CLAMPED:
		{
			return "INVERSE_DISTANCE_CLAMPED";
			break;
		}
		case AL_LINEAR_DISTANCE:
		{
			return "LINEAR_DISTANCE";
			break;
		}
		case AL_LINEAR_DISTANCE_CLAMPED:
		{
			return "LINEAR_DISTANCE_CLAMPED";
			break;
		}
		case AL_EXPONENT_DISTANCE:
		{
			return "EXPONENT_DISTANCE";
			break;
		}
		case AL_EXPONENT_DISTANCE_CLAMPED:
		{
			return "EXPONENT_DISTANCE_CLAMPED";
			break;
		}
	}
	return "UNKNOWN";
}

void dtkSoundManager::setDopplerFactor( const float& doppler_factor )
{
	alDopplerFactor( doppler_factor );

	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING,
				"dtkSoundManager::setDopplerFactor( %10.4f ) failed\n%s\n",
				doppler_factor, alutGetErrorString( error ) );
	}
	else
		m_doppler_factor = doppler_factor;
}

void dtkSoundManager::setSpeedOfSound( const float& speed_of_sound )
{
	alSpeedOfSound( speed_of_sound );

	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING,
				"dtkSoundManager::setSpeedOfSound( %10.4f ) failed\n%s\n",
				speed_of_sound, alutGetErrorString( error ) );
	}
	else
		m_speed_of_sound = speed_of_sound;
}

dtkSoundManager* dtkSoundManager::getInstance()
{
	if( m_is_inited )
		return m_sound_manager;

	return NULL;
}

std::string dtkSoundManager::getSupportedMIMETypes()
{
	return alutGetMIMETypes( ALUT_LOADER_BUFFER );
}

