#include <al.h>
#include <alut.h>
#include <string>
#include <vector>
#include <map>
#include "_config.h"
#include "config.h"
#include "dtkConfigure.h"
#include "dtkVec3.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkSoundBuffer.h"
#include "dtkSoundSource.h"

dtkSoundSource::dtkSoundSource( const std::string& name )
:m_name( name )
{
	m_is_queued = false;
	m_source_id = 0;
	ALenum error = AL_NONE;

	alGenSources( 1, &m_source_id );
	error = alGetError();
	if( error )
	{
	}

	alGetSourcei( m_source_id, AL_SOURCE_RELATIVE, &m_is_source_relative_to_listener );
	error = alGetError();
	if( error )
	{
	}
}

dtkSoundSource::dtkSoundSource( dtkSoundBuffer* buffer, const std::string& name )
:m_name( name )
{
	m_is_queued = false;
	m_source_id = 0;
	ALenum error = AL_NONE;

	alGenSources( 1, &m_source_id );
	error = alGetError();
	if( error )
	{
	}

	alSourcei( m_source_id, AL_BUFFER, buffer->getBufferID() );
	error = alGetError();
	if( error )
	{
	}
	else
		m_buffers[buffer->getName()] = buffer;
}

dtkSoundSource::dtkSoundSource( const std::vector<dtkSoundBuffer*> buffers,
		const std::string& name )
:m_name( name )
{
	m_is_queued = true;
	m_source_id = 0;
	ALenum error = AL_NONE;

	alGenSources( 1, &m_source_id );
	error = alGetError();
	if( error )
	{
	}

	if( buffers.size() )
	{
		unsigned int* buffer_ids = new unsigned int[buffers.size() ];
		int num = 0;
		dtkSoundBuffer* buffer;
		for( unsigned int i=0;i<buffers.size();i++ )
		{
			buffer = buffers[i];
			if( !buffer )
				continue;

			if( buffer->isValid() )
			{
				buffer_ids[num] = buffer->getBufferID();
				m_buffers[buffer->getName()] = buffer;
				num++;
			}
		}

		alSourceQueueBuffers( m_source_id, num, buffer_ids );
		error = alGetError();
		if( error )
		{
		}
		delete [] buffer_ids;
	}
}

dtkSoundSource::~dtkSoundSource()
{
	alDeleteSources( 1, &m_source_id );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_ERROR, "dtkSoundSource::~dtkSoundSource() - "
				"alDeleteSources( 1, %d ) FAILED: %s\n",
				m_source_id, alutGetErrorString( error ) );
	}
}

int dtkSoundSource::addBuffer( dtkSoundBuffer* buffer )
{
	if( !buffer )
	{
		return - 1;
	}

	std::map<std::string, dtkSoundBuffer*>::iterator itr =
			m_buffers.find( buffer->getName() );
	if( itr == m_buffers.end() )
	{
		if( m_is_queued )
			m_buffers[buffer->getName()] = buffer;
		else
		{
			
		}
	}
	else
	{
		dtkMsg.add( DTKMSG_ERROR, "A buffer named %s already exists\n",
				buffer->getName().c_str() );
		return -1;
	}

	ALenum error;

	ALuint buffer_id = buffer->getBufferID();
	if( m_is_queued )
	{
		alSourceQueueBuffers( m_source_id, 1, &buffer_id );
		error = alGetError();
		if( error )
		{
		}
	}
	else
	{
		alSourcei( m_source_id, AL_BUFFER, buffer_id );
		error = alGetError();
		if( error )
		{
		}
	}

	return 0;
}

