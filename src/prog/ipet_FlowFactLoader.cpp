/*
 * $Id$
 * Copyright (c) 2005, IRIT-UPS <casse@irit.fr>
 *
 * src/prog/ipet_IPETFlowFactLoader.h -- IPETFlowFactLoader class implementation.
 */

#include <otawa/cfg.h>
#include <elm/io.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/ipet/IPET.h>
#include <otawa/util/Dominance.h>
#include <otawa/ilp.h>
#include <otawa/proc/ProcessorException.h>

namespace otawa { namespace ipet {

/**
 * @class FlowFactLoader
 * This processor allows using extern flow facts in an IPET system.
 */


/**
 * Build a new flow fact loader.
 * @param props		Configuration properties.
 */
FlowFactLoader::FlowFactLoader(const PropList& props)
: Processor("otawa::ipet::FlowFactLoader", Version(1, 0, 0), props) {
}


/**
 */
void FlowFactLoader::onError(const char *fmt, ...) {
	assert(fmt);
	VARARG_BEGIN(args, fmt)
		throw ProcessorException(*this, fmt, args);
	VARARG_END
}


/**
 */
void FlowFactLoader::onLoop(address_t addr, int count) {
	assert(system);
	assert(count >= 0);
	//cout << "LOOP " << count << " times at " << addr << "\n";

	// Process basic blocks
	bool found = false;
	for(CFGCollection::Iterator cfg(cfgs); cfg; cfg++) {

		// check domination
		Dominance::ensure(cfg);
	
		// Look BB in the CFG
		for(CFG::BBIterator bb(cfg); bb; bb++)
			if(bb->address() == addr /*&& Dominance::isLoopHeader(bb)*/) {
			
				// Build the constraint
				//cout << "Added to " << *bb << "\n";
				otawa::ilp::Constraint *cons =
					system->newConstraint(otawa::ilp::Constraint::LE);
				cons->addLeft(1, bb->use<otawa::ilp::Var *>(VAR));
				for(BasicBlock::InIterator edge(bb); edge; edge++) {
					assert(edge->source());
					otawa::ilp::Var *var =
						edge->source()->use<otawa::ilp::Var *>(VAR);
					if(!Dominance::dominates(bb, edge->source()))
						cons->addRight(count, var);
				}
				return;
			}
	}
	
	// Nothing found, seems too bad
	out << "WARNING: loop " << addr << " not found.\n";
}


/**
 */
void FlowFactLoader::processFrameWork(FrameWork *fw) {
	assert(fw);
	cfgs = INVOLVED_CFGS(fw);
	assert(cfgs);
	system = getSystem(fw, ENTRY_CFG(fw));
	dom_done = false;
	run(fw);
}
	
} } // otawa::ipet
