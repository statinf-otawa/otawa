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

namespace otawa { namespace ipet {

/**
 * @class FlowFactLoader
 * This processor allows using extern flow facts in an IPET system.
 */


/**
 */
void FlowFactLoader::onError(const char *fmt, ...) {
	assert(fmt);
	va_list args;
	va_start(args, fmt);
	StringBuffer buffer;
	buffer.print(fmt, args);
	cout << buffer.toString();
	va_end(args);
	cout << '\n';
}


/**
 */
void FlowFactLoader::onLoop(address_t addr, int count) {
	assert(cfg);
	assert(system);
	assert(count >= 0);
	//cout << "LOOP " << count << " times at " << addr << "\n";
	
	// Process basic blocks
	bool found = false;
	for(CFG::BBIterator bb(cfg); bb; bb++)
		if(bb->address() == addr) {
			found = true;
			
			// Check domination
			if(!cfg->entry()->getProp(&Dominance::ID_RevDom)) {
				Dominance dom;
				dom.processCFG(0, cfg);
			}
			
			// Build the constraint
			//cout << "Added to " << *bb << "\n";
			otawa::ilp::Constraint *cons =
				system->newConstraint(otawa::ilp::Constraint::LE);
			for(BasicBlock::InIterator edge(bb); edge; edge++) {
				assert(edge->source());
				otawa::ilp::Var *var =
					edge->source()->use<otawa::ilp::Var *>(IPET::ID_Var);
				if(Dominance::dominates(bb, edge->source()))
					cons->addLeft(1, var);
				else
					cons->addRight(count, var);
			}
		}
	
	// Nothing found, seems too bad
	if(!found)
		out << "WARNING: loop " << addr << " not found.\n";
}


/**
 */
void FlowFactLoader::processCFG(FrameWork *fw, CFG *_cfg) {
	assert(fw);
	assert(_cfg);
	
	// Initialization
	cfg = _cfg;
	system = cfg->get<ilp::System *>(IPET::ID_System, 0);
	dom_done = false;
	
	// Get an ILP system
	if(!system) {
		system = fw->newILPSystem(true);
		cfg->set<ilp::System *>(IPET::ID_System, system);
		out << "INFO: ILP system created.\n";
	}
	
	// Launch load
	run(fw);
}
	
} } // otawa::ipet
