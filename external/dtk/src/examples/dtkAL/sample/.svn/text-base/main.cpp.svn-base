#include <dtk.h>
#include <dtkAL.h>
#include <vector>

int main( int argc, char** argv )
{
	dtkSoundManager::init();

printf( "%s\n", dtkSoundManager::getInstance()->getSupportedMIMETypes().c_str() );

	std::vector<dtkSoundBuffer*> buffers;
	dtkSoundBuffer* buffer;
	std::vector<dtkSoundSource*> sources;

	buffer = dtkSoundManager::getInstance()->createBufferFromFile( "chant-1.0.wav", "chant" );
	if( !buffer )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create buffer from file\n" );
		return -1;
	}

	dtkSoundSource* source_static = dtkSoundManager::getInstance()->createSourceFromBuffer( "chant", "static" );

	buffer = dtkSoundManager::getInstance()->createBufferHelloWorld( "hello" );
	if( !buffer )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create hello world buffer\n" );
		return -1;
	}
	buffers.push_back( buffer );

	buffer = dtkSoundManager::getInstance()->createBufferFromFile( "horse.wav", "file" );
	if( !buffer )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create buffer from file\n" );
		return -1;
	}
	buffers.push_back( buffer );

	buffer = dtkSoundManager::getInstance()->createBufferWaveform( dtkSoundBuffer::WAVEFORM_SINE, 1000, 180, 1, "waveform" );
	if( !buffer )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create buffer waveform\n" );
		return -1;
	}
	buffers.push_back( buffer );

	dtkSoundSource* source_queue = dtkSoundManager::getInstance()->createSourceFromBuffers( buffers, "source" );
	if( !source_queue )
	{
		dtkMsg.add( DTKMSG_ERROR, "Unable to create source from buffer\n" );
		return -1;
	}

	dtkVec3 source_static_xyz( 0.0f, 2.0f, 2.0f );
	source_static->setPosition( source_static_xyz );
	source_static->play();

	bool source_queue_played = false;
	float time = 0.0f;
	while( source_static->isPlaying() || source_queue->isPlaying() )
	{
		if( time > 2.0f && !source_queue_played )
		{
			source_queue->play();
			source_queue_played = true;
		}

		source_static_xyz.d[0] = 2.0f * sin( time );
		source_static_xyz.d[1] = 2.0f * cos( time );
		source_static_xyz.d[2] = 2.0f * cos( time * 2.0f );
		source_static->setPosition( source_static_xyz );

		usleep( 100 );
		time += 0.0001f;
	}

	dtkSoundManager::close();
}

