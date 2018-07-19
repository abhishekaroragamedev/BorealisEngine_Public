#include "Engine/Core/ThreadSafeTypes.hpp"

void SpinLock::Enter()
{
	m_mutex.lock();
}

bool SpinLock::TryEnter()
{
	return m_mutex.try_lock();
}

void SpinLock::Leave()
{
	m_mutex.unlock();
}
