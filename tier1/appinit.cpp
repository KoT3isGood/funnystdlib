#include "tier1/appinit.h"
#include "tier1/utlstring.h"
#include "tier0/platform.h"

void AppInitializePath()
{
	const char *szPath = Plat_GetExecutablePath();
	CUtlString szEnv = Plat_GetEnv("PATH");
	printf("%s\n",szEnv.GetString());
	szEnv.AppendTail(":");
	szEnv.AppendTail(szPath);
	Plat_SetEnv("PATH", szEnv);
}
