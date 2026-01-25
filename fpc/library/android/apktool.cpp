#include "apktool.h"
#include "helper.h"
#include "tier0/lib.h"
#include "tier0/platform.h"
#include "tier0/commandline.h"
#include "tier1/interface.h"
#include "runner.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"

void AndroidManifest_t::SetPackageVersion( CUtlString szVersion )
{
	m_szVersion = szVersion;
}

void AndroidManifest_t::SetPackageBuild( uint64_t nBuild )
{
	m_nBuild = nBuild;
}

void AndroidManifest_t::SetPackageID( CUtlString szPackageID )
{
	m_szPackageID = szPackageID;
}

void AndroidManifest_t::SetPackageName( CUtlString szPackageName )
{
	m_szPackageName = szPackageName;
}

void AndroidManifest_t::SetTargetSDKVersion( uint64_t nTargetVersion )
{
	m_nTargetVersion = nTargetVersion;
}

void AndroidManifest_t::SetMinSDKVersion( uint64_t nMinVersion )
{
	m_nMinVersion = nMinVersion;
}

void AndroidManifest_t::AddUserFeature( CUtlString szName, bool bIsRequired, uint64_t nVersion )
{

}

void AndroidManifest_t::AddUserLibrary( CUtlString szPath )
{

}

CUtlString AndroidManifest_t::BuildManifest()
{
	CPUProject_t project = {};
	project.m_szName = m_szPackageName;
	unsigned int hash = project.GenerateProjectHash();
	CUtlString szOutputDir = CUtlString("%s/android/%u_%s/",FPC_TEMPORAL_DIRNAME, hash, m_szPackageID.GetString());
	filesystem2->MakeDirectory(szOutputDir);
	filesystem2->MakeDirectory(CUtlString("%s/res", szOutputDir.GetString()));

	CUtlString szAndroidManifestPath = CUtlString("%s/AndroidManifest.xml", szOutputDir.GetString());
	FILE *pAndroidManifest = V_fopen(szAndroidManifestPath, "wb");

	V_fprintf(pAndroidManifest, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	V_fprintf(pAndroidManifest, "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\" package=\"%s\">\n", m_szPackageID.GetString());
	V_fprintf(pAndroidManifest, "<uses-sdk android:minSdkVersion=\"%lu\" android:targetSdkVersion=\"%lu\" />\n",m_nMinVersion, m_nTargetVersion);
	V_fprintf(pAndroidManifest, "<application android:label=\"%s\" android:hasCode=\"false\" android:debuggable=\"true\">\n", m_szPackageName.GetString());
	V_fprintf(pAndroidManifest, "<activity android:name=\"android.app.NativeActivity\" android:label=\"%s\" android:exported=\"true\">\n", m_szPackageName.GetString());
	V_fprintf(pAndroidManifest, "<meta-data android:name=\"android.app.lib_name\" android:value=\"native-app\" />\n");
	V_fprintf(pAndroidManifest, "<intent-filter>\n");
	V_fprintf(pAndroidManifest, "<action android:name=\"android.intent.action.MAIN\" />\n");
	V_fprintf(pAndroidManifest, "<category android:name=\"android.intent.category.LAUNCHER\" />\n");
	V_fprintf(pAndroidManifest, "</intent-filter>\n");
	V_fprintf(pAndroidManifest, "</activity>\n");
	V_fprintf(pAndroidManifest, "</application>\n");
	V_fprintf(pAndroidManifest, "</manifest>\n");

	V_fclose(pAndroidManifest);
	return szOutputDir;
};



class CAPKTool : public IAPKTool
{
public:
	virtual CUtlString BuildPackage( AndroidManifest_t manifest, CUtlString szManifestDir ) override;
	virtual CUtlString SignPackage( const char *szApk, LegalInfo_t *pLegalInfo, const char *szAlias, const char *szStorePassword, const char *szKeyPassword ) override;
private:
};



CUtlString CAPKTool::BuildPackage( AndroidManifest_t manifest, CUtlString szManifestDir )
{

	V_printf("  APKTOOL  %s\n", manifest.m_szPackageID.GetString());

	IINISection *pSection = NULL;
	const char *szBuildTools;
	pSection = g_pConfig->GetSection("Android_Build_Tools");
	if (!pSection)
		Plat_FatalErrorFunc("build_tools are required for this target");
	szBuildTools = pSection->GetStringValue("path");
	CUtlVector<CUtlString> args = {
		"package",
		"-f",
		"-M",
		"AndroidManifest.xml",
		"-S",
		"res",
		"-I",
		CUtlString("%s/../../platforms/android-%lu/android.jar", szBuildTools, manifest.m_nTargetVersion),
		"-F",
		CUtlString("%s.unaligned.apk", manifest.m_szPackageID.GetString()),
	};
	runner->Run(CUtlString("%s/aapt",szBuildTools),szManifestDir,args);
	runner->Wait();

	args = {
		"-u",
		CUtlString("%s.unaligned.apk", manifest.m_szPackageID.GetString()),
		"lib/x86_64/libnative-app.so",
	};
	runner->Run("zip",szManifestDir, args);
	runner->Wait();
	
	args = {
		"-f",
		"-v",
		"4",
		CUtlString("%s.unaligned.apk", manifest.m_szPackageID.GetString()),
		CUtlString("%s.apk", manifest.m_szPackageID.GetString()),
	};
	runner->Run(CUtlString("%s/zipalign",szBuildTools),szManifestDir,args);
	runner->Wait();

	return CUtlString("%s/%s.apk",szManifestDir.GetString(), manifest.m_szPackageID.GetString());
}

CUtlString CAPKTool::SignPackage( const char *szApk, LegalInfo_t *pLegalInfo, const char *szAlias, const char *szStorePassword, const char *szKeyPassword )
{
	V_printf("  APKSIGN    %s\n", szApk );
	CUtlString szOutput = CUtlString("%s.signed", szApk);
	LegalInfo_t stLegalInfo = {};
	CUtlVector<CUtlString> args = {};
	
	CPUProject_t project = {};
	project.m_szName = szAlias;
	unsigned int hash = project.GenerateProjectHash();
	CUtlString szOutputDir = CUtlString("%s/android/%u_%s/",FPC_TEMPORAL_DIRNAME, hash, szAlias);
	filesystem2->MakeDirectory(szOutputDir);
	CUtlString szKeystoreDir = CUtlString("%s/%s.jks",szOutputDir.GetString(),szAlias);
	CUtlString szDname = "";
	
	IINISection *pSection = NULL;
	const char *szBuildTools;
	pSection = g_pConfig->GetSection("Android_Build_Tools");
	if (!pSection)
		Plat_FatalErrorFunc("build_tools are required for this target");
	szBuildTools = pSection->GetStringValue("path");

	// We need to have a name at least
	stLegalInfo.FirstName = "Gordon";
	stLegalInfo.LastName = "Freeman";

	if (pLegalInfo != NULL)
		stLegalInfo = *pLegalInfo;

	if (!stLegalInfo.FirstName)
		Plat_FatalErrorFunc("First name wasn't provided\n");
	if (!stLegalInfo.LastName)
		Plat_FatalErrorFunc("Last name wasn't provided\n");
	szDname.AppendTail(CUtlString("CN=%s %s", stLegalInfo.FirstName, stLegalInfo.LastName));

	args = {
		"-genkeypair",
		"-keystore", szKeystoreDir,
		"-alias", szAlias,
		"-keyalg", "RSA",
		"-keysize", "2048",
		"-validity", "10000",
		"-storepass", szStorePassword,
		"-keypass", szKeyPassword,
		"-dname", szDname,
	};
	runner->Run("keytool", args);
	runner->Wait();
	
	args = {
		"sign",
		"--ks", szKeystoreDir,
		"--ks-key-alias", szAlias,
		"--ks-pass", CUtlString("pass:%s",szStorePassword),
		"--key-pass", CUtlString("pass:%s",szKeyPassword),
		szApk,
	};
	runner->Run(CUtlString("%s/apksigner",szBuildTools), args);
	runner->Wait();

	return szApk;
};

IAPKTool *APKTool()
{
	static CAPKTool s_apktool = {};
	return &s_apktool;
}
