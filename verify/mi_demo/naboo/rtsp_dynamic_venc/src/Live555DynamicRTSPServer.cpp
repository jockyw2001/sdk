#include "Live555DynamicRTSPServer.hh"

Live555DynamicRTSPServer* Live555DynamicRTSPServer::createNew(
									UsageEnvironment& env, Port ourPort
									, UserAuthenticationDatabase* authDatabase
									, unsigned reclamationTestSeconds) 
{
	int ourSocket = setUpOurSocket(env, ourPort);
	if ( ourSocket == -1 ) {
		return NULL;
	}
	return new Live555DynamicRTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

Live555DynamicRTSPServer::Live555DynamicRTSPServer(UsageEnvironment& env
									, int ourSocket, Port ourPort
									, UserAuthenticationDatabase* authDatabase
									, unsigned reclamationTestSeconds)
	: RTSPServerSupportingHTTPStreaming(env, ourSocket, ourPort, authDatabase
			, reclamationTestSeconds)
	, m_CreateNewSMSFun(NULL)
{

}

Live555DynamicRTSPServer::~Live555DynamicRTSPServer() 
{
	m_CreateNewSMSFun = NULL;
}

int Live555DynamicRTSPServer::SetCreateNewSMSFun(CREATENewSMS CreateNewSMSFun)
{
	if ( CreateNewSMSFun==NULL ) {
		return -1;
	}
	m_CreateNewSMSFun = CreateNewSMSFun;
	return 0;
}

ServerMediaSession* Live555DynamicRTSPServer::lookupServerMediaSession(
						char const* streamName, Boolean isFirstLookupInSession)
{
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
	if ( sms && isFirstLookupInSession ) {
		removeServerMediaSession(sms); 
		sms = NULL;
	} 
	if ( sms == NULL ) {
		if ( m_CreateNewSMSFun ) {
			sms = m_CreateNewSMSFun(envir(), streamName); 
			if ( sms ) {
				addServerMediaSession(sms);
			}
		}
	}
	return sms;
}

