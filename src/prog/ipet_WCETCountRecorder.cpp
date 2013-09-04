/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	WCETCountRecorder class implementation.
 */

#include <assert.h>
#include <otawa/ipet/WCETCountRecorder.h>
#include <otawa/ipet/WCETComputation.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/cfg/Edge.h>
#include <otawa/ipet/ILPSystemGetter.h>

namespace otawa { namespace ipet {

/**
 * @class WCETCountRecorder
 * This class may be used to record back in the CFG the execution count of each
 * basic block and of each edge for the WCET path.
 *
 * @par Provided Features
 * @li @ref WCET_COUNT_RECORDED_FEATURE
 *
 * @par Required Features
 * @li @ref WCET_FEATURE
 */


/**
 */
WCETCountRecorder::WCETCountRecorder(void)
:	BBProcessor("otawa::ipet::WCETCountRecorder", Version(1, 0,0)),
	system(0)
{
	require(WCET_FEATURE);
	provide(WCET_COUNT_RECORDED_FEATURE);
}


/**
 */
void WCETCountRecorder::setup(WorkSpace *fw) {
	assert(fw);
	assert(!system);
	system = SYSTEM(fw);
	assert(system);
}


/**
 */
void WCETCountRecorder::cleanup(WorkSpace *fw) {
	assert(fw);
	assert(system);
	system = 0;
}


/**
 */
void WCETCountRecorder::processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb) {
	assert(fw);
	assert(cfg);
	assert(bb);

	// Record BB var count
	ilp::Var *var = VAR(bb);
	if(var)
		COUNT(bb) = (int)system->valueOf(var);

	// Record out var count
	for(BasicBlock::OutIterator edge(bb); edge; edge++) {
		ilp::Var *var = VAR(edge);
		if(var)
			COUNT(edge) = (int)system->valueOf(var);
	}
}


/**
 * This feature asserts that WCET execution count of basic block and of edge
 * have been recorded.
 *
 * @par Properties
 * @li @ref ipet::COUNT
 */
p::feature WCET_COUNT_RECORDED_FEATURE("otawa::ipet::WCET_COUNT_RECORDED_FEATURE", new Maker<WCETCountRecorder>());

} } // otawa::ipet
