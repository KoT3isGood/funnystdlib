#include "tier1/utlmutex.h"
#include "pthread.h"

CUtlLock::CUtlLock()
{
	pthread_mutex_init(&m_lock, NULL);
};

CUtlLock::~CUtlLock()
{
	pthread_mutex_destroy(&m_lock);	
};

void CUtlLock::Lock()
{
	pthread_mutex_lock(&m_lock);

};

void CUtlLock::Unlock()
{
	pthread_mutex_unlock(&m_lock);
};
