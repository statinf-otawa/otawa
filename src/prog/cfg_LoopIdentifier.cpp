/*
 *	CFGIdentifier class
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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

#include <otawa/cfg/features.h>
#include <otawa/proc/CFGProcessor.h>
#include "../../include/otawa/graph/LoopIdentifier.h"

namespace otawa {

class LoopIdentifier: public CFGProcessor {
public:
	static p::declare reg;
	LoopIdentifier(void): CFGProcessor(reg) { }
protected:

	void processCFG(WorkSpace *ws, CFG *cfg) {
		graph::LoopIdentifier id(*cfg, cfg->entry());
		for(CFG::BlockIter b = cfg->blocks(); b(); b++)
			if(id.isHeader(*b)) {
				LOOP_HEADER(*b) = true;
				for(Block::EdgeIter e = b->ins(); e(); e++)
					if(id.isBack(*e))
						BACK_EDGE(*e) = true;
			}

	}
};

p::declare LoopIdentifier::reg = p::init("otawa::LoopIdentifier", Version(1, 0, 0))
	.extend<CFGProcessor>()
	.make<LoopIdentifier>()
	.provide(LOOP_HEADERS_FEATURE);

}	// otawa
