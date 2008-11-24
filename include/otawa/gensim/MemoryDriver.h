/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
 * 
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software 
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_MEMORY_DRIVER_H
#define OTAWA_MEMORY_DRIVER_H

#include <otawa/base.h>

namespace otawa {

namespace sim {

// External classes
class State;

// CacheDriver class
class MemoryDriver {
public:
	typedef enum {
		DATA_CACHE = 0,
		DATA_SPM = 1
	} location_t;

	virtual ~MemoryDriver(void);
	virtual location_t locate(address_t address) = 0;

	static MemoryDriver& ALWAYS_DATA_CACHE;
};

}
} // otawa::sim
#endif //OTAWA_MEMORY_DRIVER_H
