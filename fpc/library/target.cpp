#include "target.h"
#include "tier0/commandline.h"
#include "tier1/utlstring.h"
#include "sys/utsname.h"

// see Target_t::HostTarget
#if defined(__i386__) && defined(POSIX)
static utsname s_uname;
static int s_iuNameRes = uname(&s_uname);
#endif

//-----------------------------------------------------------------------------
// Generates triplet suitable for most compilers.
//-----------------------------------------------------------------------------
CUtlString Target_t::GetTriplet()
{
	CUtlString triplet = "";

	triplet.AppendTail(StringFromCPU(cpu));
	triplet.AppendTail("-");
	triplet.AppendTail(StringFromKernel(kernel));

	if ( abi != TARGET_ABI_UNDEFINED && abi != TARGET_ABI_DEFAULT )
	{
		triplet.AppendTail("-");
		triplet.AppendTail(StringFromABI(abi));
	}


	return triplet;
}

const char *Target_t::GetExecutableFileFormat()
{
	if (kernel & TARGET_KERNEL_POSIX)
		return "%s";

	if (kernel & TARGET_KERNEL_WINDOWS_DEVICES)
	{
		return "%s.exe";
	}
	return NULL;
}

const char *Target_t::GetStaticLibraryFileFormat()
{
	if (kernel & TARGET_KERNEL_POSIX)
		return "lib%s.a";

	if (kernel & TARGET_KERNEL_WINDOWS_DEVICES)
	{
		switch(abi)
		{
		case TARGET_ABI_MSVC:
			return "%s.lib";
		default:
			return "lib%s.a";
		}
	}
	return NULL;
}
const char *Target_t::GetDynamicLibraryFileFormat()
{
	if (kernel & TARGET_KERNEL_LINUX_DEVICES)
		return "lib%s.so";
	if (kernel & TARGET_KERNEL_APPLE_DEVICES)
		return "lib%s.dylib";
	if (kernel & TARGET_KERNEL_WINDOWS_DEVICES)
		return "lib%s.dylib";
	return NULL;
}
//----------------------------------------------------------------------------
// Returns target on which fpc is being run
//
// For targets such as i386, i486, i586
//----------------------------------------------------------------------------
Target_t Target_t::HostTarget()
{
	ETargetKernel kernel = TARGET_KERNEL_UNDEFINED;
#if defined(__linux__)
	kernel = TARGET_KERNEL_LINUX;
#elif defined(__APPLE__)
	kernel TARGET_KERNEL_DARWIN;
#endif
ETargetCPU cpu = TARGET_CPU_UNDEFINED;
#if defined(__x86_64__)
	cpu = TARGET_CPU_AMD64;
#endif
#if defined(__i386__)
	cpu = TARGET_CPU_80386;
#endif
// POSIX doesn't know about these
// use uname to get them
#if defined(__i386__) && defined(POSIX)
	if (!V_strcmp("i486", s_uname.machine))
		cpu = TARGET_CPU_80486;
	if (!V_strcmp("i586", s_uname.machine))
		cpu = TARGET_CPU_80586;
	if (!V_strcmp("i686", s_uname.machine))
		cpu = TARGET_CPU_80686;
#endif
	ETargetABI abi = TARGET_ABI_GNU;

#ifdef FPC_ARCH
	cpu = CPUFromString(FPC_ARCH);
#endif
#ifdef FPC_OS
	kernel = KernelFromString(FPC_OS);
#endif
#ifdef FPC_ABI
	abi = ABIFromString(FPC_ABI);
#endif


	return {
		.kernel = kernel,
		.cpu = cpu,
		.abi = abi,
		.optimization = TARGET_DEBUG,
	};
};

