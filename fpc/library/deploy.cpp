#include "deploy.h"


class CDeployDeviceManager: public IDeployDeviceManager
{
public:
	virtual void AddDevice( IDeployDevice *pDevice ) override;
	virtual IDeployDevice *FindDeviceByName( const char *szDeviceName ) override;
	virtual const CUtlVector<IDeployDevice*> &ListDevices() override;
private:
	CUtlVector<IDeployDevice*> m_devices;
};

static CDeployDeviceManager s_deviceManager;

void CDeployDeviceManager::AddDevice( IDeployDevice *pDevice )
{
	m_devices.AppendTail(pDevice);
}

IDeployDevice *CDeployDeviceManager::FindDeviceByName( const char *szDeviceName )
{
	for ( auto device: m_devices )
	{
		if ( !V_strcmp(device->GetName(), szDeviceName) )
			return device;
	}
	return NULL;
}

const CUtlVector<IDeployDevice*> &CDeployDeviceManager::ListDevices()
{
	return m_devices;
}


IDeployDeviceManager *DeployDeviceManager()
{
	return &s_deviceManager;
}
