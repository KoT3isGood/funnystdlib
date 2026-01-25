#ifndef DEPLOY_H
#define DEPLOY_H

#include "tier0/platform.h"
#include "tier1/utlstring.h"

abstract_class IDeployDevice
{
public:
	virtual const char *GetName() = 0;
	virtual void *Install( const char *szPath ) = 0;
};

abstract_class IDeployDeviceManager
{
public:
	virtual void AddDevice( IDeployDevice *pDevice ) = 0;
	virtual IDeployDevice *FindDeviceByName( const char *szDeviceName ) = 0;
	virtual const CUtlVector<IDeployDevice*> &ListDevices() = 0;
};

IDeployDeviceManager *DeployDeviceManager();


#endif
