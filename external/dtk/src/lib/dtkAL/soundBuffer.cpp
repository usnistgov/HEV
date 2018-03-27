#include <al.h>
#include <alut.h>
#include <string>
#include "_config.h"
#include "config.h"
#include "dtkConfigure.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkSoundBuffer.h"

dtkSoundBuffer::dtkSoundBuffer( const std::string name )
:m_name( name )
{
	m_buffer_id = alutCreateBufferHelloWorld();
	ALuint error = alutGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create buffer named \"%s\"\n"
				"with hello world sound\n", name.c_str() );
	}
}

dtkSoundBuffer::dtkSoundBuffer( const std::string& filename, const std::string name )
:m_name( name )
{
	m_buffer_id = alutCreateBufferFromFile( filename.c_str() );
	ALuint error = alutGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create buffer named \"%s\"\n"
				"from filename %s\n", name.c_str(), filename.c_str() );
	}
}

dtkSoundBuffer::dtkSoundBuffer( const void* data, int length, const std::string name )
:m_name( name )
{
	m_buffer_id = alutCreateBufferFromFileImage( data, length );
	ALuint error = alutGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create buffer named \"%s\"\n"
				"from data %p and length %d \n", name.c_str(), data, length );
	}
}

dtkSoundBuffer::dtkSoundBuffer( const waveformEnum& waveshape, const float& frequency,
		const float& phase, const float& duration, const std::string name )
:m_name( name )
{
	m_buffer_id = alutCreateBufferWaveform( waveshape, frequency, phase, duration );
	ALuint error = alutGetError();
	if( error )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create buffer named \"%s\"\n"
				"from waveform with waveshape\n", name.c_str() );
	}
}

dtkSoundBuffer::~dtkSoundBuffer()
{
	alDeleteBuffers( 1, &m_buffer_id );
}

int dtkSoundBuffer::getFrequency()
{
	int frequency;
	alGetBufferi( m_buffer_id, AL_FREQUENCY, &frequency );

	ALenum error = alGetError();
	if( error )
	{
		return -1;
	}

	return frequency;
}

int dtkSoundBuffer::getDataSize()
{
	int data_size;
	alGetBufferi( m_buffer_id, AL_SIZE, &data_size );

	ALenum error = alGetError();
	if( error )
	{
		return -1;
	}

	return data_size;
}

int dtkSoundBuffer::getBits()
{
	int bits;
	alGetBufferi( m_buffer_id, AL_BITS, &bits );

	ALenum error = alGetError();
	if( error )
	{
		return -1;
	}

	return bits;
}

int dtkSoundBuffer::getChannels()
{
	int channels;
	alGetBufferi( m_buffer_id, AL_CHANNELS, &channels );

	ALenum error = alGetError();
	if( error )
	{
		return -1;
	}

	return channels;
}

bool dtkSoundBuffer::isValid()
{
	return alIsBuffer( m_buffer_id );
}

