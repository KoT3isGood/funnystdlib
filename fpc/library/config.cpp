#include "target.h"
#include "config.h"

static struct CompilerConfig_t
{
	const char *m_szName;
	CUtlVector<CUtlString> m_szLanguages;
	CUtlVector<CompilerProperty_t> m_properties;
	CUtlVector<CompilerProperty_t> m_propertyoverrides;
	struct CompilerConfig_t *m_pNext;
} *s_pCompilerConfigs;

CCompilerValueRegistry::CCompilerValueRegistry( EConfigurationType t, const char *szCompiler, const char *szT )
{

	CompilerConfig_t *pConfig = s_pCompilerConfigs;
	while (pConfig)
	{
		if (!V_strcmp(pConfig->m_szName, szCompiler))
		{
			goto setvalues;
		}
		pConfig = pConfig->m_pNext;
	}
	pConfig = new CompilerConfig_t;

	*pConfig = {};
	pConfig->m_pNext = s_pCompilerConfigs;
	s_pCompilerConfigs = pConfig;
	pConfig->m_szName = szCompiler;

setvalues:
	switch (t)
	{
	case k_EConfig_Language:
		pConfig->m_szLanguages.AppendTail(szT);
		break;
	default:
		break;
	}
}

CCompilerValueRegistry::CCompilerValueRegistry( EConfigurationType t, const char *szCompiler, const char *szName, const char *szValue, const char *szDescription )
{
	CompilerConfig_t *pConfig = s_pCompilerConfigs;
	while (pConfig)
	{
		if (!V_strcmp(pConfig->m_szName, szCompiler))
		{
			goto setvalues;
		}
		pConfig = pConfig->m_pNext;
	}
	pConfig = new CompilerConfig_t;
	*pConfig = {};
	pConfig->m_szName = szCompiler;
	pConfig->m_pNext = s_pCompilerConfigs;
	s_pCompilerConfigs = pConfig;
setvalues:
	pConfig->m_properties.AppendTail(((CompilerProperty_t){szName, szDescription, szValue}));
}

CCompilerValueRegistry::CCompilerValueRegistry( EConfigurationType t, Target_t target, const char *szCompiler, const char *szName, const char *szValue )
{

}

class CConfigManager: public IConfigManager
{
public:
	virtual void Init( IINIFile *cfg ) override;
	virtual void OverrideProperty( const char *szCompiler, Target_t target, const char *szName, const char *szValue, const char *szDescription ) override;
	virtual void QueryCompilerRegistries() override;
	virtual void QueryCompilerValues( const char *szName ) override;
	virtual CUtlString GetProperty( const char *szCompiler, const char *szName, Target_t target) override {};
};

void CConfigManager::Init( IINIFile *cfg )
{
	
};

void CConfigManager::OverrideProperty( const char *szCompiler, Target_t target, const char *szName, const char *szValue, const char *szDescription )
{
	CompilerConfig_t *pConfig = s_pCompilerConfigs;
	while (pConfig)
	{
		if (!V_strcmp(pConfig->m_szName, szCompiler))
		{
			goto setvalues;
		}
		pConfig = pConfig->m_pNext;
	}
	pConfig = new CompilerConfig_t;
	*pConfig = {};
	pConfig->m_szName = szCompiler;
	pConfig->m_pNext = s_pCompilerConfigs;
	s_pCompilerConfigs = pConfig;
setvalues:
	pConfig->m_propertyoverrides.AppendTail(((CompilerProperty_t){szName, szDescription}));
}

void CConfigManager::QueryCompilerRegistries()
{
	CompilerConfig_t *pConfig = s_pCompilerConfigs;
	while (pConfig)
	{
		V_printf("%s\n", pConfig->m_szName);
		V_printf("\tLanguages:");
		for ( auto l: pConfig->m_szLanguages )
		{
			V_printf(" %s", l.GetString());
		}
		V_printf("\n");
		pConfig = pConfig->m_pNext;
	}
}

void CConfigManager::QueryCompilerValues( const char *szName )
{

	CompilerConfig_t *pConfig = s_pCompilerConfigs;
	while (pConfig)
	{
		if(!V_strcmp(szName, pConfig->m_szName))
		{
			for ( auto p: pConfig->m_properties )
			{
				V_printf("%s = %s\n\t%s\n", p.szName, p.szValue.GetString(), p.m_szDescription);
			}


		}
		pConfig = pConfig->m_pNext;
	}
}

static CConfigManager s_configmgr;
EXPOSE_INTERFACE_GLOBALVAR(CConfigManager, CConfigManager, CONFIG_MANAGER_INTERFACE_VERSION, s_configmgr)
