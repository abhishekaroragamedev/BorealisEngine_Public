#pragma once

template< typename T >
struct LinkedList
{

	T* GetLast() const
	{
		T* ptr = this;
		while ( ptr->m_next != nullptr )
		{
			ptr = ptr->m_next;
		}
		return ptr;
	}
	bool HasNext() const
	{
		return ( m_next != nullptr );
	}

public:
	T* m_next = nullptr;

};
