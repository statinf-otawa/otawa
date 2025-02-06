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
//#include <otawa/dcache/features.h>

using namespace otawa::cache;

namespace otawa { namespace etime {


class AlwaysMissEvent: public Event {
public:
	AlwaysMissEvent(Inst *inst, ot::time cost)
		: Event(inst), _cost(cost) {	}
	kind_t kind(void) const override { return FETCH; }
	ot::time cost(void) const override { return _cost; }
	type_t type(void) const override { return LOCAL; }
	occurrence_t occurrence(void) const override { return ALWAYS; }
	cstring name(void) const override { return "fetch stage"; }
	string detail(void) const override { return _ << "instruction fetch @ " << inst()->address(); }
	int weight(void) const override { return 0; }
	bool isEstimating(bool on) const override { return false; }
	void estimate(ilp::Constraint *cons, bool on) const override { }
private:
	ot::time _cost;
};

class DataAlwaysMissEvent: public AlwaysMissEvent {
public:
	DataAlwaysMissEvent(Inst *inst, ot::time cost)
		: AlwaysMissEvent(inst, cost) {	}
	kind_t kind(void) const override { return MEM; }
	cstring name(void) const override { return "memory stage"; }
	string detail(void) const override { return _ << "data access @ " << inst()->address(); }
};


class MissEvent: public Event {
public:
	MissEvent(Inst *inst, ot::time cost, LBlock *lb)
		: Event(inst), _lb(lb), _cost(cost) {	}

	kind_t kind() const override { return FETCH; }
	ot::time cost() const override { return _cost; }
	type_t type() const override { return LOCAL; }

	occurrence_t occurrence() const override {
		switch(CATEGORY(_lb)) {
		case ALWAYS_HIT:		return NEVER;
		case FIRST_HIT:			return SOMETIMES;
		case FIRST_MISS:		return SOMETIMES;
		case ALWAYS_MISS:		return ALWAYS;
		case NOT_CLASSIFIED:	return SOMETIMES;
		default:				ASSERT(0); return SOMETIMES;
		}
	}

	cstring name() const override { return "L1 instruction cache"; }

	string detail() const override { return _ << *CATEGORY(_lb) << " L1-I" << " @ " << _lb->address(); }

	int weight() const override {
		switch(CATEGORY(_lb)) {
		case ALWAYS_HIT:		return 0;
		case FIRST_HIT:
		case ALWAYS_MISS:
		case NOT_CLASSIFIED:	return WEIGHT(_lb->bb());
		case FIRST_MISS:		{
									Block *parent = otawa::ENCLOSING_LOOP_HEADER(_lb);
									if(!parent)
										return 1;
									else
										return WEIGHT(parent);
								}
		default:				ASSERT(0); return SOMETIMES;
		}
	}

	bool isEstimating(bool on) const override {
		return on && CATEGORY(_lb) != NOT_CLASSIFIED;	// only when on = true!
	}

	void estimate(ilp::Constraint *cons, bool on) const override {
		if(on)
			cons->addLeft(1, otawa::MISS_VAR(_lb));
	}

private:
	LBlock *_lb;
	ot::time _cost;
};


class BranchPredictionEvent: public Event {
public:
	BranchPredictionEvent(Inst *inst, ot::time cost, occurrence_t occ, ilp::Var *var, Block *wbb)
		: Event(inst), _var(var), _cost(cost), _occ(occ), _wbb(wbb) { }

	kind_t kind() const override { return BRANCH; }
	ot::time cost() const override { return _cost; }
	type_t type() const override { return AFTER; }
	occurrence_t occurrence() const override { return _occ; }
	cstring name() const override { return "branch prediction"; }
	string detail() const override { return _ << "BP " << *branch::CATEGORY(_wbb); }
	bool isEstimating(bool on) const override { return on; }

	void estimate(ilp::Constraint *cons, bool on) const override {
		if(on)
			cons->addLeft(1, _var);
	}

	int weight() const override {
		if(!_wbb)
			return 1;
		else
			return WEIGHT(_wbb);
	}

private:
	ilp::Var *_var;
	ot::time _cost;
	occurrence_t _occ;
	Block *_wbb;
};

#if 0
class DataMissEvent: public Event {
public:
	DataMissEvent(Inst *inst, ot::time cost, dcache::BlockAccess& acc, BasicBlock *bb)
	: Event(inst), _cost(cost), _acc(acc), _bb(bb) { }

