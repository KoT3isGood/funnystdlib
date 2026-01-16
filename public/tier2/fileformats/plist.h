#ifndef TIER2_PLIST_H
#define TIER2_PLIST_H

#include "tier2/fileformats/json.h"

abstract_class IPropertyListManager
{
public:
	virtual IJSONObject *ReadString( const char *psz ) = 0;
};

IPropertyListManager *PropertyListManager();
#endif
