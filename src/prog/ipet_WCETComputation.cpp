/*
 *	$Id$
 *	WCETComputation class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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

#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/WCETComputation.h>
#include <otawa/cfg/CFG.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>
#include <otawa/ipet/FlowFactConstraintBuilder.h>
#include <otawa/proc/Registry.h>
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/stats/BBStatCollector.h>
#include <otawa/stats/StatInfo.h>
#include <otawa/ilp/ILPPlugin.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

// Registration
p::declare WCETComputation::reg = p::init("otawa::ipet::WCETComputation", Version(1, 1, 0))
	.require(CONTROL_CONSTRAINTS_FEATURE)
	.require(OBJECT_FUNCTION_FEATURE)
	.require(FLOW_FACTS_CONSTRAINTS_FEATURE)
	.provide(WCET_FEATURE)
	.maker<WCETComputation>();


/**
 * Configuration item for WCETComputation. Default to false, if set to true,
 * the processor will output the WCET at the end of computation.
 */
p::id<bool> WCETComputation::DO_DISPLAY("otawa::ipet::WCETComputation::DO_DISPLAY", false);


// total time statistics
class TotalTimeStat: public BBStatCollector {
public:
	TotalTimeStat(WorkSpace *ws): BBStatCollector(ws) {
		system = SYSTEM(ws);
		ASSERT(system);
	}

	virtual cstring id(void) const { return "ipet/total_time"; }
	virtual cstring name(void) const { return "Total Execution Time"; }
	virtual cstring unit(void) const { return "cycle"; }
	virtual int total(void) { return WCET(ws()); }

	void collect(Collector& collector, BasicBlock *bb, const ContextualPath& path) {
		if(bb->isEnd())
			return;
		ot::time time = TIME(bb);
		if(time < 0)
			return;
		ilp::Var *var = VAR(bb);
		if(!var)
			return;
		int cnt = int(system->valueOf(var));
		if(cnt < 0)
			return;
		collector.collect(bb->address(), bb->size(), cnt * time, path);
	}

private:
	ilp::System *system;
};


// total block execution count
class TotalCountStat: public BBStatCollector {
public:
	TotalCountStat(WorkSpace *ws): BBStatCollector(ws) {
		system = SYSTEM(ws);
		ASSERT(system);
	}

	virtual cstring id(void) const { return "ipet/total_count"; }
	virtual cstring name(void) const { return "Total Execution Count"; }

	void collect(Collector& collector, BasicBlock *bb, const ContextualPath& path) {
		if(bb->isEnd())
			return;
		ilp::Var *var = VAR(bb);
		if(!var)
			return;
		int cnt = int(system->valueOf(var));
		if(cnt < 0)
			return;
		collector.collect(bb->address(), bb->size(), cnt, path);
	}

private:
	ilp::System *system;
};


/**
 * @class WCETComputation
 * This class is used for computing the WCET from the system found in the root
 * CFG.
 *
 * @par Configuration
 * @li @ref WCETComputation::DO_DISPLAY
 *
 * @par Required Features
 * @li @ref ipet::CONTROL_CONSTRAINTS_FEATURE
 * @li @ref ipet::OBJECT_FUNCTION_FEATURE
 * @li @ref ipet::FLOW_FACTS_CONSTRAINTS_FEATURE
 *
 * @par Provided Features
 * @li @ref ipet::WCET_FEATURE
 *
 * @par Statistics
 * @li BB time
 */


/**
 * Build a new WCET computer.
 */
WCETComputation::WCETComputation(void): Processor(reg), system(0), do_display(false) {
}


/**
 */
void WCETComputation::configure(const PropList& props) {
	Processor::configure(props);
	do_display = DO_DISPLAY(props);
}


/**
 */
void WCETComputation::processWorkSpace(WorkSpace *ws) {
	ASSERT(ws);
	System *system = SYSTEM(ws);
	ASSERT(system);
	ot::time wcet = -1;
	if(logFor(LOG_FILE)) {
		string name = "unknown";
		ilp::ILPPlugin *p = system->plugin();
		if(p)
			name = p->name();
		log << "\tlaunching ILP solver: " << name << io::endl;
	}
	if(system->solve(ws, *this)) {
		if(logFor(LOG_FILE))
			log << "\tobjective function = " << system->value() << io::endl;
		wcet = ot::time(system->value());
		if(logFor(LOG_FILE))
			log << "\tWCET = " << wcet << io::endl;
	}
	else
		log << "ERROR: " << system->lastErrorMessage() << io::endl;
	WCET(ws) = wcet;
	if(do_display)
		cout << "wcet[" << otawa::ENTRY_CFG(ws)->label() << "] = " << wcet << " cycles\n";
}


/**
 */
void WCETComputation::collectStats(WorkSpace *ws) {
	record(new TotalTimeStat(ws));
	record(new TotalCountStat(ws));
}


/**
 * @class TimeStat
 * Statistics producing the execution of each basic block.
 * @ingroup ipet
 */

/** */
TimeStat::TimeStat(WorkSpace *ws): BBStatCollector(ws) { }

/** */
cstring TimeStat::id(void) const { return "ipet/time"; }

/** */
void TimeStat::keywords(Vector<cstring>& kws) { kws.add("time"); kws.add("block"); kws.add("cfg"); }

/** */
cstring TimeStat::name(void) const { return "Block Execution Time"; }

/** */
cstring TimeStat::unit(void) const { return "cycle"; }

/** */
int TimeStat::getStat(BasicBlock *bb) { return TIME(bb); }


/**
 * This feature ensures that the WCET has been computed using IPET approach.
 *
 * @par Properties
 * @li @ref otawa::ipet::WCET (FrameWork)
 */
p::feature WCET_FEATURE("otawa::ipet::WCET_FEATURE", new Maker<WCETComputation>());

} } // otawa::ipet