	kind_t kind() const override { return MEM; }
	ot::time cost() const override { return _cost; }
	type_t type() const override { return LOCAL; }

	occurrence_t occurrence(void) const override {
		switch(dcache::CATEGORY(_acc)) {
		case cache::ALWAYS_HIT:		return NEVER;
		case cache::FIRST_HIT:		return SOMETIMES;
		case cache::FIRST_MISS:		return SOMETIMES;
		case cache::ALWAYS_MISS:	return ALWAYS;
		case cache::NOT_CLASSIFIED:	return SOMETIMES;
		default:					ASSERT(0); return SOMETIMES;
		}
	}

	cstring name() const override { return "L1 data cache"; }
	string detail() const override {
		if(_acc.kind() == dcache::BlockAccess::BLOCK)
			return _ << *dcache::CATEGORY(_acc) << " L1-D" << " @ " << _acc.block().address();
		else if(_acc.kind() == dcache::BlockAccess::RANGE)
			return _ << *dcache::CATEGORY(_acc) << " L1-D" << " @ set[" << _acc.first() << ":" << _acc.last() << "]";
		else
			return _ << *dcache::CATEGORY(_acc) << " L1-D any";
	}

	int weight() const override {
		switch(dcache::CATEGORY(_acc)) {
		case cache::ALWAYS_HIT:		return 0;
		case cache::FIRST_HIT:
		case cache::ALWAYS_MISS:
		case cache::NOT_CLASSIFIED:	return WEIGHT(_bb);
		case cache::FIRST_MISS:		{
										Block *parent = otawa::ENCLOSING_LOOP_HEADER(_bb);
										if(!parent)
											return 1;
										else
											return WEIGHT(parent);
									}
		default:					ASSERT(0); return 0;
		}
	}

	bool isEstimating(bool on) override {
		return on && dcache::CATEGORY(_acc) != NOT_CLASSIFIED;	// only when on = true!
	}

	void estimate(ilp::Constraint *cons, bool on) override {
		if(on) {
			ASSERT(*dcache::MISS_VAR(_acc) != nullptr);
			if(_acc.kind() == dcache::BlockAccess::BLOCK)
				cons->addLeft(1, dcache::MISS_VAR(_acc));
			else
				cons->addLeft(_acc.blocks().length(), dcache::MISS_VAR(_acc));
		}
	}

private:
	ot::time _cost;
	dcache::BlockAccess& _acc;
	BasicBlock *_bb;
};


class DataPurgeEvent: public Event {
public:
	DataPurgeEvent(Inst *inst, dcache::BlockAccess& acc, BasicBlock *bb)
	: Event(inst), _acc(acc), _bb(bb) { }

	kind_t kind() const override { return MEM; }
	ot::time cost() const override { return dcache::PURGE_TIME(_acc); }
	type_t type() const override { return LOCAL; }

	occurrence_t occurrence() const override {
		switch(dcache::PURGE(_acc)) {
		case dcache::NO_PURGE:		return NEVER;
		case dcache::MAY_PURGE:
		case dcache::PERS_PURGE:	return SOMETIMES;
		case dcache::MUST_PURGE:	return ALWAYS;
		default:					ASSERT(0); return SOMETIMES;
		}
	}

	cstring name() const override { return "L1 data cache purge"; }
	string detail() const override { return _ << *dcache::PURGE(_acc) << " L1-D"; }

	int weight() const override {
		switch(dcache::PURGE(_acc)) {
		case dcache::NO_PURGE:		return 0;
		case dcache::MAY_PURGE:
		case dcache::MUST_PURGE:	return WEIGHT(_bb);
		case dcache::PERS_PURGE:	{
										Block *parent = otawa::ENCLOSING_LOOP_HEADER(_bb);
										if(!parent)
											return 1;
										else
											return WEIGHT(parent);
									}
		case dcache::INV_PURGE:		ASSERTP(false, "unsupported invalid purge"); return 0;
		default:					ASSERT(0); return 0;
		}
	}

	bool isEstimating(bool on) override {
		return on;	// only when on = true!
	}

