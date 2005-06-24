/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/IPET.h -- IPET class interface.
 */
#ifndef OTAWA_IPET_IPET_H
#define OTAWA_IPET_IPET_H

#include <otawa/prop/Identifier.h>

namespace otawa {

// External classes
class BasicBlock;

// IPET class
class IPET {
public:
	static Identifier ID_Time;
	static Identifier ID_Var;
	static Identifier ID_System;
	static Identifier ID_WCET;
	static Identifier ID_Explicit;
};
		
} // otawa

#endif	// OTAWA_IPET_IPET_H
