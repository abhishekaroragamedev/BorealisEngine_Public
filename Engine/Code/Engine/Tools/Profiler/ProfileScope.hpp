#pragma once

#define PROFILE_SCOPE( tag ) ProfileScope __timer_##__LINE__## ( tag )
#define PROFILE_SCOPE_FUNCTION( tag ) ProfileScope __FUNCTION__##__LINE__##( tag )

class ProfileScope
{

	friend class Profiler;

public:
	ProfileScope( const char* tag );
	~ProfileScope();

public:
	const char* m_tag;

};

#define PROFILE_LOG_SCOPE( tag ) ProfileLogScope __timer_##__LINE__## ( tag )
#define PROFILE_LOG_SCOPE_FUNCTION( tag ) ProfileLogScope __FUNCTION__##__LINE__##( tag )

class ProfileLogScope
{

public:
	ProfileLogScope( const char* tag );
	~ProfileLogScope();

public:
	const char* m_tag;
	uint64_t m_startHPC = 0;

};
