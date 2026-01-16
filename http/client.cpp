
#include "http/http.h"
#include "tier1/interface.h"
#include "tier1/utlstring.h"
#include "netdb.h"
#include "unistd.h"
#include "fcntl.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "arpa/inet.h"

abstract_class CHTTPClient: public IHTTPClient
{
public:
	void ConnectToServer();
	void CloseConnection();

	virtual void Post( const char *szResource, HTTPHeader_t *pHeader, uint32_t uDataSize, const void *data ) override;
	virtual void Get( const char *szResource, HTTPHeader_t *pHeader ) override;
	virtual HTTPResponse_t GetResponse() override;

	virtual bool WebSocket_Connect( const char *szResource ) override;
	virtual void WebSocket_Close( void ) override;
	virtual void WebSocket_SendText( const char *szData ) override;
	virtual CUtlString WebSocket_RecvText() override;
	virtual void WebSocket_SendBinary( size_t uSize, const void *pData ) override;
	virtual WebSocketPacket_t WebSocket_RecvBinary() override;

	ssize_t Write( void *pData, ssize_t uSize );
	ssize_t Read( void *pData, ssize_t uSize );

	HTTPResponse_t ParseResponse( const char *szMessage, uint32_t uDataSize  );	
	const char *GetVersion();
	HTTPHeaderParam_t ParseHeaderParams( const char *szHeaderLine );

	const char *m_szHostName;
	uint16_t m_uPort;
	bool m_bIsSecure;
	SSL *m_pSSL;
	SSL_CTX *m_pSSLCtx;

	int m_iFileDescriptor;
};

void CHTTPClient::Post( const char *szResource, HTTPHeader_t *pHeader, uint32_t uDataSize, const void *data )
{
	if (pHeader == NULL)
		return;

	ConnectToServer();

	CUtlString szMessage = CUtlString(
			"POST %s HTTP/%s\r\n",
			szResource, GetVersion());
	CUtlString szHeader = CUtlString(
			"Host: %s\r\n"
			"Content-Length: %u\r\n",
			m_szHostName,
			uDataSize
			);

	CUtlString szCombined;
	int i = 0;
	
	for ( i = 0; i < pHeader->m_nParamCount; i++ )
	{
		szHeader.AppendTail(CUtlString("%s: %s\r\n", pHeader->m_params[i].m_szParamName.GetString(), pHeader->m_params[i].m_szValue.GetString()));
	}

	szCombined = szMessage;
	szCombined.AppendTail(szHeader);
	szCombined.AppendTail("\r\n");
	V_printf("%s\n",szCombined.GetString());

	Write(szCombined.GetString(), szCombined.GetLenght());
	Write((void*)data, uDataSize);
}

void CHTTPClient::Get( const char *szResource, HTTPHeader_t *pHeader )
{
	if (pHeader == NULL)
		return;

	ConnectToServer();

	CUtlString szMessage = CUtlString(
			"GET %s HTTP/%s\r\n",
			szResource, GetVersion());
	CUtlString szHeader = CUtlString(
			"Host: %s\r\n",
			m_szHostName
			);

	CUtlString szCombined;
	int i = 0;
	
	for ( i = 0; i < pHeader->m_nParamCount; i++ )
	{
		szHeader.AppendTail(CUtlString("%s: %s\r\n", pHeader->m_params[i].m_szParamName.GetString(), pHeader->m_params[i].m_szValue.GetString()));
	}

	szCombined = szMessage;
	szCombined.AppendTail(szHeader);
	szCombined.AppendTail("\r\n");

	Write(szCombined.GetString(), szCombined.GetLenght());
}

