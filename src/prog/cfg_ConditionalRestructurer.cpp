/*
 *	ConditionalRestructurer class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include <otawa/cfg/ConditionalRestructurer.h>
#include <otawa/prog/VirtualInst.h>

namespace otawa {

// set of blocks matching the block it is put on
static p::id<Bag<Block *> > BB_SET("");


/**
 * This feature ensures that the CFG is transformed to reflect the effects of conditional instructions.
 *
 * Default implementation:
 * @li @ref ConditionalRestructurer
 */
p::feature CONDITIONAL_RESTRUCTURED_FEATURE("otawa::CONDITIONAL_RESTRUCTURED_FEATURE", p::make<ConditionalRestructurer>());


/**
 */
p::declare ConditionalRestructurer::reg = p::init("otawa::ConditionalRestructurer", Version(1, 0, 0))
	.require(VIRTUAL_INST_FEATURE)
	.provide(CONDITIONAL_RESTRUCTURED_FEATURE)
	.extend<CFGTransformer>()
	.make<ConditionalRestructurer>();

typedef enum {
	NEVER = 0,
	MAYBE = 1,
	ALWAYS = 2
} effect_t;


/**
 * Apply a condition
 */
static effect_t apply(sem::cond_t state, sem::cond_t cond) {
	switch(state) {
	case sem::NO_COND:
		return MAYBE;

	case sem::EQ:
		switch(state) {
		case sem::EQ:		return ALWAYS;
		case sem::NE:		return NEVER;
		case sem::LT:		return NEVER;
		case sem::GT:		return NEVER;
		case sem::LE:		return MAYBE;
		case sem::GE:		return MAYBE;
		case sem::ANY_COND:	return MAYBE;
		}
		break;

	case sem::NE:
		switch(state) {
		case sem::EQ:		return NEVER;
		case sem::NE:		return ALWAYS;
		case sem::LT:		return MAYBE;
		case sem::GT:		return MAYBE;
		case sem::LE:		return MAYBE;
		case sem::GE:		return MAYBE;
		case sem::ANY_COND:	return MAYBE;
		}
		break;

	case sem::LT:
		switch(state) {
		case sem::EQ:		return NEVER;
		case sem::NE:		return MAYBE;
		case sem::LT:		return ALWAYS;
		case sem::GT:		return NEVER;
		case sem::LE:		return MAYBE;
		case sem::GE:		return NEVER;
		case sem::NO_COND:	return MAYBE;
		}
		break;
	case sem::LE:
		switch(state) {
		case sem::EQ:		return ALWAYS;
		case sem::NE:		return MAYBE;
		case sem::LT:		return MAYBE;
		case sem::GT:		return NEVER;
		case sem::LE:		return ALWAYS;
		case sem::GE:		return MAYBE;
		case sem::NO_COND:	return MAYBE;
		}
		break;
	case sem::GE:
	case sem::GT:
	case sem::ANY_COND:		break;
	}
}


/**
 * @class ConditionalRestructurer
 * This code processor transforms the CFG to reflect the effect of conditional instruction.
 * BB are duplicated according the conditional instructions it contains. The new BBS are made
 * of unconditional instructions, conditional instructions whose condition true and NOP
 * instructions to replace instructions which condition is false. When a condition becomes
 * true, the semantic of the instruction it applies to is prefixed with a semantic instruction
 * @ref sem::ASSUME to assert the condition.
 *
 * As a result, the obtained BBs reflects exactly the execution in the pipeline and the
 * semantic of the machine instructions.
 *
 * Used features:
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * Required features:
 * @li @ref VIRTUAL_INST_FEATURE
 *
 * Provided features:
 * @li @ref CONDITIONAL_RESTRUCTURED_FEATURE
 */

/**
 */
ConditionalRestructurer::ConditionalRestructurer(p::declare& r): CFGTransformer(r) {
}

Block *ConditionalRestructurer::transform(Block *b) {

}

Edge *ConditionalRestructurer::transform(Edge *e) {
	Block *src = e->source();
	Block *snk = e->sink();
	if(src->hasProp(BB_SET) || snk->hasProp(BB_SET)) {
		Bag<Block *> src_one(1, &src), snk_one(1, &snk);
		const Bag<Block *>& src_bag = src->hasProp(BB_SET) ? BB_SET(src) : src_one;
		const Bag<Block *>& snk_bag = snk->hasProp(BB_SET) ? BB_SET(snk) : snk_one;
		for(int i = 0; i < src_bag.count(); i++)
			for(int j = 0; j < snk_bag.count(); j++)
				clone(src_bag[i], e, snk_bag[j]);
		return 0;
	}
	else
		return CFGTransformer::transform(e);
}

} // otawa
