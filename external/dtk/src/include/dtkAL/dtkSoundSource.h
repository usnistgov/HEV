#ifndef _DTK_SOUND_SOURCE_H_
#define _DTK_SOUND_SOURCE_H_

// The dtkSoundSource may contain a single static buffer or one or more queued buffers.
// All buffers in a queue must have the same format and attributes, with the exception of the
// NULL buffer (i.e., 0) which can always be queued.

class dtkSoundSource
{
public:
	friend class dtkSoundManager;

	// Return the name of the dtkSoundSource
	std::string getName() { return m_name; };

	// Specify whether the source is queued or not - default is false
	// If NOT queued and then set queued and a buffer already exists, then it will
	// become the first buffer in the queue
	// If queued and then set false, the entire queue will be stopped and removed
	void setQueued( bool queued );
	// return queued state
	bool isQueued() { return m_is_queued; };

	// If queued will add the buffer to the queue
	// If NOT queued will replace the current buffer
	int addBuffer( dtkSoundBuffer* buffer );
	// If queued will add the buffers to the queue
	// If NOT queued will return error -1
	int addBuffers( std::vector<dtkSoundBuffer*> buffers );
	// Removes the specified buffer if it exists
	int removeBuffer( const std::string& name );
	// Removes all buffers from the source
	int removeAllBuffers();
	// returns the specified buffer if it exists
	dtkSoundBuffer* getBuffer( const std::string& name );
	// returns the current buffer if it exists
	dtkSoundBuffer* getCurrentBuffer();

	// Specify that the position, velocity, cone, and direction properties
	// are relative to the listener
	void setSourceRelativeToListener( bool relative );
	// Return true if source is relative to listener, otherwise return false
	bool isSourceRelativeToListener();

	// Specify that the source should loop if true
	void setLooping( bool is_looping );
	// Return true if set for lopping, otherwise returns false
	bool isLooping();

	// Sets the position of the source
	void setPosition( const dtkVec3& xyz );
	// Returns position
	dtkVec3 getPosition();

	// Set the velocity of the source
	// only the translational velocities are used in openal.
	void setVelocity( const dtkVec3& velocity );
	// Return  the velocity
	dtkVec3 getVelocity();

	void setGain( const float& gain );
	float getGain();

	void setMinGain( const float& min_gain );
	float getMinGain();

	void setMaxGain( const float& max_gain );
	float getMaxGain();

	void setReferenceDistance( const float& reference_distance );
	float getReferenceDistance();

	void setRolloffFactor( const float& rolloff_factor );
	float getRolloffFactor();

	void setMaxDistance( const float& max_distance );
	float getMaxDistance();

	void setPitch( const float& pitch );
	float getPitch();

	void setDirection( const dtkVec3& direction );
	dtkVec3 getDirection();

	void setConeInnerAngle( const float& inner_angle );
	float getInnerConeAngle();

	void setConeOuterAngle( const float& outer_angle );
	float getOuterConeAngle();

	void setConeOuterGain( const float& outer_gain );
	float getConeOuterGain();
 
	void setSecondsOffset( const float& offset );
	float getSecondsOffset();

	void setSamplesOffset( const float& offset );
	float getSamplesOffset();

	void setBytesOffset( const int& offset );
	int getBytesOffset();

	int play();
	int pause();
	int stop();
	int rewind();

	bool isPlaying();
	bool isPaused();
	bool isStopped();
	bool isRewound();

	// returns AL_NONE, AL_INITIAL, AL_PLAYING, AL_PAUSED, AL_STOPPED
	int getCurrentState();

	ALuint getSourceID() { return m_source_id; };

	bool isValid();

private:
	// Constructor
	dtkSoundSource( const std::string& name );
	// Constructor
	dtkSoundSource( dtkSoundBuffer* buffer, const std::string& name );
	// Constructor
	dtkSoundSource( const std::vector<dtkSoundBuffer*> buffers, const std::string& name );
	// Destrcutor
	virtual ~dtkSoundSource();

	std::string m_name;
	ALuint m_source_id;

	std::map<std::string, dtkSoundBuffer*> m_buffers;

	bool m_is_queued;
	int m_is_source_relative_to_listener;
	bool m_is_looping;
	dtkVec3 m_position;
	dtkVec3 m_velocity;
	float m_gain;
	float m_min_gain;
	float m_max_gain;
	float m_reference_distance;
	float m_rolloff_factor;
	float m_max_distance;
	float m_pitch;
	dtkVec3 m_direction;
	float m_inner_angle;
	float m_outer_angle;
	float m_outer_gain;
};

#endif

