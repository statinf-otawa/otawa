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

// Classes
class AbstractCacheState;

// Identifiers
extern NameSpace NS;
extern Identifier<int> LOOP_COUNT;
extern Identifier<int> WCET;
extern Identifier<AbstractCacheState *> ACS;
extern Identifier<int> HITS;
extern Identifier<int> MISSES;
extern Identifier<int> FIRST_MISSES;
extern Identifier<int> CONFLICTS;
		
} } // otawa::ets

#endif	// OTAWA_ETS_ETS_H
