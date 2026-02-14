#include "helper.h"
#include "tier2/ifilesystem.h"

static CUtlString s_szTop;
static CUtlString s_szBottom;
void BuildPage( const char *szFakeFile )
{
	V_printf("%s\n", szFakeFile);
	CUtlString szFile = szFakeFile;
	if (szFile.GetFileExtension() != "html")
		return;

	CUtlString szOutputDir = CUtlString("../build/docs/%s", szFile.GetDirectory().GetString());
	CUtlString szOutputFile = CUtlString("../build/docs/%s", szFile.GetString());
	filesystem2->MakeDirectory(szOutputDir);
	IFileHandle *pFile = filesystem->Open(szFile, FILEMODE_READ);
	CUtlString szFileData = filesystem->ReadString(pFile);
	IFileHandle *pOutput = filesystem->Open(szOutputFile, FILEMODE_WRITE);
	filesystem->Write(pOutput, s_szTop, s_szTop.GetLenght());
	filesystem->Write(pOutput, szFileData, szFileData.GetLenght());
	filesystem->Write(pOutput, s_szBottom, s_szBottom.GetLenght());
}

DECLARE_BUILD_STAGE(docs)
{
	IFileHandle *pTop = filesystem->Open("top.html", FILEMODE_READ);
	IFileHandle *pBottom = filesystem->Open("bottom.html", FILEMODE_READ);
	s_szTop = filesystem->ReadString(pTop);
	s_szBottom = filesystem->ReadString(pBottom);

	Plat_ListDirRecursive("fpc", BuildPage, NULL);

	return 0;
}
