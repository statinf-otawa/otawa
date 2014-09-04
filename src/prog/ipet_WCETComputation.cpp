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

using namespace otawa::ilp;

namespace otawa { namespace ipet {

// Registration
p::declare WCETComputation::reg = p::init("otawa::ipet::WCETComputation", Version(1, 0, 0))
	.require(CONTROL_CONSTRAINTS_FEATURE)
	.require(OBJECT_FUNCTION_FEATURE)
	.require(FLOW_FACTS_CONSTRAINTS_FEATURE)
	.provide(WCET_FEATURE)
	.maker<WCETComputation>();


/* BB time statistics collector */
class BBTimeCollector: public BBStatCollector {
public:
	BBTimeCollector(WorkSpace *ws): BBStatCollector(ws) {
		system = SYSTEM(ws);
		ASSERT(system);
	}

	virtual cstring name(void) const { return "Execution Time"; }
	virtual cstring unit(void) const { return "cycle"; }
	virtual bool isEnum(void) const { return false; }
	virtual const cstring valueName(int value) { return ""; }
	virtual int total(void) { return WCET(ws()); }
	virtual int mergeContext(int v1, int v2) { return max(v1, v2); }
	virtual int mergeAgreg(int v1, int v2) { return v1 + v2; }

	void collect(Collector& collector, BasicBlock *bb) {
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
		collector.collect(bb->address(), bb->size(), cnt * time);
	}

private:
	ilp::System *system;
};


/**
 * @class WCETComputation
 * This class is used for computing the WCET from the system found in the root
 * CFG.
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
WCETComputation::WCETComputation(void): Processor(reg) {
}


/**
 */
void WCETComputation::processWorkSpace(WorkSpace *fw) {
	System *system = SYSTEM(fw);
	if(!system)
		throw ProcessorException(*this, "no ILP system defined in this CFG");
	ot::time wcet = -1;
	if(logFor(LOG_DEPS))
		log << "\tlaunching ILP solver\n";
	if(system->solve(fw)) {
		if(logFor(LOG_DEPS))
			log << "\tobjective function = " << system->value() << io::endl;
		wcet = ot::time(system->value());
	}
	if(logFor(LOG_DEPS))
		log << "\tWCET = " << wcet << io::endl;
	WCET(fw) = wcet;
}


/**
 */
void WCETComputation::collectStats(WorkSpace *ws) {
	recordStat(WCET_FEATURE, new BBTimeCollector(ws));
}


static SilentFeature::Maker<WCETComputation> maker;
/**
 * This feature ensures that the WCET has been computed using IPET approach.
 *
 * @par Properties
 * @li @ref otawa::ipet::WCET (FrameWork)
 */
SilentFeature WCET_FEATURE("otawa::ipet::WCET_FEATURE", maker);

} } // otawa::ipet
