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
#include <otawa/hard/BHT.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/dcache/features.h>

namespace otawa { namespace etime {


class MissEvent: public Event {
public:
	MissEvent(Inst *inst, ot::time cost, LBlock *lb)
		: Event(inst), _cost(cost), _lb(lb) {	}

	virtual kind_t kind(void) const { return FETCH; }
	virtual ot::time cost(void) const { return _cost; }
	virtual type_t type(void) const { return BLOCK; }

	virtual occurrence_t occurrence(void) const {
		switch(otawa::CATEGORY(_lb)) {
		case ALWAYS_HIT:		return NEVER;
		case FIRST_HIT:			return SOMETIMES;
		case FIRST_MISS:		return SOMETIMES;
		case ALWAYS_MISS:		return ALWAYS;
		case NOT_CLASSIFIED:	return SOMETIMES;
		default:				ASSERT(0); return SOMETIMES;
		}
	}

	virtual cstring name(void) const { return "L1 instruction cache"; }

	virtual string detail(void) const { return _ << *otawa::CATEGORY(_lb); }

	virtual int weight(void) const {
		switch(otawa::CATEGORY(_lb)) {
		case ALWAYS_HIT:		return 0;
		case FIRST_HIT:
		case ALWAYS_MISS:
		case NOT_CLASSIFIED:	return WEIGHT(_lb->bb());
		case FIRST_MISS:		{
									BasicBlock *parent = otawa::ENCLOSING_LOOP_HEADER(_lb);
									if(!parent)
										return 1;
									else
										return WEIGHT(parent);
								}
		default:				ASSERT(0); return SOMETIMES;
		}
	}

	virtual bool isEstimating(bool on) {
		return on;	// only when on = true!
	}

	virtual void estimate(ilp::Constraint *cons, bool on) {
		if(on)
			cons->addLeft(1, otawa::MISS_VAR(_lb));
	}

private:
	LBlock *_lb;
	ot::time _cost;
};


class BranchPredictionEvent: public Event {
public:
	BranchPredictionEvent(Inst *inst, ot::time cost, occurrence_t occ, ilp::Var *var, BasicBlock *wbb)
		: Event(inst), _var(var), _cost(cost), _occ(occ), _wbb(wbb) { }

	virtual kind_t kind(void) const { return BRANCH; }
	virtual ot::time cost(void) const { return _cost; }
	virtual type_t type(void) const { return EDGE; }
	virtual occurrence_t occurence(void) const { return _occ; }
	virtual cstring name(void) const { return "branch prediction"; }
	virtual string detail(void) const { return ""; }
	virtual bool isEstimating(bool on) { return on; }

	virtual void estimate(ilp::Constraint *cons, bool on) {
		if(on)
			cons->addLeft(1, _var);
	}

	virtual int weight(void) const {
		if(!_wbb)
			return 1;
		else
			return WEIGHT(_wbb);
	}

private:
	ilp::Var *_var;
	ot::time _cost;
	occurrence_t _occ;
	BasicBlock *_wbb;
};


class DataMissEvent: public Event {
public:
	DataMissEvent(Inst *inst, ot::time cost, dcache::BlockAccess& acc, BasicBlock *bb): Event(inst), _cost(cost), _acc(acc), _bb(bb) { }

	virtual kind_t kind(void) const { return MEM; }
	virtual ot::time cost(void) const { return _cost; }
	virtual type_t type(void) const { return BLOCK; }

	virtual occurrence_t occurrence(void) const {
		switch(dcache::CATEGORY(_acc)) {
		case cache::ALWAYS_HIT:		return NEVER;
		case cache::FIRST_HIT:		return SOMETIMES;
		case cache::FIRST_MISS:		return SOMETIMES;
		case cache::ALWAYS_MISS:	return ALWAYS;
		case cache::NOT_CLASSIFIED:	return SOMETIMES;
		default:					ASSERT(0); return SOMETIMES;
		}
	}

	virtual cstring name(void) const { return "L1 data cache"; }
	virtual string detail(void) const { return _ << *dcache::CATEGORY(_acc); }

	virtual int weight(void) const {
		switch(dcache::CATEGORY(_acc)) {
		case cache::ALWAYS_HIT:		return 0;
		case cache::FIRST_HIT:
		case cache::ALWAYS_MISS:
		case cache::NOT_CLASSIFIED:	return WEIGHT(_bb);
		case cache::FIRST_MISS:		{
										BasicBlock *parent = otawa::ENCLOSING_LOOP_HEADER(_bb);
										if(!parent)
											return 1;
										else
											return WEIGHT(parent);
									}
		default:					ASSERT(0); return 0;
		}
	}

	virtual bool isEstimating(bool on) {
		return on;	// only when on = true!
	}

