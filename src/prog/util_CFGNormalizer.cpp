/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CFGNormalizer class implementation
 */

#include <elm/assert.h>
#include <otawa/util/CFGNormalizer.h>
#include <otawa/cfg.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/proc/ProcessorException.h>

namespace otawa {

/**
 * @class CFGNormalizer
 * This processor check and transform the CFG to make it normalized, that is,
 * it performs the following checks and transformation:
 *
 * @li the entering edges in entry from other CFG are cut,
 * @li the leaving edges from exit to other CFG are cut,
 * @li the entering edge from dead code cause an error or are removed
 * (@ref CFGNormalizer::FORCE option set).
 *
 * @par Provided Feature
 * @li @ref NORMALIZED_CFGS_FEATURE
 *
 * @par Required Feature
 * @li @ref COLLECTED_CFG_FEATURE
 */


/**
 * May be passed in configuration with a boolean for for forcing or not the
 * normalization (default to false, avoid many useless normalization of the
 * same CFG).
 */
Identifier<bool>
	CFGNormalizer::FORCE("otawa::CFGNormalizer::FORCE", false);


/**
 * Configuration identifier for activating (boolean) or not the verbose node.
 * In verbose mode, each edge removal displays information about the action.
 */
Identifier<bool>
	CFGNormalizer::VERBOSE("otawa::CFGNormalizer::VERBOSE", false);


// Internal use
static Identifier<bool> IN_CFG("", false);


/**
 * Build a new CFG normalizer.
 */
CFGNormalizer::CFGNormalizer(void):
	CFGProcessor("otawa::CFGNormalizer", Version(1, 0, 0)),
	force(false),
	verbose(false)
{
	provide(NORMALIZED_CFGS_FEATURE);
	require(COLLECTED_CFG_FEATURE);
}


/**
 */
void CFGNormalizer::processCFG(WorkSpace *fw, CFG *cfg) {

	// Put marks
	elm::genstruct::Vector<Edge *> removes;
	for(CFG::BBIterator bb(cfg); bb; bb++)
		IN_CFG(bb) = true;

	// Examine BB
	for(CFG::BBIterator bb(cfg); bb; bb++) {

		// Look for an unresolved indirect branch
		if(bb->isTargetUnknown()) {
			bool solved = false;
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				if(edge->kind() != Edge::NOT_TAKEN) {
					solved = true;
					break;
				}
			if(!solved)
				throw ProcessorException(*this, _
					<< "unresolved indirect branch at " << bb->address());
		}

		// Record all entering edges
		for(BasicBlock::InIterator edge(bb); edge; edge++)
			if(edge->source() && !IN_CFG(edge->source()))
				removes.add(edge);

		// Remove entering edges
		for(elm::genstruct::Vector<Edge *>::Iterator edge(removes); edge; edge++) {
			if(!force)
				throw ProcessorException(*this, _ << "edge from dead code "
					<< edge->source()->address() << " to living code "
					<< edge->target()->address());
			else {
				if(verbose)
					warn(_ << "edge from dead code " << edge->source()->address()
						<< " to living code " << edge->target()->address()
						<< " removed");
				delete edge;
			}
		}
		removes.clear();
	}

	// Remove marks
	for(CFG::BBIterator bb(cfg); bb; bb++)
		bb->removeProp(&IN_CFG);
}


/**
 */
void CFGNormalizer::configure(const PropList& props) {
	CFGProcessor::configure(props);
	force = FORCE(props);
	verbose = VERBOSE(props);
}


/**
 * This feature ensures that the CFG are in a normalized form: fully resolved
 * branches, no entering or exiting edges to or from external CFGs.
 */
Feature<CFGNormalizer> NORMALIZED_CFGS_FEATURE("otawa::NORMALIZED_CFGS_FEATURE");

} // otawa