	void estimate(ilp::Constraint *cons, bool on) override {
		if(on) {
			ASSERT(*dcache::MISS_VAR(_acc) != nullptr);
			cons->addLeft(1, dcache::MISS_VAR(_acc));
		}
	}

private:
	dcache::BlockAccess& _acc;
	BasicBlock *_bb;
};
#endif


/**
 * Abstract classes for event builders based on memory.
 */
class MemBuilder: public Monitor {
public:
	MemBuilder(Monitor& mon, const hard::Memory *_mem)
	: Monitor(mon), mem(_mem) {
		bank = mem->banks()[0];
	}

	ot::time costOf(Address addr, bool write = false) {
		if(!bank->contains(addr)) {
			const hard::Bank *new_bank = mem->get(addr);
			if(!new_bank)
				return -1;
			bank = new_bank;
		}
		return write ? bank->writeLatency() : bank->latency();
	}

	inline const hard::Memory *memory() const { return mem; }

private:
	const hard::Memory *mem;
	const hard::Bank *bank;
};


/**
 * Abstract class to manage instruction fetch events.
 */
class FetchBuilder: public MemBuilder {
public:
	using MemBuilder::MemBuilder;
	virtual ~FetchBuilder() { }

	virtual void process(WorkSpace *ws, BasicBlock *bb) = 0;
};


/**
 * Actual fetch builder where there is not instruction cache.
 */
class NoCacheFetchBuilder: public FetchBuilder {
public:
	using FetchBuilder::FetchBuilder;

	void process(WorkSpace *ws, BasicBlock *bb) override {
		for(auto i: *bb) {
			ot::time t = costOf(i->address());
			if(t < 0) {
				log << "WARNING: no bank contains instruction at " << i->address() << ": assuming worst case latency.\n";
				t = memory()->worstReadAccess();
			}
			Event *event = new AlwaysMissEvent(i, t);
			if(logFor(LOG_BB))
				log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
			EVENT(bb).add(event);

		}
	}
};


/**
 * Actual fetch builder when an instruction cache is available.
 */
class CacheFetchBuilder: public FetchBuilder {
public:
	using FetchBuilder::FetchBuilder;

	void process(WorkSpace *ws, BasicBlock *bb) override {
		const AllocArray<LBlock* >& blocks = **BB_LBLOCKS(bb);
		BasicBlock::InstIter inst = bb->insts();
		for(int i = 0; i < blocks.count(); i++) {

			// find the instruction
			while(inst->topAddress() <= blocks[i]->address()) {
				inst++;
				ASSERT(inst);
			}

			// find the bank
			ot::time cost = costOf(blocks[i]->address());
			if(!cost) {
				log << "WARNING: no bank contains instruction at " << inst->address() << ". Assuming worst latency.";
				cost = memory()->worstReadAccess();
			}

			// create the event
			Event *event = new MissEvent(*inst, cost, blocks[i]);
			if(logFor(LOG_BB))
				log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
			EVENT(bb).add(event);
		}
	}
};


/**
 * Abstract data access event builder.
 */
class DataAccessBuilder: public MemBuilder {
public:
	using MemBuilder::MemBuilder;
	virtual ~DataAccessBuilder() { }

	virtual void process(WorkSpace *ws, BasicBlock *bb) = 0;
};

/**
 * Actual data cache builder when there is not data cache.
 */
class NoCacheDataAccessBuilder: public DataAccessBuilder {
public:
	using DataAccessBuilder::DataAccessBuilder;

	void process(WorkSpace *ws, BasicBlock *bb) override {
		for(auto i: *bb) {
			if(i->isMem()) {

				// count the number of access
				int n = 1;
				if(i->isMulti())
					n = i->multiCount();

				// compute access time
				ot::time t;
				if(i->isStore())
					t = n * memory()->worstWriteTime();
				else
					t = n * memory()->worstReadTime();

				// create event
				Event *event = new DataAlwaysMissEvent(i, t);
				if(logFor(LOG_BB))
					log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
				EVENT(bb).add(event);
			}
		}
	}
};


/**
 * Actual data cache builder when a data cache is available.
 */
#if 0
class CacheDataAccessBuilder: public DataAccessBuilder {
public:
	CacheDataAccessBuilder(Monitor& mon, const hard::Memory *mem, bool _wb)
		: DataAccessBuilder(mon, mem), wb(_wb) { }

