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
Identifier<bool>
	CFGNormalizer::FORCE("CFGNormalizer::force", false, otawa::NS);


/**
 * Put on the CFG after the normalization has been performed.
 */
Identifier<bool>
	CFGNormalizer::DONE("CFGNormalizer::done", false, otawa::NS);


/**
 * Configuration identifier for activating (boolean) or not the verbose node.
 * In verbose mode, each edge removal displays information about the action.
 */
Identifier<bool>
	CFGNormalizer::VERBOSE("CFGNormalizer::verbose", false, otawa::NS);


// Internal use
static Identifier<bool> IN_CFG("in_cfg", false, otawa::NS);


/**
 * Build a new CFG normalizer.
 */
CFGNormalizer::CFGNormalizer(void): force(false), verbose(false) {
}


/**
 */
void CFGNormalizer::processCFG(FrameWork *fw, CFG *cfg) {
	
	// Check if work need to be done
	if(!force && DONE(cfg))
		return;
	
	// Check entry
	
	// Check exit
	
	// Check dead code entering edges
	elm::genstruct::Vector<Edge *> removes;
	for(CFG::BBIterator bb(cfg); bb; bb++)
		IN_CFG(bb) = true;
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		for(BasicBlock::InIterator edge(bb); edge; edge++)
			if(edge->source() && !IN_CFG(edge->source()))
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
		bb->removeProp(&IN_CFG);
	
	// Mark as done
	DONE(cfg) = true;
}


/**
 */
void CFGNormalizer::configure(PropList& props) {
	CFGProcessor::configure(props);
	force = FORCE(props);
	verbose = VERBOSE(props);
}

} // otawa
