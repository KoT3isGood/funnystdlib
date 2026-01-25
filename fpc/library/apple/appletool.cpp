#include "appletool.h"
#include "helper.h"
#include "runner.h"
#include "appleauth/iauth.h"

void AppleManifest_t::SetPackageID( CUtlString szPackageID )
{
	m_szPackageID = szPackageID;
}

void AppleManifest_t::SetPackageName( CUtlString szPackageName )
{
	m_szPackageName = szPackageName;
}

void AppleManifest_t::SetPackageExecutable( CUtlString szPackageExecutable )
{
	m_szPackageExecutable = szPackageExecutable;
}

CUtlString AppleManifest_t::BuildManifest()
{
	CPUProject_t project = {};
	project.m_szName = m_szPackageName;
	unsigned int hash = project.GenerateProjectHash();
	CUtlString szOutputDir = CUtlString("%s/apple/%u_%s/app/",FPC_TEMPORAL_DIRNAME, hash, m_szPackageID.GetString());
	filesystem2->MakeDirectory(szOutputDir);
	filesystem2->CopyFile(szOutputDir, m_szPackageExecutable);
	CUtlString szInfoPlist = CUtlString("%s/Info.plist", szOutputDir.GetString());
	FILE *pInfoPlistFile = V_fopen(szInfoPlist, "wb");

	V_fprintf(pInfoPlistFile, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	V_fprintf(pInfoPlistFile, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
	V_fprintf(pInfoPlistFile, "<plist version=\"1.0\">\n");
	V_fprintf(pInfoPlistFile, "<dict>\n");
	V_fprintf(pInfoPlistFile, "<key>CFBundleIdentifier</key>\n");
	V_fprintf(pInfoPlistFile, "<string>%s</string>\n", m_szPackageID.GetString());
	V_fprintf(pInfoPlistFile, "<key>CFBundleName</key>\n");
	V_fprintf(pInfoPlistFile, "<string>%s</string>\n", m_szPackageName.GetString());
	V_fprintf(pInfoPlistFile, "<key>CFBundleDisplayName</key>\n");
	V_fprintf(pInfoPlistFile, "<string>%s</string>\n", m_szPackageName.GetString());
	V_fprintf(pInfoPlistFile, "<key>CFBundleExecutable</key>\n");
	V_fprintf(pInfoPlistFile, "<string>%s</string>\n", m_szPackageExecutable.GetFileName().GetString());
	V_fprintf(pInfoPlistFile, "<key>CFBundlePackageType</key>\n");
	V_fprintf(pInfoPlistFile, "<string>APPL</string>\n");
	V_fprintf(pInfoPlistFile, "<key>CFBundleVersion</key>\n");
	V_fprintf(pInfoPlistFile, "<string>1.0</string>\n");
	V_fprintf(pInfoPlistFile, "<key>CFBundleShortVersionString</key>\n");
	V_fprintf(pInfoPlistFile, "<string>1.0</string>\n");
	V_fprintf(pInfoPlistFile, "<key>LSRequiresIPhoneOS</key>\n");
	V_fprintf(pInfoPlistFile, "<true/>\n");
	V_fprintf(pInfoPlistFile, "<key>UIDeviceFamily</key>\n");
	V_fprintf(pInfoPlistFile, "<array>\n");
	V_fprintf(pInfoPlistFile, "<integer>1</integer>\n");
	V_fprintf(pInfoPlistFile, "<integer>2</integer>\n");
	V_fprintf(pInfoPlistFile, "</array>\n");
	V_fprintf(pInfoPlistFile, "</dict>\n");

	V_fclose(pInfoPlistFile);
	return szOutputDir;
}


class CAppleTool: public IAppleTool
{
public:
	virtual CUtlString BuildPackage( AppleManifest_t manifest, CUtlString szManifestDir ) override;
	virtual CUtlString SignPackage( const char *szIpa, const char *szPassword ) override;
};

CUtlString CAppleTool::BuildPackage( AppleManifest_t manifest, CUtlString szManifestDir )
{
	CUtlVector<CUtlString> args = {};
	args = {
		"-r",
		CUtlString("../%s.ipa", manifest.m_szPackageName.GetString()),
		CUtlString("."),
	};
	runner->Run("zip",szManifestDir, args);
	runner->Wait();
	return CUtlString("../%s.ipa", manifest.m_szPackageName.GetString());
}

static IAppleAuth *g_pAppleAuth;
CUtlString CAppleTool::SignPackage( const char *szIpa, const char *szPassword )
{
	CreateInterfaceFn fnFactory = Sys_GetFactory("appleauth");
	if (fnFactory == NULL)
		Plat_FatalErrorFunc("Couldn't get xtool\n");

	g_pAppleAuth = (IAppleAuth*)fnFactory(APPLE_AUTH_INTERFACE_VERSION, NULL);
	g_pAppleAuth->Init();
	char *szGSAEmail = CommandLine()->ParamValue("-apple-login");
	char *szGSAPassword = CommandLine()->ParamValue("-apple-password");
	if (szGSAEmail && szGSAPassword)
		g_pAppleAuth->SubmitLoginData(szGSAEmail, szGSAPassword);
	return szIpa;
}


IAppleTool *AppleTool()
{
	static CAppleTool s_tool;
	return &s_tool;
}