	void process(WorkSpace *ws, BasicBlock *bb) override {
		auto& blocks = *dcache::DATA_BLOCKS(bb);
		for(int i = 0; i < blocks.count(); i++) {
			dcache::BlockAccess& acc = blocks[i];
			dcache::BlockAccess::action_t action = acc.action();
			bool write =  action == dcache::BlockAccess::STORE;

			// compute cost
			ot::time cost = -1;
			switch(acc.kind()) {
			case dcache::BlockAccess::ANY:
				break;
			case dcache::BlockAccess::BLOCK:
				cost = costOf(acc.block().address(), write);
				if(cost == -1)
					goto error;
				break;
			case dcache::BlockAccess::RANGE:
				cost = costOf(Address(acc.first()), write);
				if(cost == -1)
					goto error;
				break;
			error:
				log << "WARNING: memory instruction at " << acc.instruction()->address() << " access " << acc << " that is out of banks!";
				break;
			}
			if(cost == -1)
				cost = action == write ? memory()->worstWriteAccess() : memory()->worstReadAccess();

			// make the event
			Event *event = new DataMissEvent(acc.instruction(),  cost, acc, bb);
			if(logFor(LOG_BB))
				log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
			EVENT(bb).add(event);

			// for write-back, take into account the purge
			if(wb && dcache::CATEGORY(acc) != cache::ALWAYS_HIT) {
				Event *pevt = new DataPurgeEvent(acc.instruction(), acc, bb);
				if(logFor(LOG_BB))
					log << "\t\t\t\tadded " << pevt->inst()->address() << "\t" << pevt->name() << io::endl;
				EVENT(bb).add(pevt);
			}
		}
	};

private:
	bool wb;
};
#endif


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
:	BBProcessor(r),
	mem(nullptr),
	caches(nullptr),
	bht(nullptr),
	has_branch(false),
	//bank(nullptr),
	_explicit(false),
	fetch(nullptr),
	data(nullptr)
{ }


void StandardEventBuilder::configure(const PropList& props) {
	BBProcessor::configure(props);
	_explicit = ipet::EXPLICIT(props);
}


///
void StandardEventBuilder::cleanup(WorkSpace *ws) {
	delete fetch;
	fetch = nullptr;
}


/**
 */
void StandardEventBuilder::setup(WorkSpace *ws) {

	// look for memory
	mem = hard::MEMORY_FEATURE.get(ws);
	if(mem == nullptr)
		throw ProcessorException(*this, "cannot manage memory access events without memory description!");

	// get cache configuration
	if(ws->provides(hard::CACHE_CONFIGURATION_FEATURE))
		caches = hard::CACHE_CONFIGURATION_FEATURE.get(ws);

	// look if instruction cache L1 is available
	if(caches == nullptr || caches->instCache() == nullptr) {
		fetch = new NoCacheFetchBuilder(*this, mem);
		if(logFor(LOG_FUN))
			log << "\tevents for straight instruction fetch\n";
	}
	else {
        #if 0
		if(!ws->isProvided(ICACHE_ONLY_CONSTRAINT2_FEATURE))
			throw ProcessorException(*this, "L1 instruction cache but no cache analysis!");
		fetch = new CacheFetchBuilder(*this, mem);
		if(logFor(LOG_CFG))
			log << "\tevents for instruction cache L1\n";
        #endif
	}

	// look for data cache L1
	if(caches == nullptr || caches->dataCache() == nullptr) {
		data = new NoCacheDataAccessBuilder(*this, mem);
		if(logFor(LOG_FUN))
			log << "\tevents for straight data access\n";
	}
	else {
#		if 0
		if(!ws->isProvided(dcache::CONSTRAINTS_FEATURE))
			throw ProcessorException(*this, "L1 data cache but no cache analysis!");
		bool wb = caches->dataCache()->writePolicy() == hard::Cache::WRITE_BACK;
		if(wb && !ws->isProvided(dcache::PURGE_FEATURE))
			throw ProcessorException(*this, "write-back L1 data cache but no purge analysis provided (dcache::PURGE_FEATURE)!");
		data = new CacheDataAccessBuilder(*this, mem, wb);
		if(logFor(LOG_CFG))
			log << "\tevents for data cache L1\n";
#		endif
	}

	// look if branch prediction is available
	has_branch = ws->isProvided(branch::CONSTRAINTS_FEATURE);
	if(has_branch) {
		bht = hard::BHT_CONFIG(ws);
		ASSERT(bht);
	}
}


/**
 */
void StandardEventBuilder::processBB(WorkSpace *ws, CFG *cfg, Block *b) {
	if(b->isEnd())
		return;

	if(b->isBasic()) {
		auto bb = b->toBasic();

		// process instruction stage L1
        if (fetch)
		    fetch->process(ws, bb);

		// process data stage L1
		if (data)
			data->process(ws, bb);
	}

	// process branch prediction
	if(has_branch && b->isBasic()) {
		BasicBlock *bb = b->toBasic();
		Inst *binst = bb->control();
		if(binst && binst->isConditional()) {
			switch(branch::CATEGORY(bb)) {

			// simple case of default prediction
			case branch::ALWAYS_D:
				for(Block::EdgeIter out = bb->outs(); out(); out++) {
					occurrence_t occ;
					if(out->isNotTaken())
						occ = ALWAYS;
					else
						occ = NEVER;
					Event *event = new BranchPredictionEvent(out->target()->toBasic()->first(), bht->getCondPenalty(), occ, 0, bb);
					if(logFor(LOG_BB))
						log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << io::endl;
					EVENT(*out).add(event);
				}
				break;

			// complex case with good/miss-prediction
			case branch::FIRST_UNKNOWN:		handleVariableBranchPred(bb, ENCLOSING_LOOP_HEADER(branch::HEADER(bb))); break;
			case branch::ALWAYS_H:
			case branch::NOT_CLASSIFIED:	handleVariableBranchPred(bb, bb); break;

			case branch::STATIC_TAKEN:
				for(Block::EdgeIter out = bb->outs(); out(); out++) {
					occurrence_t occ;
					int cost = 0;
					if(out->isNotTaken()) // incorrect not taken
						cost = bht->getIncorrectNotTakenPenalty();
					else // correct not taken
						cost = bht->getCorrectTakenPenalty();

					if(cost)
						occ = ALWAYS;
					else
						occ = NEVER;

					Event *event = new BranchPredictionEvent(out->target()->toBasic()->first(), cost, occ, 0, bb);
					if(logFor(LOG_BB))
						log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << ", occ = " << occ << ", cost =  " << cost << io::endl;
					EVENT(*out).add(event);
				}
				break;

			case branch::STATIC_NOT_TAKEN:
				for(Block::EdgeIter out = bb->outs(); out(); out++) {
					occurrence_t occ;
					int cost = 0;
					if(out->isNotTaken())
						cost = bht->getCorrectNotTakenPenalty();
					else
						cost = bht->getIncorrectTakenPenalty();

					if(cost)
						occ = ALWAYS;
					else
						occ = NEVER;

					Event *event = new BranchPredictionEvent(out->target()->toBasic()->first(), cost, occ, 0, bb);
					if(logFor(LOG_BB))
						log << "\t\t\t\tadded " << event->inst()->address() << "\t" << event->name() << ", occ = " << occ << ", cost =  " << cost << io::endl;
					EVENT(*out).add(event);
				}
				break;

			default:						ASSERT(false); break;
			}
		}
	}

}



void StandardEventBuilder::handleVariableBranchPred(BasicBlock *bb, Block *wbb) {

	// x^MP_i = \sum{(i, j) in N} x^MP_(i,j)
	static string msg = "branch prediction relation to edges";
	ilp::System *sys = ipet::SYSTEM(workspace());
	ilp::Constraint *c = sys->newConstraint(msg, ilp::Constraint::EQ);
	c->addLeft(1, branch::MISSPRED_VAR(bb));

	// traverse the successors
	for(Block::EdgeIter out = bb->outs(); out(); out++) {
		string name;
		if(_explicit)
			name = _ << "x_mp_" << out->source()->index() << "_" << out->target()->index();
		ilp::Var *var = sys->newVar(name);
		Inst *tinst = 0;
		if(out->sink()->isBasic())
			tinst = out->sink()->toBasic()->first();
		else if(out->sink()->isSynth())
			tinst = out->sink()->toSynth()->callee()->first();
		ASSERT(tinst);
		Event *event = new BranchPredictionEvent(tinst, bht->getCondPenalty(), SOMETIMES, var, wbb);
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
	.require(ipet::ILP_SYSTEM_FEATURE)
	.require(hard::MEMORY_FEATURE);

} } // otawa::etime
