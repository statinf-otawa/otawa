/*
 *	$Id$
 *	Copyright (c) 2007, IRIT-UPS <casse@irit.fr>.
 *
 *	CacheDriver class interface.
 */
#ifndef OTAWA_SIM_CACHE_DRIVER_H
#define OTAWA_SIM_CACHE_DRIVER_H

#include <otawa/base.h>

namespace otawa {

namespace sim {

// External classes
class State;

// CacheDriver class
class CacheDriver {
public :
	typedef enum {
		MISS = 0,
		HIT = 1
	} result_t;
	
	typedef enum {
		READ,
		WRITE
	} action_t;
	
	virtual ~CacheDriver(void); 
	virtual result_t access(address_t address, size_t size, action_t action) = 0;
	
	static CacheDriver& ALWAYS_HIT;
	static CacheDriver& ALWAYS_MISS;
};

} } // otawa::sim

#endif	// OTAWA_SIM_CACHE_DRIVER_H
