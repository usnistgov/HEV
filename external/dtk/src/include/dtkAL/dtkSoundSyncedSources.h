#ifndef _DTK_SOUND_SYNCED_SOURCES_H_
#define _DTK_SOUND_SYNCED_SOURCES_H_

// The dtkSoundSyncedSources conatin sources which you wish to have played synchronously.
// Unlike a queued source, they are played at the same time and each source is separate
// (i.e. they have different positions, velocities, gain, pitch, etc).
class dtkSoundSyncedSources
{
public:
	friend class dtkSoundManager;

	// Return the name of the dtkSoundSource
	std::string getName() { return m_name; };

	void addSource( const dtkSoundSource* source );
	void addSources( const std::vector<dtkSoundSource*>& sources );

	dtkSoundSource* getSource( const std::string& name );
	std::vector<dtkSoundSource*> getSources( const std::vector<std::string> names );
	std::vector<dtkSoundSource*> getAllSources();

	void removeSource( const dtkSoundSource* source );
	void removeSources( const std::vector<dtkSoundSource*>& sources );
	void removeSource( const std::string& name );
	void removeSources( const std::vector<std::string>& namees );
	void removeAllSources();

	int play();
	int pause();
	int stop();
	int rewind();

private:
	dtkSoundSyncedSources( const std::string& name );
	dtkSoundSyncedSources( const std::vector<dtkSoundSource*>& sources,
			const std::string& name );
	virtual ~dtkSoundSyncedSources();

	std::string m_name;
	ALuint* m_source_ids;
	std::map<std::string, dtkSoundSource*> m_sources;
};

#endif

