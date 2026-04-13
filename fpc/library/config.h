#ifndef COMPILER_CONFIG_H
#define COMPILER_CONFIG_H


#include "helper.h"

struct CompilerProperty_t
{
	const char *szName;
	CUtlString szValue;	
};

class CCompilerValueRegistry
{
public:
	CCompilerValueRegistry( const char *szCompiler, const char *szName, const char *szValue );
};
#define COMPILER_VALUE(compiler, name, value) \
static CCompilerValueRegistry __s_##compiler##name( #compiler, #name, value);

class CConfigManager
{
public:
	void Init( IINIFile *cfg );
	void QueryCompilerRegistries();
	void QueryCompilerValues( const char *szName );
};


#define CONFIG_MANAGER_INTERFACE_VERSION "ConfigManager001"

#endif
