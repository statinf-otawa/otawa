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
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/stats/BBStatCollector.h>

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
extern Identifier<string> ILP_PLUGIN_NAME;

// Features
extern p::feature INTERBLOCK_SUPPORT_FEATURE;

extern p::feature BB_TIME_FEATURE;
extern Identifier<ot::time> TIME;
extern Identifier<ot::time> TIME_DELTA;

extern Identifier<int> COUNT;

extern p::feature ASSIGNED_VARS_FEATURE;
extern Identifier<ilp::Var *> VAR;

extern p::feature WCET_FEATURE;
extern Identifier<ot::time> WCET;

extern p::feature OBJECT_FUNCTION_FEATURE;

extern p::feature CONTROL_CONSTRAINTS_FEATURE;
extern Identifier<otawa::ilp::Constraint *> CALLING_CONSTRAINT;

extern p::feature FLOW_FACTS_CONSTRAINTS_FEATURE;

extern p::feature FLOW_FACTS_CONFLICT_CONSTRAINTS_FEATURE;  // conflict MDM

extern p::feature FLOW_FACTS_FEATURE;

extern p::id<bool> MAXIMIZE;
extern p::feature ILP_SYSTEM_FEATURE;
extern Identifier<ilp::System *> SYSTEM;

extern p::feature DATA_CACHE_SUPPORT_FEATURE;
extern p::feature INST_CACHE_SUPPORT_FEATURE;
extern p::feature CACHE_SUPPORT_FEATURE;

extern p::feature WCET_COUNT_RECORDED_FEATURE;

// statistics
// AbstractTotalTimeStat class
class AbstractTotalTimeStat: public BBStatCollector {
public:
	AbstractTotalTimeStat(WorkSpace *ws);

	cstring id() const override { return "ipet/total_time"; }
	cstring name() const override { return "Total Execution Time"; }
	cstring unit() const override { return "cycle"; }
	int total() override { return WCET(ws()); }
	cstring description() const override
		{ return "Represents the total number of cycles the code takes during the execution of the worst case execution path."; }
	void definitions(ListMap<cstring, string> & defs) const override;
	void keywords(Vector<cstring>& kws) { kws.add("time"); kws.add("block"); kws.add("cfg"); }
		
protected:
	ilp::System *system;
	p::id<ot::time>& LTS_TIME;
};
/*
class TimeStat: public AbstractTotalTimeStat {
public:
	TimeStat(WorkSpace *ws);
protected:
	virtual int getStat(BasicBlock *bb);
};
*/
class TimeStat: public BBStatCollector {
public:
	TimeStat(WorkSpace *ws);
	virtual cstring id(void) const;
	virtual void keywords(Vector<cstring>& kws);
	virtual cstring name(void) const;
	virtual cstring unit(void) const;
protected:
	virtual int getStat(BasicBlock *bb);
};


extern p::feature PIPELINE_TIME_FEATURE;
typedef Pair<ot::time, ilp::Var *> pipeline_time_t;
extern p::id<pipeline_time_t> PIPELINE_TIME;

} }	// otawa::ipet

#endif	// OTAWA_IPET_FEATURES_H
