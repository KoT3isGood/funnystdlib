#include "runner.h"
#include "tier0/platform.h"
#include "tier1/interface.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"
#include "unistd.h"
#ifndef __WIN32__
#include "sys/wait.h"
#else
#include "windows.h"
#endif
#include "tier0/commandline.h"

#include "winerunner.h"

#ifndef __WIN32__
class CPOSIXRunner: public IRunner
{
public:
	virtual int Run( CUtlString szName, CUtlVector<CUtlString>& args ) override;
	virtual int Run( CUtlString szName, CUtlString szDirectory, CUtlVector<CUtlString>& args ) override;
	virtual int Run( CUtlString szName, CUtlString szDirectory, CUtlVector<CUtlString>& args, CUtlVector<CUtlString>& environment ) override;
	virtual int Wait( void ) override;
	CUtlVector<pid_t> m_processes;
};

EXPOSE_INTERFACE(CPOSIXRunner, IRunner, RUNNER_INTERFACE_NAME);
IRunner *runner;
IWineRunner *winerunner;

int CPOSIXRunner::Run(CUtlString szName, CUtlVector<CUtlString>& args)
{
	pid_t pid = fork();
	if (pid < 0)
		Plat_FatalErrorFunc("Failed to fork");
	/* child */
	if (pid == 0)
	{
		CUtlVector<const char*> execargs;
		execargs.AppendTail(szName);
		if (CommandLine()->CheckParam("-fpcdebug"))
			V_printf("%s",szName.GetString());
		for (auto &arg: args)
		{
			execargs.AppendTail(arg);
			if (CommandLine()->CheckParam("-fpcdebug"))
				V_printf(" %s",arg.GetString());
		}
		if (CommandLine()->CheckParam("-fpcdebug"))
			V_printf("\n");
		execargs.AppendTail(0);
		if ( execvp(szName, (char *const*)execargs.GetData()) == -1 )
		{
			V_printf("Failed to launch %s\n",szName.GetString());
			_exit(0);
		}
	}
	m_processes.AppendTail(pid);
	return 0;
}

int CPOSIXRunner::Run(CUtlString szName, CUtlString szDirectory, CUtlVector<CUtlString>& args)
{
	pid_t pid = fork();
	if (pid < 0)
		Plat_FatalErrorFunc("Failed to fork");
	/* child */
	if (pid == 0)
	{
		CUtlVector<const char*> execargs;
		execargs.AppendTail(szName);
		if (CommandLine()->CheckParam("-fpcdebug"))
			V_printf("%s",szName.GetString());
		for (auto &arg: args)
		{
			execargs.AppendTail(arg);
			if (CommandLine()->CheckParam("-fpcdebug"))
				V_printf(" %s",arg.GetString());
		}
		if (CommandLine()->CheckParam("-fpcdebug"))
			V_printf("\n");
		execargs.AppendTail(0);
		chdir(szDirectory.GetString());
		if ( execvp(szName, (char *const*)execargs.GetData()) == -1 )
		{
			V_printf("Failed to launch %s\n",szName.GetString());
			_exit(0);
		}
	}
	m_processes.AppendTail(pid);
	/* parent */
	return 0;
}

int CPOSIXRunner::Run(CUtlString szName, CUtlString szDirectory, CUtlVector<CUtlString>& args, CUtlVector<CUtlString>& environment)
{

}

int CPOSIXRunner::Wait( void )
{
	for (int i = 0; i<m_processes.GetSize(); i++)
	{
		int status;
		pid_t wpid = waitpid(m_processes[i], &status, 0);
	}
	m_processes = {};
	return 0;
};
#else
class CWindowsRunner: public IRunner
{
public:
	virtual int Run( CUtlString szName, CUtlVector<CUtlString>& args ) override;
	virtual int Run( CUtlString szName, CUtlString szDirectory, CUtlVector<CUtlString>& args ) override;
	virtual int Run( CUtlString szName, CUtlString szDirectory, CUtlVector<CUtlString>& args, CUtlVector<CUtlString>& environment ) override;
	virtual int Wait( void ) override;
	CUtlVector<HANDLE> m_processes;
};

EXPOSE_INTERFACE(CWindowsRunner, IRunner, RUNNER_INTERFACE_NAME);
IRunner *runner;
IWineRunner *winerunner;

int CWindowsRunner::Run(CUtlString szName, CUtlVector<CUtlString>& args)
{
	CUtlString szArgs = szName;
	for (auto &s: args)
	{
		szArgs.AppendHead(" ");
		szArgs.AppendHead(s);
	}
	STARTUPINFOA stStartup = {0};
	PROCESS_INFORMATION stProcessInfo = {0};
	if (CommandLine()->CheckParam("-fpcdebug"))
		V_printf("%s\n",szArgs.GetString());
	CreateProcessA(szName, szArgs, NULL, NULL, FALSE, 0, NULL, NULL, &stStartup, &stProcessInfo);
	m_processes.AppendTail(stProcessInfo.hProcess);
	return 0;
}

int CWindowsRunner::Run(CUtlString szName, CUtlString szDirectory, CUtlVector<CUtlString>& args)
{
	CUtlString szArgs = szName;
	STARTUPINFOA stStartup;
	PROCESS_INFORMATION stProcessInfo;
	if (CommandLine()->CheckParam("-fpcdebug"))
		V_printf("%s\n",szArgs.GetString());
	CreateProcessA(szName, szArgs, NULL, NULL, FALSE, 0, NULL, szDirectory, &stStartup, &stProcessInfo);
	if (stProcessInfo.hProcess)
		m_processes.AppendTail(stProcessInfo.hProcess);
	return 0;
}

int CWindowsRunner::Run(CUtlString szName, CUtlString szDirectory, CUtlVector<CUtlString>& args, CUtlVector<CUtlString>& environment)
{

}

int CWindowsRunner::Wait( void )
{
	for ( HANDLE &process: m_processes)
	{
		WaitForSingleObject(process, INFINITE);
	}
	m_processes = {};
	return 0;
};

#endif
