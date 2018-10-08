#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/NetConnection.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetPacket.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/UDPSocket.hpp"

NetConnection::NetConnection( const NetAddressIPv4& addr, NetSession* session, unsigned int connectionIndex )
	:	m_address( addr ),
		m_session( session )
{

}

NetConnection::NetConnection( const NetConnection& copy )
{
	m_session = copy.m_session;
	m_address = NetAddressIPv4( copy.m_address );
	m_connectionIndex = copy.m_connectionIndex;
}

void NetConnection::Send( const NetMessage& message )
{
	NetMessage* newMessage = new NetMessage( message );
	m_outboundMessages.push_back( newMessage );
}

void NetConnection::ProcessOutgoing()
{
	if ( m_outboundMessages.size() > 0U )
	{
		NetPacket* packet = new NetPacket( static_cast< uint8_t >( m_connectionIndex ) );

		for ( size_t messageIndex = 0U; messageIndex < m_outboundMessages.size(); messageIndex++ )
		{
			NetMessage* message = m_outboundMessages[ messageIndex ];
			bool success = packet->WriteMessage( *message );
			if ( success )
			{
				delete message;
				m_outboundMessages.erase( m_outboundMessages.begin() + messageIndex );
				messageIndex--;
			}
			else
			{
				break;
			}
		}

		m_session->GetSocket()->SendTo( m_address, packet->m_buffer, packet->GetWrittenByteCount() );
		delete packet;
	}
}
