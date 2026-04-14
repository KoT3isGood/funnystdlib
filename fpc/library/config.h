#ifndef COMPILER_CONFIG_H
#define COMPILER_CONFIG_H


#include "helper.h"

struct CompilerProperty_t
{
	const char *szName;
	const char *m_szDescription;
	CUtlString szValue;
	Target_t target;
};

enum EConfigurationType
{
	k_EConfig_Undefined,
	k_EConfig_Language,
	k_EConfig_DefaultValue,
};

class CCompilerValueRegistry
{
public:
	CCompilerValueRegistry( EConfigurationType t, const char *szCompiler, const char *szV1 );
	CCompilerValueRegistry( EConfigurationType t, const char *szCompiler, const char *szName, const char *szValue, const char *szDescription );
	CCompilerValueRegistry( EConfigurationType t, Target_t target, const char *szCompiler, const char *szName, const char *szValue );
};
#define COMPILER_LANGUAGE(compiler, id, language) \
static CCompilerValueRegistry __s_##compiler##id( k_EConfig_Language, #compiler, language );

#define COMPILER_VALUE(compiler, name, value, desc) \
static CCompilerValueRegistry __s_##compiler##name( k_EConfig_DefaultValue, #compiler, #name, value, desc );

class IConfigManager
{
public:
	virtual void Init( IINIFile *cfg ) = 0;
	virtual void OverrideProperty( const char *szCompiler, Target_t target, const char *szName, const char *szValue, const char *szDescription ) = 0;
	virtual void QueryCompilerRegistries() = 0;
	virtual void QueryCompilerValues( const char *szName ) = 0;
	virtual CUtlString GetProperty( const char *szCompiler, const char *szName, Target_t target) = 0;
};


#define CONFIG_MANAGER_INTERFACE_VERSION "ConfigManager001"

#endif
