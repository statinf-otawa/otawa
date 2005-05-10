/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ets/ETS.h -- ETS class interface.
 */
 
#ifndef OTAWA_ETS_ETS_H
#define OTAWA_ETS_ETS_H

#include <otawa/prop/Identifier.h>

namespace otawa { namespace ets {
	
class ETS {
public:
	static Identifier ID_LOOP_COUNT;
	static Identifier ID_WCET;
};
		
} } // otawa::ets

#endif	// OTAWA_ETS_ETS_H
