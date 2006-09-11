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

// Properties
extern GenericIdentifier<int> TIME;
extern GenericIdentifier<ilp::Var *> VAR;
extern GenericIdentifier<ilp::System *> SYSTEM;
extern GenericIdentifier<int> WCET;
extern GenericIdentifier<bool> EXPLICIT;
	
// Subprograms
ilp::System *getSystem(FrameWork *fw, CFG *cfg);
ilp::Var *getVar(ilp::System *system, BasicBlock *bb);
ilp::Var *getVar(ilp::System *system, Edge *edge);

} }	// otawa::ipet

#endif	// OTAWA_IPET_IPET_H
