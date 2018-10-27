#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Networking/Test/UDPTest.hpp"
#include "Engine/Tools/DevConsole.hpp"

UDPTest g_udpTest;

bool UDPTest::Start()
{
	m_socket = new UDPSocket();
	bool success = m_socket->Bind( m_socket->m_netAddress, GAME_PORT );

	if ( !success )
	{
		LogErrorf( "UDPTest::Start(): Could not bind!" );
		return false;
	}
	else
	{
		LogPrintf( "UDPTest::Start(): Bound successfully." );
	}

	CommandRegister( "udp_send", UDPTestSendCommand, "Tests the UDP send functionality.", true );
	return true;
}

void UDPTest::Stop()
{
	CommandUnregister( "udp_send" );
	m_socket->Close();
	delete m_socket;
}

void UDPTest::SendTo( const NetAddressIPv4& addr, const void* buffer, unsigned int byteCount )
{
	m_socket->SendTo( addr, buffer, byteCount ); 
}

void UDPTest::Update()
{
	char buffer[ PACKET_MTU ];	// Since the max size of the infrastructure is unknown (hardware layer), we use a relatively small number

	NetAddressIPv4 fromAddr; 
	size_t read = m_socket->ReceiveFrom( &fromAddr, buffer, PACKET_MTU ); 

	if ( read > 0U )
	{
		unsigned int maxBytes = static_cast< unsigned int >( Min( static_cast< int >( read ), 128 ) );

		char* output = new char[ ( maxBytes * 2U ) + 3U ];
		output[ 0 ] = '0'; output[ 1 ] = 'x';

		char* iter = output; 
		iter += 2U; // Skip the 0x
		for ( unsigned int i = 0; i < read; i++ )
		{
			sprintf_s( iter, 3U, "%02X", buffer[ i ] ); 
			iter += 2U; 
		}
		*iter = NULL;

		LogPrintf( "UDPTest::Update(): Received from %s;\n%s", fromAddr.ToString().c_str(), output );

		delete[] output;
	}
}

bool UDPTestSendCommand( Command& command )
{
	if ( command.GetName() == "udp_send" )
	{
		std::string netAddrStr = command.GetNextString();
		if ( netAddrStr == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: udp_send: No address added." );
			return false;
		}
		NetAddressIPv4 addr;
		if ( !NetAddressIPv4::IsValid( netAddrStr.c_str() ) )
		{
			ConsolePrintf( Rgba::RED, "ERROR: udp_send: Invalid address added." );
			return false;
		}
		addr.FromString( netAddrStr );

		std::string text = command.GetNextString();
		if ( text == "" )
		{
			ConsolePrintf( Rgba::RED, "ERROR: udp_send: Invalid address added." );
			return false;
		}

		g_udpTest.SendTo( addr, text.c_str(), static_cast< unsigned int >( text.size() ) );

		return true;
	}
	else
	{
		return false;
	}
}
