#include "target.h"
#include "config.h"

static struct CompilerConfig_t
{
	const char *m_szName;
	CUtlVector<CompilerProperty_t> m_properties;
	struct CompilerConfig_t *m_pNext;
} *s_pCompilerConfigs;

CCompilerValueRegistry::CCompilerValueRegistry( const char *szCompiler, const char *szName, const char *szValue )
{
	CompilerConfig_t *pConfig = s_pCompilerConfigs;
	while (pConfig)
	{
		if (!V_strcmp(pConfig->m_szName, szCompiler))
		{
			pConfig->m_properties.AppendTail(((CompilerProperty_t){szName, szValue}));
		}
		pConfig = pConfig->m_pNext;
	}
	pConfig = new CompilerConfig_t;
	pConfig->m_szName = szCompiler;
	pConfig->m_properties = {(CompilerProperty_t){szName, szValue}};
	pConfig->m_pNext = s_pCompilerConfigs;
	s_pCompilerConfigs = pConfig;
}

void QueryCompilerRegistries()
{

	CompilerConfig_t *pConfig = s_pCompilerConfigs;
	V_printf("%p\n", pConfig);
	while (pConfig)
	{
		V_printf("Compiler: %s\n", pConfig->m_pNext);
		pConfig = pConfig->m_pNext;
	}
}

void QueryCompilerValues( const char *szName )
{

}

static CConfigManager s_configmgr;
EXPOSE_INTERFACE_GLOBALVAR(CConfigManager, CConfigManager, CONFIG_MANAGER_INTERFACE_VERSION, s_configmgr)
