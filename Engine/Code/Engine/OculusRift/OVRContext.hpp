#pragma once

// Link in the LIB file based on Target and Flavor
#if defined( _DEBUG )
	#if defined( _WIN64 )
	#pragma comment( lib, "ThirdParty/OculusSDK/Lib/x64/Debug/LibOVR.lib" )
	#else
	#pragma comment( lib, "ThirdParty/OculusSDK/Lib/x86/Debug/LibOVR.lib" )
	#endif
#else
	#if defined( _WIN64 )
	#pragma comment( lib, "ThirdParty/OculusSDK/Lib/x64/Release/LibOVR.lib" )
	#else
	#pragma comment( lib, "ThirdParty/OculusSDK/Lib/x86/Release/LibOVR.lib" )
	#endif
#endif

#include "ThirdParty/OculusSDK/Include/OVR_CAPI.h"

class OVRHeadset;
class OVRInput;

class OVRContext
{

private:
	OVRContext();
	~OVRContext();

private:
	void Initialize();

public:
	static void Startup();
	static void BeginFrame();
	static void EndFrame();
	static void Shutdown();

	static OVRContext* GetInstance();
	static OVRHeadset* GetHeadset();
	static OVRInput* GetInputSystem();
	static bool IsVREnabled();
	static long long GetFrameIndex();

private:
	ovrSession m_session;
	ovrGraphicsLuid m_luid;
	long long m_frameIndex = -1;

	OVRHeadset* m_hmd = nullptr;
	OVRInput* m_inputSystem = nullptr;

};
