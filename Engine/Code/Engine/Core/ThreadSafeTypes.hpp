#pragma once

#include <deque>
#include <mutex>
#include <set>
#include <vector>

class SpinLock
{
public:
	void Enter();
	bool TryEnter();
	void Leave();

public:
	std::mutex m_mutex;

};

template< typename T>
class ThreadSafeQueue
{
public:
	void Enqueue( const T& item )
	{
		m_lock.Enter();
		m_data.push_back( item );
		m_lock.Leave();
	}

	bool TryEnqueue( const T& item )
	{
		bool success = m_lock.TryEnter();
		if ( !success )
		{
			return false;
		}

		m_data.push_back( item );
		m_lock.Leave();

		return success;
	}

	bool Dequeue( T* out_item )
	{
		bool success = false;

		m_lock.Enter();
		if ( !m_data.empty() )
		{
			*out_item = static_cast< T >( m_data.front() );
			m_data.pop_front();
			success = true;
		}
		m_lock.Leave();

		return success;
	}

	bool TryDequeue( T* out_item )
	{
		bool success = false;
		success = m_lock.TryEnter();
		if ( !success )
		{
			return false;
		}

		if ( !m_data.empty() )
		{
			*out_item = static_cast< T >( m_data.front() );
			m_data.pop_front();
			success = true;
		}
		m_lock.Leave();

		return success;
	}

public:
	std::deque< T > m_data;
	SpinLock m_lock;

};

template< typename T>
class ThreadSafeSet
{
public:
	bool Contains( T item )
	{
		bool found = false;

		m_lock.Enter();
		if ( m_data.find( item ) != m_data.end() )
		{
			found = true;
		}
		m_lock.Leave();

		return found;
	}

	size_t Size()
	{
		m_lock.Enter();
		m_lock.Leave();
		return m_data.size();
	}

	void Insert( const T& item )
	{
		m_lock.Enter();
		m_data.insert( item );
		m_lock.Leave();
	}

	bool TryInsert( const T& item )
	{
		bool success = m_lock.TryEnter();
		if ( !success )
		{
			return false;
		}

		m_data.insert( item );
		m_lock.Leave();

		return success;
	}

	bool Erase( T& item )
	{
		bool success = false;

		m_lock.Enter();
		m_data.erase( item );
		m_lock.Leave();

		return success;
	}

	void Clear()
	{
		m_lock.Enter();
		m_data.clear();
		m_lock.Leave();
	}

public:
	std::set< T > m_data;
	SpinLock m_lock;

};

template< typename T>
class ThreadSafeVector
{
public:
	bool Get( size_t index, T* out_item )
	{
		bool success = false;

		m_lock.Enter();
		if ( m_data.size() > index )
		{
			*out_item = m_data[ index ];
			success = true;
		}
		m_lock.Leave();

		return success;
	}

	size_t Size()
	{
		m_lock.Enter();
		m_lock.Leave();
		return m_data.size();
	}

	void Push( const T& item )
	{
		m_lock.Enter();
		m_data.push_back( item );
		m_lock.Leave();
	}

	bool TryPush( const T& item )
	{
		bool success = m_lock.TryEnter();
		if ( !success )
		{
			return false;
		}

		m_data.push_back( item );
		m_lock.Leave();

		return success;
	}

	bool Erase( T& item )
	{
		bool success = false;

		m_lock.Enter();
		for ( size_t index = 0; index < m_data.size(); index++ )
		{
			if ( &m_data[ index ] = &item )
			{
				m_data.erase( m_data.begin() + index );
				success = true;
				break;
			}
		}
		m_lock.Leave();

		return success;
	}

public:
	std::vector< T > m_data;
	SpinLock m_lock;

};
