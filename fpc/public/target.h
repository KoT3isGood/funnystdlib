//================= Copyright kotofyt, All rights reserved ==================//
// Purpose: Target manager for compilers.
//===========================================================================//

#ifndef TARGET_T
#define TARGET_T

#include "tier0/commandline.h"
#include "tier1/utlstring.h"

#define TARGET_LINUX_MASK 0x010000
#define TARGET_APPLE_MASK 0x020000
#define TARGET_POSIX_MASK 0x030000
#define TARGET_WINDOWS_MASK 0x100000
#define TARGET_WEB_MASK 0x200000

enum ETargetKernel
{
	TARGET_KERNEL_UNDEFINED,
	TARGET_KERNEL_UNKNOWN = 1,
	TARGET_KERNEL_DEFAULT,

	TARGET_KERNEL_WINDOWS_DEVICES = TARGET_WINDOWS_MASK,
	TARGET_KERNEL_WINDOWS,

	TARGET_KERNEL_POSIX = TARGET_POSIX_MASK,
	TARGET_KERNEL_LINUX_DEVICES = TARGET_LINUX_MASK,
	TARGET_KERNEL_UNKNOWN_LINUX,
	TARGET_KERNEL_LINUX = TARGET_KERNEL_UNKNOWN_LINUX,
	TARGET_KERNEL_ALPINE_LINUX,
	TARGET_KERNEL_PC_LINUX,
	TARGET_KERNEL_ANDROID,
	TARGET_KERNEL_APPLE_DEVICES = TARGET_APPLE_MASK,
	TARGET_KERNEL_DARWIN,
	TARGET_KERNEL_IOS,
	TARGET_KERNEL_IOS_SIMULATOR,

	TARGET_KERNEL_WEB_DEVICES = TARGET_WEB_MASK,
	TARGET_KERNEL_WASI,
	TARGET_KERNEL_EMSCRIPTEN,
};

enum ETargetCPU
{
	TARGET_CPU_UNDEFINED,

	TARGET_CPU_80386,
	TARGET_CPU_80486,
	TARGET_CPU_80586,
	TARGET_CPU_80686,
	TARGET_CPU_AMD64,

	TARGET_CPU_ARM,
	TARGET_CPU_AARCH64,
	TARGET_CPU_ARM64 = TARGET_CPU_AARCH64,
	TARGET_CPU_ARMV7,
	TARGET_CPU_ARMV8,
	TARGET_CPU_ARMV9,

	TARGET_CPU_WASM32,
	TARGET_CPU_WASM64,
};
enum ETargetABI
{
	TARGET_ABI_UNDEFINED,
	TARGET_ABI_DEFAULT,
	TARGET_ABI_GNU,
	TARGET_ABI_MUSL,
	TARGET_ABI_MSVC,
};

enum ETargetOptimization
{
	TARGET_DEBUG,
	TARGET_RELEASE_SPEED,
	TARGET_RELEASE_SIZE
};

struct Target_t
{
	ETargetKernel kernel;
	ETargetCPU cpu;
	ETargetABI abi;
	ETargetOptimization optimization;
	const char *szSysroot = CommandLine()->ParamValue("-sysroot");

	CUtlString GetTriplet();
	const char *GetExecutableFileFormat();
	const char *GetStaticLibraryFileFormat();
	const char *GetDynamicLibraryFileFormat();
	static Target_t HostTarget();
	static Target_t DefaultTarget();
	static const char *StringFromCPU( ETargetCPU eCPU );
	static const char *StringFromKernel( ETargetKernel eKernel );
	static const char *StringFromABI( ETargetABI eABI );
	static ETargetCPU CPUFromString( const char *szName );
	static ETargetKernel KernelFromString( const char *szName );
	static ETargetABI ABIFromString( const char *szName );
};

inline Target_t NewTarget( ETargetKernel k, ETargetCPU c, ETargetABI abi, ETargetOptimization o )
{
	return {k,c,abi,o};
}

#define DEFAULT_TARGET_INTERFACE_VERSION "DefaultTarget001"
#define DEFAULT_TARGET_ENABLED_INTERFACE_VERSION "DefaultTargetEnabled001"

enum EShaderTarget
{
	SHADER_TARGET_VULKAN_SPIRV,
	SHADER_TARGET_OPENGL_SPIRV,
	SHADER_TARGET_GLSL,
	SHADER_TARGET_HLSL,
	SHADER_TARGET_MSL,
};


#endif
