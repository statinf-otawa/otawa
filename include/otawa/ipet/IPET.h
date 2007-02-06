/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/IPET.h -- IPET class interface.
 */
#ifndef OTAWA_IPET_IPET_H
#define OTAWA_IPET_IPET_H

#include <otawa/prop/Identifier.h>
#include <otawa/proc/Feature.h>

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

// Namespaces
extern NameSpace NS;

// Properties
extern Identifier<int> TIME;
extern Identifier<int> TIME_DELTA;
extern Identifier<ilp::Var *> VAR;
extern Identifier<ilp::System *> SYSTEM;
extern Identifier<int> WCET;
extern Identifier<bool> EXPLICIT;
extern Identifier<int> LOOP_COUNT;
extern Identifier<int> COUNT;
	
// Subprograms
ilp::System *getSystem(FrameWork *fw, CFG *cfg);
ilp::Var *getVar(ilp::System *system, BasicBlock *bb);
ilp::Var *getVar(ilp::System *system, Edge *edge);

// Feratures
extern Feature<NoProcessor> INTERBLOCK_SUPPORT_FEATURE;

} }	// otawa::ipet

#endif	// OTAWA_IPET_IPET_H
