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
#include <otawa/util/FlowFactLoader.h>
#include <otawa/ipet/features.h>

namespace otawa {

class Weighter: public BBProcessor {
public:
	static p::declare reg;
	Weighter(p::declare& r = reg): BBProcessor(r) { }

protected:
	virtual void processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
		compute(bb);
	}

private:

	void compute(BasicBlock *bb) {

		// already computed?
		if(bb->hasProp(WEIGHT))
			return;

		// if no parent, look in loop properties
		BasicBlock *parent = otawa::ENCLOSING_LOOP_HEADER(bb);
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
						throw ProcessorException(*this, _ << "cannot compute weight for loop at " << workspace()->format(bb->address()));
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
						throw ProcessorException(*this, _ << "cannot compute weight for loop at " << workspace()->format(bb->address()));
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
