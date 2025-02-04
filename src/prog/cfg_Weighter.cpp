/*
 *	Weighter class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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

#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg/features.h>
#include <otawa/ipet/features.h>
#include "../../include/otawa/flowfact/FlowFactLoader.h"

namespace otawa {

class Weighter: public BBProcessor {
public:
	static p::declare reg;
	Weighter(p::declare& r = reg): BBProcessor(r) { }

protected:
	void processBB(WorkSpace *ws, CFG *cfg, Block *bb) override {
		compute(bb);
		if(logFor(LOG_BLOCK))
			log << "\t\t\tweight = " << *WEIGHT(bb) << io::endl;
	}

	void cleanup(WorkSpace *ws) override {
		if(logFor(LOG_BLOCK))
			log << "\tweight for edges:\n";
		for(auto v: INVOLVED_CFGS(ws)->blocks())
			for(auto e: v->outEdges()) {
				WEIGHT(e) = min(*WEIGHT(v), *WEIGHT(e->sink()));
				if(logFor(LOG_BLOCK))
					log << "\t\t" << e << ": weight = " << WEIGHT(e) << io::endl;
			}
	}

	void destroy(WorkSpace *ws) override {
		for(auto v: INVOLVED_CFGS(ws)->blocks())
			WEIGHT(v).remove();
	}

private:

	void compute(Block *bb) {

		// already computed?
		if(bb->hasProp(WEIGHT))
			return;

		// if no parent, look in loop properties
		Block *parent = otawa::ENCLOSING_LOOP_HEADER(bb);
		if(!parent) {

			// not an header: weight is 1
			if(!LOOP_HEADER(bb))
				WEIGHT(bb) = 1;

			// header: look for max or total
			else {
				int total = TOTAL_ITERATION(bb);
				if(total >= 0)
					WEIGHT(bb) = total;
				else {
					int max = MAX_ITERATION(bb);
					if(max >= 0)
						WEIGHT(bb) = max;
					else
						WEIGHT(bb) = 1;
						// throw ProcessorException(*this, _ << "cannot compute weight for loop at " << bb << " -- " << bb->cfg()->name());
				}
			}

		}

		// else ask to parent
		else {
			compute(parent);

			// no header: simple weight propagation
			if(!LOOP_HEADER(bb))
				WEIGHT(bb) = *WEIGHT(parent);

			// header: multiply the weights
			else {
				int total = TOTAL_ITERATION(bb);
				if(total >= 0)
					WEIGHT(bb) = total;
				else {
					int max = MAX_ITERATION(bb);
					if(max >= 0)
						WEIGHT(bb) = max * WEIGHT(parent);
					else
						WEIGHT(bb) = 1;
						// throw ProcessorException(*this, _ << "cannot compute weight from parent for loop at " << workspace()->format(bb->toBasic()->address()));
				}
			}
		}
	}

};



p::declare Weighter::reg = p::init("otawa::Weighter", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<Weighter>()
	.require(LOOP_INFO_FEATURE)
	.require(ipet::FLOW_FACTS_FEATURE)
	.provide(WEIGHT_FEATURE);


/**
 * Get the maximum weight for the basic block it is hooked to.
 *
 * @par Feature
 * @li @ref otawa::WEIGHT_FEATURE
 *
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<int> WEIGHT("otawa::WEIGHT", 0);


/**
 * This feature ensures that weight information has been hooked to any
 * BB of the CFG. Weight information is coarse-grain estimation
 * of the number of execution of a basic block. It is simply derived
 * from the maximum and total number of iterations of each loop.
 *
 * @par Header
 * #include <otawa/cfg/features.h>
 *
 * @par Properties
 * @li @ref WEIGHT
 */
p::feature WEIGHT_FEATURE("otawa::WEIGHT_FEATURE", new Maker<Weighter>());

}	// otawa
