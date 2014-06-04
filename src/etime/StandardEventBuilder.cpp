/*
 *	StandardEventBuilder class implementation
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

#include <otawa/etime/StandardEventBuilder.h>
#include <otawa/hard/Processor.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cache/cat2/features.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hard/Memory.h>

namespace otawa { namespace etime {


class MissEvent: public Event {
public:
	MissEvent(Inst *inst, ot::time cost, category_t cat, ilp::Var *var)
		: Event(inst), _cat(cat), _cost(cost), _var(var) {	}

	virtual kind_t kind(void) const { return FETCH; }
	virtual ot::time cost(void) const { return _cost; }

	virtual occurrence_t occurrence(void) const {
		switch(_cat) {
		case ALWAYS_HIT:		return NEVER;
		case FIRST_HIT:			return SOMETIMES;
		case FIRST_MISS:		return SOMETIMES;
		case ALWAYS_MISS:		return ALWAYS;
		case NOT_CLASSIFIED:	return SOMETIMES;
		default:				ASSERT(0); return SOMETIMES;
		}
	}

	virtual cstring name(void) const { return "L1 instruction cache"; }

	virtual bool isOverestimating(bool on) {
		return on;	// only when on = true!
	}

	virtual void overestimate(ilp::Constraint *cons, bool on) {
		if(on)
			cons->addLeft(1, _var);
	}

private:
	category_t _cat;
	ilp::Var *_var;
	ot::time _cost;
};


/**
 * @class StandardEventBuilder
 * Build standard events.
 *
 * @par Provided Features
 * @li @ref STANDARD_EVENTS_FEATURE
 *
 * @par Required Features
 * @li @ref otawa::COLLECTED_CFG_FEATURE
 * @li @ref otawa::hard::PROCESSOR_FEATURE
 * @li @ref otawa::hard::MEMORY_FEATURE
 *
 * @par Optional
 * @li @ref ICACHE_CONSTRAINT2_FEATURE (to get L1 instruction cache events)
 *
 * @ingroup etime
 */


/**
 */
StandardEventBuilder::StandardEventBuilder(p::declare& r): BBProcessor(r), proc(0), mem(0) {
}


/**
 */
void StandardEventBuilder::setup(WorkSpace *ws) {

	// look for processor
	proc = hard::PROCESSOR(ws);
	ASSERT(proc);
	if(logFor(Processor::LOG_DEPS))
		log << "\tprocessor = " << proc->getModel() << " (" << proc->getBuilder() << ")\n";

	// look for memory
	mem = hard::MEMORY(ws);
	ASSERT(mem);
	bank = mem->banks()[0];

	// look if instruction cacheL1 is available
	has_il1 = ws->isProvided(ICACHE_ONLY_CONSTRAINT2_FEATURE);
}


/**
 */
void StandardEventBuilder::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	if(bb->isEnd())
		return;

	// process instruction stage L1
	if(has_il1) {
		const genstruct::AllocatedTable<LBlock* >& blocks = **BB_LBLOCKS(bb);
		BasicBlock::InstIter inst(bb);
		for(int i = 0; i < blocks.count(); i++) {

			// find the instruction
			while(inst->address() != blocks[i]->address()) {
				inst++;
				ASSERT(inst);
			}

			// find the bank
			if(!bank->contains(blocks[i]->address())) {
				const hard::Bank *bank = mem->get(blocks[i]->address());
				if(!bank)
					throw ProcessorException(*this, _ << "no bank contains instruction at " << blocks[i]->address());
			}

			// create the event
			Event *event = new MissEvent(*inst, bank->latency(), CATEGORY(blocks[i]), MISS_VAR(blocks[i]));
			EVENT(bb).add(event);
		}
	}
}

p::declare StandardEventBuilder::reg = p::init("otawa::etime::StandardEventBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<StandardEventBuilder>()
	.provide(STANDARD_EVENTS_FEATURE)
	.require(hard::PROCESSOR_FEATURE)
	.require(hard::MEMORY_FEATURE);


} } // otawa::etime