//-----------------------------------------------------------------------------
// Returns default target for build, by default it will be host target
//-----------------------------------------------------------------------------
Target_t Target_t::DefaultTarget()
{
	CUtlString szDevice = CommandLine()->ParamValue("-device");
	CUtlString szOS = CommandLine()->ParamValue("-os");
	CUtlString szArch = CommandLine()->ParamValue("-arch");
	CUtlString szAbi = CommandLine()->ParamValue("-abi");

	ETargetKernel kernel = KernelFromString(szOS);
	ETargetCPU cpu = CPUFromString(szArch);
	ETargetABI abi = ABIFromString(szAbi);
	if (cpu == TARGET_CPU_UNDEFINED)
		cpu = HostTarget().cpu;
	if (kernel == TARGET_KERNEL_UNDEFINED)
		kernel = HostTarget().kernel;
	if (abi == TARGET_ABI_UNDEFINED)
		abi = HostTarget().abi;

	
	return {
		.kernel = kernel,
		.cpu = cpu,
		.abi = abi,
		.optimization = TARGET_DEBUG,
	};
}
const char *Target_t::StringFromCPU( ETargetCPU cpu )
{
	if ( cpu == TARGET_CPU_AMD64 )
		return "x86_64";
	if ( cpu == TARGET_CPU_80386 )
		return "i386";
	if ( cpu == TARGET_CPU_80486 )
		return "i486";
	if ( cpu == TARGET_CPU_80586 )
		return "i586";
	if ( cpu == TARGET_CPU_80686 )
		return "i686";
	if ( cpu == TARGET_CPU_AARCH64 )
		return "aarch64";
	if ( cpu == TARGET_CPU_WASM32 )
		return "wasm32";
	return NULL;
}

const char *Target_t::StringFromKernel( ETargetKernel kernel )
{
	if ( kernel == TARGET_KERNEL_UNKNOWN )
		return "unknown-unknown";
	if ( kernel == TARGET_KERNEL_UNKNOWN_LINUX )
		return "unknown-linux";
	if ( kernel == TARGET_KERNEL_PC_LINUX )
		return "pc-linux";
	if ( kernel == TARGET_KERNEL_ALPINE_LINUX )
		return "alpine-linux";
	if ( kernel == TARGET_KERNEL_WINDOWS )
		return "pc-windows";
	if ( kernel == TARGET_KERNEL_DARWIN )
		return "apple-darwin";
	if ( kernel == TARGET_KERNEL_IOS )
		return "apple-ios";
	if ( kernel == TARGET_KERNEL_ANDROID )
		return "linux-android";
	if ( kernel == TARGET_KERNEL_WASI )
		return "unknown-wasi";
	if ( kernel == TARGET_KERNEL_EMSCRIPTEN )
		return "unknown-emscripten";
	return NULL;
}

const char *Target_t::StringFromABI( ETargetABI abi )
{
	if ( abi == TARGET_ABI_GNU )
		return "gnu";
	if ( abi == TARGET_ABI_MUSL )
		return "musl";
	return NULL;
}

ETargetCPU Target_t::CPUFromString( const char *szName )
{
	CUtlString szUtlName = szName;
	if ( szUtlName == "x86_64" )
		return TARGET_CPU_AMD64;
	else if ( szUtlName == "i386" )
		return TARGET_CPU_80386;
	else if ( szUtlName == "i486" )
		return TARGET_CPU_80486;
	else if ( szUtlName == "i586" )
		return TARGET_CPU_80586;
	else if ( szUtlName == "i686" )
		return TARGET_CPU_80686;
	else if ( szUtlName == "aarch64" )
		return TARGET_CPU_AARCH64;
	else if ( szUtlName == "wasm32" )
		return TARGET_CPU_WASM32;
	return TARGET_CPU_UNDEFINED;
}

ETargetKernel Target_t::KernelFromString( const char *szName )
{
	CUtlString szUtlName = szName;
	if ( szUtlName == "unknown" )
		return TARGET_KERNEL_UNKNOWN;
	else if ( szUtlName == "windows" )
		return TARGET_KERNEL_WINDOWS;
	else if ( szUtlName == "linux" )
		return TARGET_KERNEL_LINUX;
	else if ( szUtlName == "unknown-linux" )
		return TARGET_KERNEL_UNKNOWN_LINUX;
	else if ( szUtlName == "pc-linux" )
		return TARGET_KERNEL_PC_LINUX;
	else if ( szUtlName == "alpine-linux" )
		return TARGET_KERNEL_ALPINE_LINUX;
	else if ( szUtlName == "macos" )
		return TARGET_KERNEL_DARWIN;
	else if ( szUtlName == "ios" )
		return TARGET_KERNEL_IOS;
	else if ( szUtlName == "android" )
		return TARGET_KERNEL_ANDROID;
	return TARGET_KERNEL_UNDEFINED;
}

ETargetABI Target_t::ABIFromString( const char *szName )
{
	CUtlString szUtlName = szName;
	if ( szUtlName == "gnu" )
		return TARGET_ABI_GNU;
	else if ( szUtlName == "musl" )
		return TARGET_ABI_MUSL;
	return TARGET_ABI_UNDEFINED;
}

