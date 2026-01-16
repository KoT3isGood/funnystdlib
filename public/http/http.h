#ifndef HTTP_H
#define HTTP_H

#include "tier0/platform.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"

struct HTTPHeaderParam_t
{
	CUtlString m_szParamName;
	CUtlString m_szValue;
};

struct HTTPHeader_t
{
	uint32_t m_nParamCount;
	HTTPHeaderParam_t *m_params;
};

struct HTTPResponse_t
{
	CUtlBuffer<HTTPHeaderParam_t> m_params;
	CUtlBuffer<char> m_message;
	uint32_t m_uCode;
	bool m_bIsComplete;
};

struct WebSocketPacket_t
{
	size_t m_uSize;
	void *m_pData;
};

abstract_class IHTTPClient
{
public:
	virtual void Post( const char *szResource, HTTPHeader_t *pHeader, uint32_t uDataSize, const void *data ) = 0;
	virtual void Get( const char *szResource, HTTPHeader_t *pHeader ) = 0;

	virtual bool WebSocket_Connect( const char *szResource ) = 0;
	virtual void WebSocket_Close( void ) = 0;
	virtual void WebSocket_SendText( const char *szData ) = 0;
	virtual CUtlString WebSocket_RecvText() = 0;
	virtual void WebSocket_SendBinary( size_t uSize, const void *pData ) = 0;
	virtual WebSocketPacket_t WebSocket_RecvBinary() = 0;

	virtual HTTPResponse_t GetResponse() = 0;
};

abstract_class IHTTPClientManager
{
public:
	virtual IHTTPClient *Connect( const char *szUrl, bool bSecure, uint16_t *pPort ) = 0;
	virtual void Disconnect( IHTTPClient *pClient ) = 0;
};

#define HTTP_CLIENT_INTERFACE_VERSION "FHTTPClientMgr001"
extern IHTTPClientManager *g_pHttpClientMgr;


#endif
