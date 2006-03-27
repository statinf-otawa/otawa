/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/IPET.h -- IPET class interface.
 */
#ifndef OTAWA_IPET_IPET_H
#define OTAWA_IPET_IPET_H

#include <otawa/prop/GenericIdentifier.h>

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
	static GenericIdentifier<int> ID_Time;
	static GenericIdentifier<ilp::Var *> ID_Var;
	static GenericIdentifier<ilp::System *> ID_System;
	static GenericIdentifier<int> ID_WCET;
	static GenericIdentifier<bool> ID_Explicit;
	
	// Subprograms
	static ilp::System *getSystem(FrameWork *fw, CFG *cfg);
	static ilp::Var *getVar(ilp::System *system, BasicBlock *bb);
	static ilp::Var *getVar(ilp::System *system, Edge *edge);
};
		
} }	// otawa::ipet

#endif	// OTAWA_IPET_IPET_H
