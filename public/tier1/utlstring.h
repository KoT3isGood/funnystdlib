//================= Copyright kotofyt, All rights reserved ==================//
// Purpose: Own implementation of string.
//===========================================================================//

#ifndef TIER1_UTL_STRING_H
#define TIER1_UTL_STRING_H

#include "tier1/utlvector.h"
#include "tier0/lib.h"
#include "stdarg.h"

class CUtlString {
public:
	CUtlString( void );	
	CUtlString( const char *psz, ... );
	CUtlString( const CUtlString &sz );

	void AppendTail( const char *psz );
	void AppendTail( char ch );
	void AppendHead( const char *psz );
	void AppendHead( char ch );
	void AppendAt( size_t nPosition, const char *psz );

	void RemoveTail( size_t nCount );
	void RemoveHead( size_t nCount );
	void RemoveAt( size_t nPosition, size_t nCount );

	CUtlString GetFileName();
	CUtlString GetFileExtension();
	CUtlString GetDirectory();
	CUtlString RemoveHeadFile();

	char *GetString( void );
	size_t GetLenght( void );
	operator char*( void );
	operator CUtlVector<char>&( void );
	CUtlString& operator=(const CUtlString &sz);
	bool operator==(const char* psz);
	bool operator!=(const char* psz);
	bool operator==(CUtlString& string);
	bool operator!=(CUtlString& string);
private:
	CUtlVector<char> m_data = 0;
};
inline CUtlString::CUtlString( void )
{
	m_data.Reserve(1);
	m_data[0]=0;
}

inline CUtlString::CUtlString( const char *szFormat, ... )
{
	if (szFormat == 0)
	{
		m_data.Reserve(1);
		m_data[0]=0;
		return;
	}
	va_list vlArgs;
	va_start(vlArgs, szFormat);
	va_list vlArgs2;
	va_copy(vlArgs2, vlArgs);
	size_t nSize = V_vsnprintf(NULL, 0, szFormat, vlArgs2);
	m_data.Reserve(nSize + 1);
	va_end(vlArgs2);
	V_vsnprintf(m_data.GetData(), nSize + 1, szFormat, vlArgs);
	m_data.Resize(nSize + 1);
	va_end(vlArgs);
}

inline CUtlString::CUtlString( const CUtlString &sz )
{
	m_data = sz.m_data;
};


inline void CUtlString::AppendTail( const char *psz )
{
	m_data.Resize(V_strlen(m_data.GetData()));
	m_data.AppendTail(psz,V_strlen(psz));
	m_data.Reserve(m_data.GetSize()+1);
	m_data[m_data.GetSize()] = 0;
}

inline void CUtlString::AppendHead( const char *psz )
{
	m_data.AppendHead(psz,V_strlen(psz));
	m_data.Reserve(m_data.GetSize()+1);
	m_data[m_data.GetSize()] = 0;
}

inline void CUtlString::AppendTail( char ch )
{
	m_data.Resize(V_strlen(m_data.GetData()));
	m_data.AppendTail(ch);
	m_data.Reserve(m_data.GetSize()+1);
	m_data[m_data.GetSize()] = 0;
}

inline void CUtlString::AppendHead( char ch )
{
	m_data.AppendHead(ch);
	m_data.Reserve(m_data.GetSize()+1);
	m_data[m_data.GetSize()] = 0;
}
inline void CUtlString::AppendAt( size_t nPosition, const char *psz )
{

}

inline void CUtlString::RemoveTail( size_t nCount )
{
	m_data.RemoveTail(nCount);
	m_data[m_data.GetSize()-1] = 0;
}

inline void CUtlString::RemoveHead( size_t nCount )
{
	m_data.RemoveHead(nCount);
}

inline void CUtlString::RemoveAt( size_t nPosition, size_t nCount )
{

}

inline CUtlString CUtlString::GetFileName()
{
	CUtlString szFileName = GetString();
	if (GetLenght() == 0)
		return "";

	char *pLast = &m_data[GetLenght()-1];	
	while (pLast != m_data.GetData())
	{
		if (*pLast=='/')
		{
			pLast++;
			break;			
		}
		pLast--;
	}

	return pLast;
}

inline CUtlString CUtlString::GetFileExtension()
{
	CUtlString szFileName = GetString();

	char *pLast = &m_data[GetLenght()-1];	
	while (pLast != m_data.GetData())
	{
		if (*pLast=='.')
			break;			
		if (*pLast=='/')
			return NULL;
		pLast--;
	}

	return pLast+1;
}



inline CUtlString CUtlString::GetDirectory()
{
	if (GetLenght() == 0)
		return NULL;
	size_t iNumDeleted = 0;
	char *pLast = &m_data[GetLenght()-1];
	CUtlString szDirectory = GetString();
	while (pLast != m_data.GetData())
	{
		if (*pLast=='/')
		{
			iNumDeleted++;
			break;
		}
		pLast--;
		iNumDeleted++;
	}

	szDirectory.RemoveTail(iNumDeleted);

	return szDirectory;
}

inline CUtlString CUtlString::RemoveHeadFile()
{
	size_t iLenght = GetLenght();
	size_t iNumDeleted = 0;
	char *pc = GetString();
	CUtlString szDirectory = pc;

	if (GetLenght() == 0)
		return NULL;
	while ( iNumDeleted < iLenght )
	{
		if (*pc == '/')
			goto remove_slashes;
		pc++;
		iNumDeleted++;
	}
	return NULL;
remove_slashes:
	while ( iNumDeleted < iLenght )
	{
		if (*pc != '/')
		{
			szDirectory.RemoveHead(iNumDeleted);
			return szDirectory;
		}
		pc++;
		iNumDeleted++;
	}
	return NULL;
}

inline char *CUtlString::GetString( void )
{
	return m_data.GetData();
}

inline size_t CUtlString::GetLenght( void )
{
	return V_strlen(GetString());
}

inline CUtlString::operator char*( void )
{
	return GetString();
}

inline CUtlString::operator CUtlVector<char>&( void )
{
	return m_data;
}

inline CUtlString &CUtlString::operator=(const CUtlString &sz)
{
	if (this != &sz)
	{
		m_data = sz.m_data;
	}
	return *this;
}

inline bool CUtlString::operator==(const char *psz)
{
	if (psz==0)
		psz = "";
	if (!V_strcmp(GetString(), psz))
		return true;
	return false;
}

inline bool CUtlString::operator!=(const char *psz)
{
	if (psz==0)
		psz = "";
	if (!V_strcmp(GetString(), psz))
		return false;
	return true;
}

inline bool CUtlString::operator==(CUtlString &string)
{
	if (!V_strcmp(GetString(), string.GetString()))
		return true;
	return false;
}

inline bool CUtlString::operator!=(CUtlString &string)
{
	if (!V_strcmp(GetString(), string.GetString()))
		return false;
	return true;
}
#endif
