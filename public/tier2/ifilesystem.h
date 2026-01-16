#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "iappsystem.h"
#include "tier0/platform.h"
#include "tier1/utlbuffer.h"

enum EFileMode
{
	FILEMODE_READ = 0x01,
	FILEMODE_WRITE = 0x02,
	FILEMODE_APPEND = 0x04,
};

enum ESeekMode
{
	SEEKMODE_RELATIVE_CURRENT,
	SEEKMODE_RELATIVE_START,
	SEEKMODE_RELATIVE_END,
};

abstract_class IFileSystem;

class IFileHandle
{
public:
	IFileSystem *m_pFileSystem;
};

abstract_class IDirectoryHandle
{
};

//----------------------------------------------------------------------------
// IFileSystem is an app system which manages files, could have different 
// file systems etc. Because of that there is base file system which manages 
// others. PAK files are opened first, then mounted stuff comes second and 
// host's system last.
//----------------------------------------------------------------------------
abstract_class IFileSystem: public IAppSystem
{
public:
	virtual IFileHandle *Open( const char *szFileName, int eOpCode ) = 0;	
	virtual size_t Write( IFileHandle *pFile, const void *pData, size_t nDataSize ) = 0;
	virtual size_t Read( IFileHandle *pFile, void *pData, size_t nDataSize ) = 0;

	virtual size_t Seek( IFileHandle *pFile, ESeekMode eSeekMode, size_t nOffset ) = 0;
	virtual size_t Tell( IFileHandle *pFile ) = 0;
	virtual size_t Size( IFileHandle *pFile ) = 0;

	virtual void Close( IFileHandle *pFile ) = 0;

	// Some cool stuff
	virtual CUtlBuffer<unsigned char> Read( IFileHandle *pFile ) = 0;

	// Leaks memory
	// Should be cleaned by the user
	virtual const char *ReadString( IFileHandle *pFile ) = 0;
	
	// Directory stuff
	virtual IDirectoryHandle *OpenDir( const char *szDirName ) = 0;
	virtual void CloseDir( IDirectoryHandle *pDir ) = 0;

	IFileSystem *m_pNext;
	
	void RegisterFileSystem();
};

#define FILESYSTEM_INTERFACE_VERSION "FileSystem001"

extern IFileSystem *filesystem;

#endif
