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
class FrameWork;
class CFG;
class BasicBlock;
class Edge;
namespace ilp {
class System;
class Var;
}

namespace ipet {

// IPET class
class IPET {
public:

	// Constants
	static Identifier ID_Time;
	static Identifier ID_Var;
	static Identifier ID_System;
	static Identifier ID_WCET;
	static Identifier ID_Explicit;
	
	// Subprograms
	static ilp::System *getSystem(FrameWork *fw, CFG *cfg);
	static ilp::Var *getVar(ilp::System *system, BasicBlock *bb);
	static ilp::Var *getVar(ilp::System *system, Edge *edge);
};
		
} }	// otawa::ipet

#endif	// OTAWA_IPET_IPET_H