HTTPResponse_t CHTTPClient::GetResponse()
{
	CUtlResizableBuffer<char> szCharBuffer(0);
	char response[4096] = {};
	int n;
	int nPreviousSize = 0;

readSocket:

	n = Read(response, sizeof(response));
	V_printf("%s\n",response);
	if (n == -1)
		goto responseDone;


	szCharBuffer.Resize(nPreviousSize+n);
	V_memcpy((char*)szCharBuffer.GetMemory()+nPreviousSize, response, n);
	nPreviousSize += n;

	// HTTP 1.0 reacts either to socket being closed or Content-Lenght
	// HTTP 1.1 has Transfer-Encoding: chunked, which we need to respect
	// Still some response codes may not include a body
	if ( n > 0 )
	{
		HTTPResponse_t r = ParseResponse(szCharBuffer, szCharBuffer.GetSize());
		if (r.m_bIsComplete)
			goto responseDone;

		// these do not provide body 
	}
	// there is some data so go and read back again
	goto readSocket;
responseDone:

	return ParseResponse(szCharBuffer, szCharBuffer.GetSize());
}

HTTPResponse_t CHTTPClient::ParseResponse( const char *szMessage, uint32_t uDataSize )
{
	char cPreviousCharacter = 0;
	char cCurrentCharacter = 0;
	const char *pcCurrentCharacter = szMessage;
	CUtlString szBuffer = "";
	bool bIsMessage = true;
	HTTPResponse_t response = {};
	CUtlVector<HTTPHeaderParam_t> headers = {};
	CUtlBuffer<char> data = {};

	if (!szMessage)
		return {};
	// Parse header 
	while (*pcCurrentCharacter)
	{
		cCurrentCharacter = *pcCurrentCharacter;
		if ( cPreviousCharacter == '\r')
		{
			if ( cCurrentCharacter == '\n')
			{
				if (bIsMessage)
				{
					uint32_t uResult = 0;
					while (szBuffer[0] != ' ')
						szBuffer.RemoveHead(1);
					while (szBuffer[0] == ' ')
						szBuffer.RemoveHead(1);
					V_sscanf(szBuffer, "%i", &uResult);
					response.m_uCode = uResult;
				}
					if (szBuffer == "")
						break;
				if (!bIsMessage)
					headers.AppendTail(ParseHeaderParams(szBuffer));
				bIsMessage = false;
				szBuffer = "";
				cPreviousCharacter = 0;
				cCurrentCharacter = *pcCurrentCharacter++;
				continue;
			}
		}
		if (cPreviousCharacter != 0)
			szBuffer.AppendTail(cPreviousCharacter);
		pcCurrentCharacter++;

		cPreviousCharacter = cCurrentCharacter;
	};
	switch (response.m_uCode)
	{
	case 100:
	case 101:
	case 204:
	case 205:
	case 304:
		response.m_bIsComplete = true;
		return response;
	default:
		break;
	}

	bool bParseInChunks = false;
	uint64_t uBodySize = 0;
	// check content lenght
	for ( int i = 0; i < headers.GetSize(); i++ )
	{
		if ( !V_stricmp( headers[i].m_szParamName, "Content-Length" ) )
		{
			uBodySize = atoll(headers[i].m_szValue);
			bParseInChunks = false;
		};


		if ( !V_stricmp( headers[i].m_szParamName, "Transfer-Encoding" ) )
		{
			if ( !V_stricmp( headers[i].m_szValue, "Chunked" ) )
			{
				bParseInChunks = true; 
			};
		};
	}
	pcCurrentCharacter++;
	if ( !bParseInChunks )
	{
		uint32_t nDataLen = uDataSize-(pcCurrentCharacter-szMessage);
		if (nDataLen < uBodySize)
		{
			response.m_bIsComplete = false;
			return response;
		}
		data = CUtlBuffer<char>(nDataLen+1);
		V_memcpy(data.GetMemory(), pcCurrentCharacter, nDataLen);
		data[nDataLen] = 0;
		response.m_params = CUtlBuffer<HTTPHeaderParam_t>(headers.GetSize());
		for (int i = 0; i < headers.GetSize(); i++ )
		{
			response.m_params.operator[](i) = headers.operator[](i);
		}
		response.m_message = data;
		response.m_bIsComplete = true;

	} else {
		szBuffer = "";
		while (*pcCurrentCharacter)
		{

			cCurrentCharacter = *pcCurrentCharacter;
			if ( cPreviousCharacter == '\r')
			{
				if ( cCurrentCharacter == '\n')
				{
					uint32_t uChunkSize;
					uChunkSize = strtol(szBuffer, NULL, 0);
				}
			}
			if (cPreviousCharacter != 0)
				szBuffer.AppendTail(cPreviousCharacter);
			pcCurrentCharacter++;

			cPreviousCharacter = cCurrentCharacter;
		}
	}	
	
	return response;
}

