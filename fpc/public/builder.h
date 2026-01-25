#ifndef BUILDER_H
#define BUILDER_H

#include "tier1/interface.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"

struct BuildOutput_t
{
	const char *m_szName;
};

struct BuildOutputs_t
{
	const char *m_szBuildStageName;
	CUtlVector<BuildOutput_t> m_buildOutputs;
};

struct BuildFile_t
{
	CUtlString m_szOutputFile;
	void *m_pLibrary;
	CUtlVector<BuildOutputs_t> m_compiledProjects;

	struct BuildFile_t *m_pNext;
};

abstract_class IProjectBuilder
{
public:
	virtual BuildFile_t *BuildProject( const char *szProjectName, const char *szPath ) = 0;
};

IProjectBuilder *ProjectBuilder();

#define PROJECT_BUILDER_INTERFACE_NAME "ProjectBuilder001"

#endif
