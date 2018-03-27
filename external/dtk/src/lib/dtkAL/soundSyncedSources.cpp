#include <al.h>
#include <alut.h>
#include <string>
#include <vector>
#include <map>
#include "_config.h"
#include "config.h"
#include "dtkConfigure.h"
#include "dtkVec3.h"
#include "dtkSoundBuffer.h"
#include "dtkSoundSource.h"
#include "dtkSoundSyncedSources.h"

dtkSoundSyncedSources::dtkSoundSyncedSources( const std::string& name )
:m_name( name )
{
	m_source_ids = NULL;
}

dtkSoundSyncedSources::dtkSoundSyncedSources( const std::vector<dtkSoundSource*>& sources,
		const std::string& name )
:m_name( name )
{
	m_source_ids = NULL;

	if( sources.size() )
	{
		m_source_ids = new ALuint[sources.size()];
	}

	for( unsigned int i=0;i<sources.size();i++ )
	{
		m_source_ids[i] = sources[i]->getSourceID();
		
	}
}

dtkSoundSyncedSources::~dtkSoundSyncedSources()
{
}

void dtkSoundSyncedSources::addSource( const dtkSoundSource* source )
{
	if( source && m_source_ids )
	{
		
	}
}

void dtkSoundSyncedSources::addSources( const std::vector<dtkSoundSource*>& sources )
{
}

dtkSoundSource* dtkSoundSyncedSources::getSource( const std::string& name )
{
	return NULL;
}

std::vector<dtkSoundSource*> dtkSoundSyncedSources::getSources( const std::vector<std::string> names )
{
	std::vector<dtkSoundSource*> sources;

	return sources;
}

std::vector<dtkSoundSource*> dtkSoundSyncedSources::getAllSources()
{
	std::vector<dtkSoundSource*> sources;

	return sources;
}

void dtkSoundSyncedSources::removeSource( const dtkSoundSource* source )
{
}

void dtkSoundSyncedSources::removeSources( const std::vector<dtkSoundSource*>& sources )
{
}

void dtkSoundSyncedSources::removeSource( const std::string& name )
{
}

void dtkSoundSyncedSources::removeSources( const std::vector<std::string>& namees )
{
}

void dtkSoundSyncedSources::removeAllSources()
{
}

int dtkSoundSyncedSources::play()
{
	return 0;
}

int dtkSoundSyncedSources::pause()
{
	return 0;
}

int dtkSoundSyncedSources::stop()
{
	return 0;
}

int dtkSoundSyncedSources::rewind()
{
	return 0;
}