int dtkSoundSource::addBuffers( std::vector<dtkSoundBuffer*> buffers )
{
	if( !buffers.size() )
	{
		return -1;
	}

	ALenum error;

	std::map<std::string, dtkSoundBuffer*>::iterator itr;
	dtkSoundBuffer* buffer;
	for( unsigned int i=0;i<buffers.size();i++ )
	{
		buffer = buffers[i];
		if( !buffer )
			continue;

		itr = m_buffers.find( buffer->getName() );
		if( itr == m_buffers.end() )
			m_buffers[buffer->getName()] = buffer;
		else
		{
			dtkMsg.add( DTKMSG_ERROR, "A buffer named %s already exists\n",
					buffer->getName().c_str() );
			return -1;
		}

		ALuint buffer_id = buffer->getBufferID();
		if( m_is_queued )
		{
			alSourceQueueBuffers( m_source_id, 1, &buffer_id );
			error = alGetError();
			if( error )
			{
			}
		}
		else
		{
			alSourcei( m_source_id, AL_BUFFER, buffer_id );
			error = alGetError();
			if( error )
			{
			}
		}
	}

	return 0;
}

int dtkSoundSource::removeBuffer( const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator itr =
			m_buffers.find( name.c_str() );
	if( itr != m_buffers.end() )
	{
		ALenum error;
		ALuint buffer_id = itr->second->getBufferID();
		if( m_is_queued )
		{
			alSourceUnqueueBuffers( m_source_id, 1, &buffer_id );
			error = alGetError();
			if( error )
			{
				dtkMsg.add( DTKMSG_WARNING, "dtkSoundSource::removeBuffer( %s ) - "
						"alSourceUnqueueBuffers( %d, 1, %d ) FAILED: %s\n",
						name.c_str(), m_source_id, buffer_id,
						alutGetErrorString( error ) );
				return -1;
			}
		}
		else
		{
			alSourcei( m_source_id, AL_BUFFER, 0 );
			error = alGetError();
			if( error )
			{
				dtkMsg.add( DTKMSG_WARNING, "dtkSoundSource::removeBuffer( %s ) - "
						"alSourcei( %d, AL_BUFFER, 0 ) FAILED: %s\n",
						name.c_str(), m_source_id, alutGetErrorString( error ) );
				return -1;
			}
		}
		m_buffers.erase( itr );
		return 0;
	}

	return -1;
}

int dtkSoundSource::removeAllBuffers()
{
	alSourcei( m_source_id, AL_BUFFER, 0 );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundSource::removeAllBuffers() - "
				"alSourcei( %d, AL_BUFFER, 0 ) FAILED: %s\n",
				m_source_id, alutGetErrorString( error ) );
		return -1;
	}

	m_buffers.clear();
	return 0;
}

dtkSoundBuffer* dtkSoundSource::getBuffer( const std::string& name )
{
	std::map<std::string, dtkSoundBuffer*>::iterator itr =
			m_buffers.find( name.c_str() );
	if( itr != m_buffers.end() )
	{
		return itr->second;
	}

	return NULL;
}

dtkSoundBuffer* dtkSoundSource::getCurrentBuffer()
{
/*	ALuint current_buffer;
	alGetSource( m_source_id, AL_BUFFER, &current_buffer );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, "dtkSoundSource::getCurrentBuffer() - "
				"alGetSource( %d, AL_BUFFER, &current_buffer ) FAILED: %s\n",
				m_source_id, alutGetErrorString( error ) );
		return NULL
	}

*/
	return NULL;	
}

void dtkSoundSource::setQueued( bool queued )
{
	if( queued == m_is_queued )
		return;

	if( m_is_queued )
	{
		std::map<std::string, dtkSoundBuffer*>::iterator itr = m_buffers.begin();
		for( ;itr!=m_buffers.end();itr++ )
		{
			
		}
	}
	else
	{
	}

	m_is_queued = queued;
}

void dtkSoundSource::setSourceRelativeToListener( bool relative )
{
	alSourcei( m_source_id, AL_SOURCE_RELATIVE, relative );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_is_source_relative_to_listener = relative;
}

bool dtkSoundSource::isSourceRelativeToListener()
{
	return m_is_source_relative_to_listener;
}

void dtkSoundSource::setLooping( bool is_looping )
{
	alSourcei( m_source_id, AL_LOOPING, is_looping );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_is_looping = is_looping;
}

