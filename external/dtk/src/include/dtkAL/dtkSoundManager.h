#ifndef _DTK_SOUND_MANAGER_H_
#define _DTK_SOUND_MANAGER_H_

// The dtkSoundManager holds all related dtkSoundBuffer, dtkSoundSource,
// and dtkSoundSyncedSources objects. The objects cannot be deleted except through
// the dtkManager. All objects can be accessed through their names though each name
// for each type must be unique though the same name may exist for each of the
// different types. As such, it is not necessary to maintain lists of the pointers
// to the objects if not needed. This allows the dtkManager to clean up all of the
// sound objects when it is closed. It also makes it easier (hopefully) to reuse
// buffers across multiple sources without worrying about maintaining knowledge
// of the current status of each buffer or source. Or something of the sort...
class dtkSoundManager
{
public:
	// Initialize the dtkSoundManager
	static int init();

	// Deallocate the dtkSoundManager
	static int close();

	dtkSoundSource* playSound( const std::string& filename, bool loop = false );
	dtkSoundSource* playSounds( const std::vector<std::string>& filenames,
			bool loop = false );

	dtkSoundSource* playSoundAtPosition( const std::string& filename, bool loop = false );
	dtkSoundSource* playSoundsAtPosition( const std::vector<std::string>& filenames,
			bool loop = false );

	dtkSoundBuffer* createBufferHelloWorld( const std::string& name );
	dtkSoundBuffer* createBufferFromFile( const std::string& filename,
			const std::string& name );
	dtkSoundBuffer* createBufferFromFileImage( const void* data, int length,
			const std::string& name );
	dtkSoundBuffer* createBufferWaveform( const dtkSoundBuffer::waveformEnum& waveshape,
			const float& frequency, const float& phase, const float& duration,
			const std::string& name );

	dtkSoundBuffer* getBuffer( const std::string& name );

	bool destroyBuffer( const std::string& name );
	bool destroyBuffer( dtkSoundBuffer* buffer );

	dtkSoundSource* createSourceEmpty( const std::string& name );
	dtkSoundSource* createSourceFromBuffer( dtkSoundBuffer* buffer,
			const std::string& name );
	dtkSoundSource* createSourceFromBuffer( const std::string& buffer_name,
			const std::string& name );
	dtkSoundSource* createSourceFromBuffers( const std::vector<dtkSoundBuffer*>& buffers,
			const std::string& name );
	dtkSoundSource* createSourceFromBuffers( const std::vector<std::string>& buffer_names,
			const std::string& name );

	dtkSoundSource* getSource( const std::string& name );

	bool destroySource( const std::string& name );
	bool destroySource( dtkSoundSource* source );

	dtkSoundSyncedSources* createSyncedSources( const std::string& name ); 
	dtkSoundSyncedSources* createSyncedSources( const std::vector<dtkSoundSource*>& sources,
			const std::string& name );

	dtkSoundSyncedSources* getSyncedSources( const std::string& name );

	bool destroySyncedSources( const std::string& name );
	bool destroySyncedSources( dtkSoundSyncedSources* synced_sources );

	int setListenerPosition( const dtkVec3& position );
	dtkVec3 getListenerPosition();

	int setListenerVelocity( const dtkVec3& velocity );
	dtkVec3 getListenerVelocity();

	int setListenerGain( const float& gain );
	float getListenerGain();

	int setListenerOrientation( const dtkVec3 hpr );
	int setListenerOrientation( const dtkVec3 at, const dtkVec3 up );
	dtkVec3 getListenerOrientation();

	enum distanceModelEnum
	{
		NONE = AL_NONE,
		INVERSE_DISTANCE = AL_INVERSE_DISTANCE,
		INVERSE_DISTANCE_CLAMPED = AL_INVERSE_DISTANCE_CLAMPED,
		LINEAR_DISTANCE = AL_LINEAR_DISTANCE,
		LINEAR_DISTANCE_CLAMPED = AL_LINEAR_DISTANCE_CLAMPED,
		EXPONENT_DISTANCE = AL_EXPONENT_DISTANCE,
		EXPONENT_DISTANCE_CLAMPED = AL_EXPONENT_DISTANCE_CLAMPED
	};

	// Set the distance model for the context
	void setDistanceModel( const distanceModelEnum& distance_model );
	// Returns the distance model for the context
	distanceModelEnum getDistanceModel() { return m_distance_model; };
	// Returns a string representing the distance model enumeration passed in
	std::string getDistanceModel( const distanceModelEnum& distance_model );

	// Set the doppler factor
	void setDopplerFactor( const float& doppler_factor );
	// Return the doppler factor
	float getDopplerFactor() { return m_doppler_factor; };

	// Set the speed of sound (should be in diverse units)
	void setSpeedOfSound( const float& speed_of_sound );
	// Return the speed of sound
	float getSpeedOfSound() { return m_speed_of_sound; };

	// Returns a pointer to the dtkSoundManager or NULL if it has not been initialized
	static dtkSoundManager* getInstance();

	// returns the MIME types supported by openAL
	std::string getSupportedMIMETypes();

private:
	// Constructor
	dtkSoundManager();
	// Destructor
	virtual ~dtkSoundManager();

	// Possibly called by dtk, dgl, or dpf - up in the air
	// Would update the listener location for the context
	// in world coordinates for the head
	void updateListenerLocation();

	distanceModelEnum m_distance_model;
	float m_doppler_factor;
	float m_speed_of_sound;

	dtkVec3 m_listener_xyz;
	dtkVec3 m_listener_hpr;
	dtkVec3 m_listener_velocity;
	float m_listener_gain;

	std::map<std::string, dtkSoundBuffer*> m_buffers;
	std::map<std::string, dtkSoundSource*> m_sources;
	std::map<std::string, dtkSoundSyncedSources*> m_synced_sources;

	static bool m_is_inited;
	static dtkSoundManager* m_sound_manager;
};

#endif

