#ifndef APPLE_AUTH_H
#define APPLE_AUTH_H

#include "tier0/platform.h"
#include "tier2/iappsystem.h"

enum EAppleAuthDaemonStatus
{
	APPLE_AUTH_DAEMON_NOT_LOGGED_IN,
	APPLE_AUTH_DAEMON_LOGGED_IN,
};

enum EAppleAuthStatus
{
	APPLE_AUTH_FAILURE,
	APPLE_AUTH_SUCCESS,
	APPLE_AUTH_NEED_2FA,
	APPLE_AUTH_NEED_SMS_2FA,
};

abstract_class IAppleAuth: public IAppSystem
{
public:
	virtual void LaunchLoginDaemon() = 0;
	virtual EAppleAuthDaemonStatus GetStatus() = 0;
	virtual EAppleAuthStatus SubmitLoginData( const char *szEmail, const char *szPassword ) = 0;
	virtual EAppleAuthStatus Submit2FA( const char *szCode ) = 0;
};

#define APPLE_AUTH_INTERFACE_VERSION "AppleAuth001"

#endif
