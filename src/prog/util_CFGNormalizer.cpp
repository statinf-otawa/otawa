/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/CFGNormalizer.cpp -- CFGNormalizer class implementation.
 */

#include <assert.h>
#include <otawa/util/CFGNormalizer.h>
#include <otawa/cfg.h>
#include <elm/genstruct/Vector.h>

namespace otawa {

/**
 * @class CFGNormalizer
 * This processor transform the CFG to making it normalized, that is, it performs
 * the following transformation:
 * <ul>
 * <li>the entering edges in entry from other CFG are cut,</li>
 * <li>the leaving edges from exit to other CFG are cut,</li>
 * <li>the entering edge from dead code are removed (GCC with middle-function
 * return instruction).</li>
 * </ul>
 */


/**
 * May be passed in configuration with a boolean for for forcing or not the
 * normalization (default to false, avoid many useless normalization of the
 * same CFG).
 */
Identifier CFGNormalizer::ID_Force("otawa.cfg_normalizer.force");


/**
 * Put on the CFG after the normalization has been performed.
 */
Identifier CFGNormalizer::ID_Done("otawa.cfg_normalizer.done");


/**
 * Configuration identifier for activating (boolean) or not the verbose node.
 * In verbose mode, each edge removal displays information about the action.
 */
Identifier CFGNormalizer::ID_Verbose("otawa.cfg_normalizer.verbose");


// Internal use
Identifier ID_InCFG("otawa.cfg_normalier.done");


/**
 * Build a new CFG normalizer.
 */
CFGNormalizer::CFGNormalizer(void): force(false), verbose(false) {
}


/**
 */
void CFGNormalizer::processCFG(FrameWork *fw, CFG *cfg) {
	
	// Check if work need to be done
	if(!force && cfg->get<bool>(ID_Done, false))
		return;
	
	// Check entry
	
	// Check exit
	
	// Check dead code entering edges
	elm::genstruct::Vector<Edge *> removes;
	for(CFG::BBIterator bb(cfg); bb; bb++)
		bb->add<bool>(ID_InCFG, true);
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		for(Iterator<Edge *> edge(bb->inEdges()); edge; edge++)
			if(edge->source() && !edge->source()->getProp(&ID_InCFG))
				removes.add(edge);
		for(elm::genstruct::Vector<Edge *>::Iterator edge(removes); edge; edge++) {
			if(verbose)
				out << "WARNING: edge from " << edge->source()->address()
					<< " to " << edge->target()->address() << " removed.\n";
			delete edge;
		}
		removes.clear();
	}
	for(CFG::BBIterator bb(cfg); bb; bb++)
		bb->removeProp(&ID_InCFG);
	
	// Mark as done
	cfg->set<bool>(ID_Done, true);
}


/**
 */
void CFGNormalizer::configure(PropList& props) {
	CFGProcessor::configure(props);
	force = props.get<bool>(ID_Force, false);
	verbose = props.get<bool>(ID_Verbose, false);
}

} // otawa
