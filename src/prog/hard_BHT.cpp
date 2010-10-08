/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/BHT.cpp -- implementation of BHT class.
 */

#include <stdarg.h>
#include <otawa/prop/Identifier.h>
#include <otawa/hard/BHT.h>

using namespace elm;

namespace otawa { namespace hard {

Identifier<BHT*> BHT_CONFIG("otawa::hard::BHT_CONFIG", NULL);

} } // otawa::hard

// Serialization support
SERIALIZE(otawa::hard::BHT);