bool dtkSoundSource::isLooping()
{
	return m_is_looping;
}

void dtkSoundSource::setPosition( const dtkVec3& xyz )
{
	alSourcefv( m_source_id, AL_POSITION, xyz.d );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_position = xyz;
}

dtkVec3 dtkSoundSource::getPosition()
{
	return m_position;
}

void dtkSoundSource::setVelocity( const dtkVec3& velocity )
{
	alSourcefv( m_source_id, AL_VELOCITY, velocity.d );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_velocity = velocity;
}

dtkVec3 dtkSoundSource::getVelocity()
{
	return m_velocity;
}

void dtkSoundSource::setGain( const float& gain )
{
	alSourcef( m_source_id, AL_GAIN, gain );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_gain = gain;
}

float dtkSoundSource::getGain()
{
	return m_gain;
}

void dtkSoundSource::setMinGain( const float& min_gain )
{
	alSourcef( m_source_id, AL_MIN_GAIN, min_gain );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_min_gain = min_gain;
}

float dtkSoundSource::getMinGain()
{
	return m_min_gain;
}

void dtkSoundSource::setMaxGain( const float& max_gain )
{
	alSourcef( m_source_id, AL_MAX_GAIN, max_gain );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_max_gain = max_gain;
}

float dtkSoundSource::getMaxGain()
{
	return m_max_gain;
}

void dtkSoundSource::setReferenceDistance( const float& reference_distance )
{
	alSourcef( m_source_id, AL_REFERENCE_DISTANCE, reference_distance );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_reference_distance = reference_distance;
}

float dtkSoundSource::getReferenceDistance()
{
	return m_reference_distance;
}

void dtkSoundSource::setRolloffFactor( const float& rolloff_factor )
{
	alSourcef( m_source_id, AL_ROLLOFF_FACTOR, rolloff_factor );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_rolloff_factor = rolloff_factor;
}

float dtkSoundSource::getRolloffFactor()
{
	return m_rolloff_factor;
}

void dtkSoundSource::setMaxDistance( const float& max_distance )
{
	alSourcef( m_source_id, AL_MAX_DISTANCE, max_distance );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_max_distance = max_distance;
}

float dtkSoundSource::getMaxDistance()
{
	return m_max_distance;
}

void dtkSoundSource::setPitch( const float& pitch )
{
	alSourcef( m_source_id, AL_PITCH, pitch );
	ALenum error = alGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_WARNING, " dtkSoundSource::setPitch - "
				"alSourcef( %d, AL_PITCH, %10.4f ) FAILED: %s\n",
				m_source_id, pitch, alutGetErrorString( error ) );
	}
	else
		m_pitch = pitch;
}

float dtkSoundSource::getPitch()
{
	return m_pitch;
}

void dtkSoundSource::setDirection( const dtkVec3& direction )
{
	alSourcefv( m_source_id, AL_DIRECTION, direction.d );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_direction = direction;
}

dtkVec3 dtkSoundSource::getDirection()
{
	return m_direction;
}

void dtkSoundSource::setConeInnerAngle( const float& inner_angle )
{
	alSourcef( m_source_id, AL_CONE_INNER_ANGLE, inner_angle );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_inner_angle = inner_angle;
}

float dtkSoundSource::getInnerConeAngle()
{
	return m_inner_angle;
}

void dtkSoundSource::setConeOuterAngle( const float& outer_angle )
{
	alSourcef( m_source_id, AL_CONE_OUTER_ANGLE, outer_angle );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_outer_angle = outer_angle;
}

float dtkSoundSource::getOuterConeAngle()
{
	return m_outer_angle;
}

void dtkSoundSource::setConeOuterGain( const float& outer_gain )
{
	alSourcef( m_source_id, AL_CONE_OUTER_GAIN, outer_gain );
	ALenum error = alGetError();
	if( error )
	{
	}
	else
		m_outer_gain = outer_gain;
}