const char *CHTTPClient::GetVersion()
{

	return "1.1";
}
HTTPHeaderParam_t CHTTPClient::ParseHeaderParams( const char *szHeaderLine )
{
	const char *psz = szHeaderLine;
	CUtlString szName;
	CUtlString szValue;

	int stage = 0;
	while (*psz)
	{
		if (stage == 0)
		{
			if (*psz == ':')
				stage = 1;
			else
				szName.AppendTail(*psz);
		} else if (stage == 1)
		{
			if (*psz != ' ')
				break;
		}
		psz++;
	}
	szValue = psz;
	return {szName, szValue};
}
void CHTTPClient::ConnectToServer()
{
	struct hostent *pServerHostName = NULL;
	struct sockaddr_in serverAddress;
	int err;
	SSL_CTX *ctx;
	SSL *ssl; 

	if (V_strcmp(m_szHostName, "localhost"))
	{
		pServerHostName = gethostbyname(m_szHostName);
		if (!pServerHostName)
			return;
	}


	V_memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	if (!V_strcmp(m_szHostName, "localhost"))
		inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
	else
		V_memcpy(&serverAddress.sin_addr.s_addr, pServerHostName->h_addr, pServerHostName->h_length);
	
	// https
	serverAddress.sin_port = htons(m_uPort);

	err = connect(m_iFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if (err < 0)
		return;

	if (m_bIsSecure)
	{
		ctx = SSL_CTX_new(TLS_client_method());
		if (!ctx)
			return;
		ssl =SSL_new(ctx);
		if (!ssl)
			return;

		SSL_set_fd(ssl, m_iFileDescriptor);
		SSL_set_tlsext_host_name(ssl, m_szHostName);

		int r = SSL_connect(ssl);
		if (r <= 0)
		{
			ERR_print_errors_fp(stdout);
			SSL_free(ssl);
			SSL_CTX_free(ctx);
			return;
		}
	};

	if (m_bIsSecure)
	{
		m_pSSL = ssl;
		m_pSSLCtx = ctx;
	}


}

void CHTTPClient::CloseConnection()
{

}
bool CHTTPClient::WebSocket_Connect( const char *szResource )
{
	HTTPResponse_t stResponse;
	HTTPHeaderParam_t params[] = {
		{"Upgrade", "websocket"},
		{"Connection", "Upgrade"},
		{"Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ=="},
		{"Sec-WebSocket-Protocol", "chat, superchat"},
		{"Sec-WebSocket-Version", "13"},
	};
	HTTPHeader_t stHeader = {
		sizeof(params)/sizeof(params[0]),
		params
	};
	Get(szResource, &stHeader);
	stResponse = GetResponse();
	// According to spec 101 is a good sign
	if (stResponse.m_uCode == 101)
		return true;
	return false;
}


void CHTTPClient::WebSocket_Close( void )
{

}

enum EWebSocketOp
{
	WEBSOCKET_CONTINUE = 0,
	WEBSOCKET_TEXT = 1,
	WEBSOCKET_BINARY = 2,
	WEBSOCKET_CLOSE = 8,
	WEBSOCKET_PING = 9,
	WEBSOCKET_PONG = 10,
};

struct WebSocketFrame_t
{
	EWebSocketOp nOpCode: 4;
	uint8_t nReserved: 3;
	uint8_t bFin: 1;
	uint8_t nPayloadLenght: 7;
	uint8_t bMasked: 1;
};

void CHTTPClient::WebSocket_SendText( const char *szData )
{
	size_t uLen;
	WebSocketFrame_t stFrame;
	uLen = V_strlen(szData);
	stFrame = (WebSocketFrame_t){
		.nOpCode = WEBSOCKET_TEXT,
		.bFin = 1,
		.bMasked = 1,
	};
	// im too lazy to mask it
	int iMask = 0;

	if ( uLen <= 125 )
	{
		stFrame.nPayloadLenght = uLen;
		Write(&stFrame, 2);
		Write(&iMask, 4);
	}
	else if ( uLen <= 65535 )
	{
		stFrame.nPayloadLenght = 126;
		Write(&stFrame, 2);
		uint16_t uLenN = htons(uLen);
		Write(&uLenN, 2);
		Write(&iMask, 4);
	}
	else
	{
		stFrame.nPayloadLenght = 127;
		Write(&stFrame, 2);
		Write(&iMask, 4);
		Write(&uLen, 8);
	}
	Write((void*)szData, uLen);

}

CUtlString CHTTPClient::WebSocket_RecvText()
{
	WebSocketFrame_t stFrame;
	size_t uLen = 0;
	CUtlBuffer<char> szText;
	Read(&stFrame, 2);
	if (stFrame.nPayloadLenght == 126)
	{
		Read(&uLen, 2);
	} else if (stFrame.nPayloadLenght == 127)
	{
		Read(&uLen, 8);
	} else {
		uLen = stFrame.nPayloadLenght;
	}
	szText = CUtlBuffer<char>(uLen+1);
	Read(szText, uLen);
	szText[uLen] = 0;
	return szText.GetMemory();
}

void CHTTPClient::WebSocket_SendBinary( size_t uSize, const void *pData )
{

}

WebSocketPacket_t CHTTPClient::WebSocket_RecvBinary()
{

}


ssize_t CHTTPClient::Write( void *pData, ssize_t uSize )
{
	if (m_bIsSecure)
		return SSL_write(m_pSSL, pData, uSize);
	else
		return write(m_iFileDescriptor, pData, uSize);
}

ssize_t CHTTPClient::Read( void *pData, ssize_t uSize )
{
	ssize_t n;
	if ( m_bIsSecure )
	{
		n = SSL_read(m_pSSL, pData, uSize);
		if (n == 0)
			return -1;
	}
	else 
	{
		n = read(m_iFileDescriptor, pData, uSize);
		if (n == -1)
			return -1;
	}
	return n;
}



abstract_class CHTTPClientManager: public IHTTPClientManager
{
public:
	CHTTPClientManager();
	virtual IHTTPClient *Connect( const char *szUrl, bool bSecure, uint16_t *pPort ) override;
	virtual void Disconnect( IHTTPClient *szConnection ) override;
};

IHTTPClient *CHTTPClientManager::Connect( const char *szUrl, bool bSecure, uint16_t *pPort )
{
	CHTTPClient *pClient;
	int fd = 0;

	pClient = new CHTTPClient();
	if (pPort)
		pClient->m_uPort = *pPort;
	else 
	{
		if (bSecure)
			pClient->m_uPort = 443;
		else
			pClient->m_uPort = 80;
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( fd < 0 )
		return NULL;

	pClient->m_iFileDescriptor = fd;

	pClient->m_szHostName = szUrl;
	pClient->m_bIsSecure = bSecure;
	return pClient;
}

void CHTTPClientManager::Disconnect( IHTTPClient *pClient )
{
	CHTTPClient *pC = (CHTTPClient*)pClient;
	close(pC->m_iFileDescriptor);
}

CHTTPClientManager::CHTTPClientManager()
{
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
}


CHTTPClientManager s_HttpClientManager;
EXPOSE_INTERFACE_GLOBALVAR(IHTTPClientManager, CHTTPClientManager, HTTP_CLIENT_INTERFACE_VERSION, s_HttpClientManager);

