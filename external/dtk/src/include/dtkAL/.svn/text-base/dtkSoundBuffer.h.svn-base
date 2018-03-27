#ifndef _DTK_SOUND_BUFFER_H_
#define _DTK_SOUND_BUFFER_H_

// The dtkSoundBuffer wraps functionality for a buffer. Buffers store the data
// which is played by the source(s) it is attached to.

class dtkSoundBuffer
{
public:
	friend class dtkSoundManager;

	enum waveformEnum
	{
		WAVEFORM_SINE = ALUT_WAVEFORM_SINE,
		WAVEFORM_SQUARE = ALUT_WAVEFORM_SQUARE,
		WAVEFORM_SAWTOOTH = ALUT_WAVEFORM_SAWTOOTH,
		WAVEFORM_WHITENOISE = ALUT_WAVEFORM_WHITENOISE,
		WAVEFORM_IMPULSE = ALUT_WAVEFORM_IMPULSE
	};

	std::string getName() { return m_name; };

	int getFrequency();
	int getDataSize();
	int getBits();
	int getChannels();

	ALuint getBufferID() { return m_buffer_id; };

	bool isValid();

private:
	// Constructor
	// Creates a buffer with a 'Hello, World!' sound
	dtkSoundBuffer( const std::string name );
	// Constructor
	// Create a buffer from the file
	dtkSoundBuffer( const std::string& filename, const std::string name );
	// Constructor
	// Creates a buffer from in-memory sound data
	dtkSoundBuffer( const void* data, int length, const std::string name );
	// Constructor
	// Create a buffer with a synthesized waveform
	dtkSoundBuffer( const waveformEnum& waveshape, const float& frequency,
			const float& phase, const float& duration, const std::string name );
	// Destructor
	virtual ~dtkSoundBuffer();

	std::string m_name;
	ALuint m_buffer_id;
};

#endif