float dtkSoundSource::getConeOuterGain()
{
	return m_outer_gain;
}

void dtkSoundSource::setSecondsOffset( const float& offset )
{
	alSourcef( m_source_id, AL_SEC_OFFSET, offset );
	ALenum error = alGetError();
	if( error )
	{
	}
}

float dtkSoundSource::getSecondsOffset()
{
	float offset;
	alGetSourcef( m_source_id, AL_SEC_OFFSET, &offset );
	ALenum error = alGetError();
	if( error )
	{
		return 0.0f;
	}
	return offset;
}

void dtkSoundSource::setSamplesOffset( const float& offset )
{
	alSourcef( m_source_id, AL_SAMPLE_OFFSET, offset );
	ALenum error = alGetError();
	if( error )
	{
	}
}

float dtkSoundSource::getSamplesOffset()
{
	float offset;
	alGetSourcef( m_source_id, AL_SAMPLE_OFFSET, &offset );
	ALenum error = alGetError();
	if( error )
	{
		return 0.0f;
	}
	return offset;
}

void dtkSoundSource::setBytesOffset( const int& offset )
{
	alSourcei( m_source_id, AL_BYTE_OFFSET, offset );
	ALenum error = alGetError();
	if( error )
	{
	}
}

int dtkSoundSource::getBytesOffset()
{
	int offset;
	alGetSourcei( m_source_id, AL_BYTE_OFFSET, &offset );
	ALenum error = alGetError();
	if( error )
	{
		return -1;
	}
	return offset;
}

int dtkSoundSource::play()
{
	if( !m_buffers.size() )
	{
		dtkMsg.add( DTKMSG_WARNING, "There are no buffers associated with source %s\n",
			m_name.c_str() );
		return -1;
	}

	alSourcePlay( m_source_id );
	ALenum error = alGetError();
	if( error )
	{
	}
	return 0;
}

int dtkSoundSource::pause()
{
	alSourcePause( m_source_id );
	ALenum error = alGetError();
	if( error )
	{
	}
	return 0;
}

int dtkSoundSource::stop()
{
	alSourceStop( m_source_id );
	ALenum error = alGetError();
	if( error )
	{
	}
	return 0;
}

int dtkSoundSource::rewind()
{
	alSourceRewind( m_source_id );
	ALenum error = alGetError();
	if( error )
	{
	}
	return 0;
}

bool dtkSoundSource::isPlaying()
{
	ALint state;
	alGetSourcei( m_source_id, AL_SOURCE_STATE, &state );

	ALenum error = alGetError();
	if( error )
	{
	}

	if( state == AL_PLAYING )
		return true;

	return false;
}

bool dtkSoundSource::isPaused()
{
	ALint state;
	alGetSourcei( m_source_id, AL_SOURCE_STATE, &state );

	ALenum error = alGetError();
	if( error )
	{
	}

	if( state == AL_PAUSED )
		return true;

	return false;
}

bool dtkSoundSource::isStopped()
{
	ALint state;
	alGetSourcei( m_source_id, AL_SOURCE_STATE, &state );

	ALenum error = alGetError();
	if( error )
	{
	}

	if( state == AL_STOPPED )
		return true;

	return false;
}

bool dtkSoundSource::isRewound()
{
	ALint state;
	alGetSourcei( m_source_id, AL_SOURCE_STATE, &state );

	ALenum error = alGetError();
	if( error )
	{
	}

	if( state == AL_INITIAL )
		return true;

	return false;
}

int dtkSoundSource::getCurrentState()
{
	ALint state;
	alGetSourcei( m_source_id, AL_SOURCE_STATE, &state );

	ALenum error = alGetError();
	if( error )
	{
	}

	if( error )
	{
printf( "ERROR: dtkSoundSource::getCurrentState()\n" );
	}

	return state;
}

bool dtkSoundSource::isValid()
{
	bool is_source = alIsSource( m_source_id );

	ALenum error = alGetError();
	if( error )
	{
	}

	return is_source;
}

