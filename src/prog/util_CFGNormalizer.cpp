/*
 *	CFGNormalizer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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

#include <elm/assert.h>
#include <elm/data/ListQueue.h>
#include <elm/data/Vector.h>
#include <otawa/cfg.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/proc/ProcessorException.h>
#include "../../include/otawa/cfg/CFGNormalizer.h"

namespace otawa {

/**
 * @class CFGNormalizer
 * This processor check and transform the CFG to make it normalized, that is,
 * without any dead-end. Some analyzes can fail if the CFG contains dead-end.
 *
 * @par Configuration
 * @li @ref CFGNormalizer::FORCE -- if a dead-end is found, stop the transformation.
 *
 * @par Provided Feature
 * @li @ref NORMALIZED_CFGS_FEATURE
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @par Required Feature
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @ingroup cfg
 */


/**
 * May be passed in configuration with a boolean for for forcing or not the
 * normalization (default to false, avoid many useless normalization of the
 * same CFG).
 */
Identifier<bool>
	CFGNormalizer::FORCE("otawa::CFGNormalizer::FORCE", false);


/**
 */
p::declare CFGNormalizer::reg = p::init("otawa::CFGNormalizer", Version(2, 0, 0))
	.provide(NORMALIZED_CFGS_FEATURE)
	.base(CFGTransformer::reg)
	.maker<CFGNormalizer>();


/**
 * Build a new CFG normalizer.
 */
CFGNormalizer::CFGNormalizer(p::declare& r): CFGTransformer(r), force(false) {
	setNoUnknown(true);
}


/**
 */
void CFGNormalizer::transform(CFG *cfg, CFGMaker& maker) {
	dfa::BitSet alive(cfg->count());

	// find dead-end blocks
	elm::ListQueue<Block *> wl;
	wl.put(cfg->exit());

	// propagate dead markers
	while(wl) {

		// next block
		Block *b = wl.get();
		alive.add(b->index());

		// propagate to predecessors
		for(Block::EdgeIter e = b->ins(); e(); e++)
			if(!alive.contains(e->source()->index()))
				wl.put(e->source());
	}

	// find block unreachable from exit in reversed CFG
	for(CFG::BlockIter b = cfg->blocks(); b(); b++)
		if(alive.contains(b->index()))
			for(Block::EdgeIter e = b->outs(); e(); e++)
				if(!alive.contains(e->sink()->index())) {
					if(!force)
						throw ProcessorException(*this, _ << "block " << e->sink() << " is the start of a dead-end. CFG will result in errors.");
					else
						if(logFor(LOG_FUN))
							warn(_ << "block " << e->sink() << " is the start of a dead-end. Dead-end removed.");
				}

	// clone alive blocks
	for(CFG::BlockIter b = cfg->blocks(); b(); b++) {
		if(!b->isEnd() && alive.contains(b->index()))
			CFGTransformer::transform(*b);
	}

	// clone alive edges
	for(CFG::BlockIter b = cfg->blocks(); b(); b++)
		if(alive.contains(b->index()))
			for(Block::EdgeIter e = b->outs(); e(); e++)
				if(alive.contains(e->sink()->index()))
					CFGTransformer::transform(*e);
}


/**
 */
void CFGNormalizer::configure(const PropList& props) {
	CFGTransformer::configure(props);
	force = FORCE(props);
}


/**
 * This feature ensures that the CFG are in a normalized form: fully resolved
 * branches, no entering or exiting edges to or from external CFGs.
 */
p::feature NORMALIZED_CFGS_FEATURE("otawa::NORMALIZED_CFGS_FEATURE", new Maker<CFGNormalizer>());

} // otawa
