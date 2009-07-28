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
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/ilp.h>

namespace otawa {

// External classes
class WorkSpace;
class CFG;
class BasicBlock;
class Edge;
namespace ilp {
class System;
class Var;
}

namespace ipet {

// Properties
extern Identifier<time_t> TIME;
extern Identifier<time_t> TIME_DELTA;
extern Identifier<ilp::Var *> VAR;
extern Identifier<time_t> WCET;
extern Identifier<bool> EXPLICIT;
extern Identifier<int> COUNT;
	
// Subprograms

// Feratures
extern Feature<NoProcessor> INTERBLOCK_SUPPORT_FEATURE;

} }	// otawa::ipet

#endif	// OTAWA_IPET_IPET_H