	virtual void estimate(ilp::Constraint *cons, bool on) {
		if(on)
			cons->addLeft(1, dcache::MISS_VAR(_bb));
	}

private:
	ot::time _cost;
	dcache::BlockAccess& _acc;
	BasicBlock *_bb;
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
 * @li @ref otawa::ICACHE_CONSTRAINT2_FEATURE (to get L1 instruction cache events)
 * @li @ref otawa::branch::CONSTRAINTS_FEATURE
 *
 * @ingroup etime
 */


/**
 */
StandardEventBuilder::StandardEventBuilder(p::declare& r)
: BBProcessor(r), mem(0), has_il1(false), has_dl1(false), has_branch(false), cconf(0), bht(0), bank(0), _explicit(false) {
}


void StandardEventBuilder::configure(const PropList& props) {
	BBProcessor::configure(props);
	_explicit = ipet::EXPLICIT(props);
}


/**
 */
void StandardEventBuilder::setup(WorkSpace *ws) {

	// look for memory
	mem = hard::MEMORY(ws);
	if(mem)
		bank = mem->banks()[0];
	else
		bank = 0;

	// look if instruction cacheL1 is available
	has_il1 = ws->isProvided(ICACHE_ONLY_CONSTRAINT2_FEATURE);
	has_dl1 = ws->isProvided(dcache::CONSTRAINTS_FEATURE);

	// look if branch prediction is available
	has_branch = ws->isProvided(branch::CONSTRAINTS_FEATURE);
	if(has_branch) {
		bht = hard::BHT_CONFIG(ws);
		ASSERT(bht);
	}
}


/**
 * Compute cost of a memory access.
 * @param addr	Accessed address.
 * @param write	Write operation if true, false else.
 */
ot::time StandardEventBuilder::costOf(Address addr, bool write) {
	if(!bank || !bank->contains(addr)) {
		const hard::Bank *new_bank = mem->get(addr);
		if(!new_bank)
			return 0;
		bank = new_bank;
	}
	return write ? bank->writeLatency() : bank->latency();
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
			ot::time cost = costOf(blocks[i]->address());
			if(!cost) {
				warn(_ << "no bank contains instruction at " << inst->address() << ". Assuming worst latency.");
				cost = mem->worstReadAccess();
			}

			// create the event
			Event *event = new MissEvent(*inst, cost, blocks[i]);
			if(logFor(LOG_BB))
				log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
			EVENT(bb).add(event);
		}
	}

	// process data stage L1
	if(has_dl1) {

		Pair<int, dcache::BlockAccess *> blocks = dcache::DATA_BLOCKS(bb);
		for(int i = 0; i < blocks.fst; i++) {
			dcache::BlockAccess& acc = blocks.snd[i];
			dcache::BlockAccess::action_t action = acc.action();
			bool write =  action == dcache::BlockAccess::STORE;

			// compute cost
			ot::time cost = 0;
			switch(acc.kind()) {
			case dcache::BlockAccess::ANY:
				break;
			case dcache::BlockAccess::BLOCK:
				cost = costOf(acc.block().address(), write);
				if(!cost)
					goto error;
				break;
			case dcache::BlockAccess::RANGE:
				cost = costOf(Address(acc.first()), write);
				if(!cost)
					goto error;
				break;
			error:
				if(!cost)
					warn(_ << "memory instruction at " << acc.instruction()->address() << " access address " << acc.block().address() << " that is out of banks!");
				break;
			}
			if(!cost)
				cost = action == write ? mem->worstWriteAccess() : mem->worstReadAccess();

			// make the event
			Event *event = new DataMissEvent(acc.instruction(), cost, acc, bb);
			if(logFor(LOG_BB))
				log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
			EVENT(bb).add(event);
		}

	}

	// process branch prediction
	if(has_branch) {
		Inst *binst = bb->controlInst();
		if(binst && binst->isConditional()) {
			switch(branch::CATEGORY(bb)) {

			// simple case of default prediction
			case branch::ALWAYS_D:
				for(BasicBlock::OutIterator out(bb); out; out++) {
					occurrence_t occ;
					if(out->kind() == Edge::NOT_TAKEN)
						occ = ALWAYS;
					else
						occ = NEVER;
					Event *event = new BranchPredictionEvent(out->target()->firstInst(), bht->getCondPenalty(), occ, 0, bb);
					if(logFor(LOG_BB))
						log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
					EVENT(*out).add(event);
				}
				break;

			// complex case with good/miss-prediction
			case branch::FIRST_UNKNOWN:		handleVariableBranchPred(bb, ENCLOSING_LOOP_HEADER(branch::HEADER(bb))); break;
			case branch::ALWAYS_H:
			case branch::NOT_CLASSIFIED:	handleVariableBranchPred(bb, bb); break;
			}
		}
	}

}



void StandardEventBuilder::handleVariableBranchPred(BasicBlock *bb, BasicBlock *wbb) {

	// x^MP_i = \sum{(i, j) in N} x^MP_(i,j)
	static string msg = "branch prediction relation to edges";
	ilp::System *sys = ipet::SYSTEM(workspace());
	ilp::Constraint *c = sys->newConstraint(msg, ilp::Constraint::EQ);
	c->addLeft(1, branch::MISSPRED_VAR(bb));

	// traverse the successors
	for(BasicBlock::OutIterator out(bb); out; out++) {
		string name;
		if(_explicit)
			name = _ << "x_mp_" << out->source()->number() << "_" << out->target()->number();
		ilp::Var *var = sys->newVar(name);
		Event *event = new BranchPredictionEvent(out->target()->firstInst(), bht->getCondPenalty(), SOMETIMES, var, wbb);
		if(logFor(LOG_BB))
			log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
		EVENT(*out).add(event);
		c->addRight(1, var);
	}
}


p::declare StandardEventBuilder::reg = p::init("otawa::etime::StandardEventBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<StandardEventBuilder>()
	.provide(STANDARD_EVENTS_FEATURE)
	.provide(EVENTS_FEATURE)
	.require(ipet::ILP_SYSTEM_FEATURE);

} } // otawa::etime
