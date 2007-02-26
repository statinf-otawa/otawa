/*
 *	$Id$
 *	Copyright (c) 2007, IRIT-UPS <casse@irit.fr>.
 *
 *	CacheDriver class interface.
 */
#ifndef OTAWA_SIM_DRIVER_H
#define OTAWA_SIM_DRIVER_H

#include <otawa/base.h>

namespace otawa {

namespace sim {

// External classes
class State;

// CacheDriver class
class CacheDriver {
public :
	typedef enum_t {
		MISS = 0,
		HIT = 1
	} result_t;
	
	typedef enum_t {
		READ,
		WRITE
	} action_t;
	
	virtual ~CacheDriver(void); 
	virtual result_t // AlwaysHitDriver class
class AlwaysHitDriver: public CacheDriver {
public:
	virtual result_t access(address_t address, size_t size, action_t action);
};

// AlwaysMissDriver class
class AlwaysMissDriver: public CacheDriver {
public:
	virtual result_t access(address_t address, size_t size, action_t action);
};

	access(address_t address, size_t size, action_t action) = 0;
	
	static CacheDriver& ALWAYS_HIT;
	static CacheDriver& ALWAYS_MISS;
};

} } // otawa::sim

#endif	// OTAWA_SIM_DRIVER_H
