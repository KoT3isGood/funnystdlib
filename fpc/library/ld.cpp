#include "ld.h"

ILinker *linker;

void LinkProject_t::AddObject( Object_t object )
{
	objects.AppendTail(object);
};
CUtlString ILinker::GetOutputObjectName( LinkProject_t *pProject, unsigned int hash, CUtlString szFileName )
{
	CUtlString szTarget = pProject->m_target.GetTriplet();
	CUtlString szFileNameFormat;
	switch (pProject->linkType)
	{
	case ELINK_EXECUTABLE:
		szFileNameFormat = CUtlString(pProject->m_target.GetExecutableFileFormat(),pProject->m_szName.GetString());
		break;
	case ELINK_DYNAMIC_LIBRARY:
		szFileNameFormat = CUtlString(pProject->m_target.GetDynamicLibraryFileFormat(),pProject->m_szName.GetString());
		break;
	case ELINK_STATIC_LIBRARY:
		szFileNameFormat = CUtlString(pProject->m_target.GetStaticLibraryFileFormat(),pProject->m_szName.GetString());
		break;
	default:
		break;
	}

	return  CUtlString(
			"%s/%s/ld/%u_%s/%s",
			FPC_TEMPORAL_DIRNAME,
			szTarget.GetString(),
			hash,
			pProject->m_szName.GetString(),
			szFileNameFormat.GetString());
}

CUtlString ILinker::Link( LinkProject_t *pProject )
{
	if (pProject->m_szName == 0)
	{
		Plat_FatalErrorFunc("m_szName must be present\n");
	}
	LinkProject_t stLinkProject = *pProject;
	if (pProject->m_target.kernel == TARGET_KERNEL_ANDROID)
		stLinkProject.linkType = ELINK_DYNAMIC_LIBRARY;

	unsigned int hash = pProject->GenerateProjectHash();
	CUtlString szOutputFile = GetOutputObjectName(&stLinkProject, hash, NULL);
	filesystem2->MakeDirectory(szOutputFile.GetDirectory());

	if (stLinkProject.linkType == ELINK_STATIC_LIBRARY)
	{
		CUtlVector<CUtlString> args;
		bool shouldRecompile = false;
		for (auto object: pProject->objects)
		{
			if (filesystem2->ShouldRecompile(object.m_szObjectFile,szOutputFile))
			{
				shouldRecompile = true;
				break;
			}
		}
		if (!shouldRecompile)
			goto compiled;
		
		V_printf("  AR       %s\n", pProject->m_szName.GetString());
		args = {
			"rcs",
			szOutputFile
		};
		for (auto object: pProject->objects)
			args.AppendTail(object.m_szObjectFile);
		runner->Run("ar", args);
		runner->Wait();
	} else {
		bool shouldRecompile = false;
		for (auto object: pProject->objects)
		{
			if (filesystem2->ShouldRecompile(object.m_szObjectFile,szOutputFile))
			{
				shouldRecompile = true;
				break;
			}
		}
		for (auto object: pProject->libraryObjects)
		{
			if (filesystem2->ShouldRecompile(object,szOutputFile))
			{
				shouldRecompile = true;
				break;
			}
		}
		if (!shouldRecompile)
			goto compiled;
		
		V_printf("  LINK     %s\n", pProject->m_szName.GetString());
		CUtlVector<CUtlString> args;
		args = BuildLinkCommandLine(pProject, szOutputFile);
		runner->Run(GetCompilerExecutable(pProject), args);
		runner->Wait();
	}
compiled:
	return szOutputFile;
}

CUtlVector<CUtlString> ILinker::BuildLinkCommandLine( LinkProject_t *pProject, const char *szOutputFileName )
{
	CUtlVector<CUtlString> cmd;
	SetTarget(cmd, pProject);
	SetOutputFile(cmd, szOutputFileName);
	SetSysroot(cmd, pProject, NULL);
	SetDefaultLibraryPaths(cmd, pProject);
	UseFullFile(cmd, pProject);
	for (auto &o: pProject->objects)
	{
		LinkFile(cmd, o.m_szObjectFile);
	}
	UsePartialFile(cmd, pProject);

	for (auto &o: pProject->libraryObjects)
	{
		LinkLibraryObject(cmd, o);
	};
	for (auto &o: pProject->libraries)
	{
		LinkLibrary(cmd, o);
	};
	for (auto &o: pProject->libraryDirectories)
	{
		LinkLibraryPath(cmd, o);
	};
	return cmd;

}

CUtlVector<CUtlString> ILinker::BuildArchiveCommandLine( LinkProject_t *pProject, const char *szOutputFileName )
{
	CUtlVector<CUtlString> cmd;
}

