#include "http/http.h"
#include "tier1/interface.h"

IHTTPClientManager *g_pHttpClientMgr;
int main()
{
	Sys_GetFactory("tier0");
	CreateInterfaceFn pHttpFactory = Sys_GetFactory("funnyhttp");
	g_pHttpClientMgr = (IHTTPClientManager*)pHttpFactory(HTTP_CLIENT_INTERFACE_VERSION, NULL);
	if ( !g_pHttpClientMgr )
		return 0;
	IHTTPClient *pClient = g_pHttpClientMgr->Connect("www.example.com", true, NULL);
	printf("%p\n",pClient);
	if ( pClient == NULL )
		return 0;
	HTTPHeaderParam_t params[] = {
		{"User-Agent", "Funny"},
		{"Accept", "application/json"},
	};

	HTTPHeader_t stHeader = {
		sizeof(params)/sizeof(params[0]),
		params
	};
	pClient->Get("/", &stHeader);

	HTTPResponse_t stResponse = pClient->GetResponse();
	if (stResponse.m_uCode == 200)
	{
		V_printf("%s\n", stResponse.m_message.GetMemory());
	}

	g_pHttpClientMgr->Disconnect(pClient);

	return 0;
}
