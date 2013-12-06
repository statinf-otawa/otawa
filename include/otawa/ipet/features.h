/*
 *	Features for the IPET module.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-13, IRIT UPS.
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
#ifndef OTAWA_IPET_FEATURES_H
#define OTAWA_IPET_FEATURES_H

#include <otawa/prop/Identifier.h>
#include <otawa/proc/Feature.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

// External classes
class WorkSpace;
class CFG;
class BasicBlock;
class Edge;
namespace ilp {
	class Constraint;
	class System;
	class Var;
}

namespace ipet {

// Common configuration
extern Identifier<bool> EXPLICIT;
extern Identifier<cstring> ILP_PLUGIN_NAME;

// Features
extern Feature<NoProcessor> INTERBLOCK_SUPPORT_FEATURE;

extern SilentFeature BB_TIME_FEATURE;
extern Identifier<ot::time> TIME;
extern Identifier<ot::time> TIME_DELTA;

extern Identifier<int> COUNT;

extern SilentFeature ASSIGNED_VARS_FEATURE;
extern Identifier<ilp::Var *> VAR;

extern SilentFeature WCET_FEATURE;
extern Identifier<ot::time> WCET;

extern SilentFeature OBJECT_FUNCTION_FEATURE;

extern SilentFeature CONTROL_CONSTRAINTS_FEATURE;
extern Identifier<otawa::ilp::Constraint *> CALLING_CONSTRAINT;

extern SilentFeature FLOW_FACTS_CONSTRAINTS_FEATURE;

extern SilentFeature FLOW_FACTS_FEATURE;

extern SilentFeature ILP_SYSTEM_FEATURE;
extern Identifier<ilp::System *> SYSTEM;

extern SilentFeature DATA_CACHE_SUPPORT_FEATURE;

extern SilentFeature INST_CACHE_SUPPORT_FEATURE;

extern p::feature WCET_COUNT_RECORDED_FEATURE;

} }	// otawa::ipet

#endif	// OTAWA_IPET_FEATURES_H
