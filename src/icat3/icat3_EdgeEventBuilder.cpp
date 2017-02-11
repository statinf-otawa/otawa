/*
 *	icat3::EdgEventBuilder class implementation
 *	Copyright (c) 2016, IRIT UPS.
 *
 *	This file is part of OTAWA
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <otawa/cfg.h>
#include <otawa/etime/features.h>
#include <otawa/hard/Memory.h>
#include <otawa/icache/features.h>
#include <otawa/ipet.h>
#include <otawa/proc/Processor.h>
#include <otawa/program.h>

#include "features.h"
#include "MustPersDomain.h"

namespace otawa { namespace icat3 {

io::Output& operator<<(io::Output& out, category_t cat) {
	static cstring labels[] = {
		"none",
		"AH",
		"PERS",
		"AM",
		"NC"
	};
	ASSERT(0 < cat && cat <= NC);
	out << labels[cat];
	return out;
}

/**
 */
class ICacheEvent: public etime::Event {
public:
	ICacheEvent(const icache::Access& acc, ot::time cost, category_t cat, Block *b, Block *head = 0,
			etime::type_t type = etime::LOCAL, rel_t rel = pair(null<Inst>(), null<const hard::PipelineUnit>()))
		: Event(acc.instruction()), _cost(cost), _acc(acc), _b(b), _cat(cat), _head(head), _type(type), _rel(rel) {	}

	virtual etime::kind_t kind(void) const { return etime::FETCH; }
	virtual ot::time cost(void) const { return _cost; }
	virtual etime::type_t type(void) const { return _type; }

	virtual etime::occurrence_t occurrence(void) const {
		switch(_cat) {
		case AH:	return etime::NEVER;
		case PERS:	return etime::SOMETIMES;
		case AM:	return etime::ALWAYS;
		case NC:	return etime::SOMETIMES;
		default:	ASSERT(0); return etime::SOMETIMES;
		}
	}

	virtual cstring name(void) const { return "L1 instruction cache"; }

	virtual string detail(void) const { return _ << _cat << " access to " << _acc.address(); }

	virtual int weight(void) const {
		switch(_cat) {
		case AH:	return 0;
		case AM:
		case NC:	return WEIGHT(_b);
		case PERS:	{
						Block *parent = otawa::ENCLOSING_LOOP_HEADER(_head);
						if(!parent)
								return 1;
							else
								return WEIGHT(parent);
					}
		default:	ASSERT(0); return 0;
		}
	}

	virtual bool isEstimating(bool on) {
		return on && _cat == PERS;	// only when on = true!
	}

	virtual void estimate(ilp::Constraint *cons, bool on) {
		ASSERT(on);
		ASSERT(_cat == PERS)
		for(Block::EdgeIter i = _head->ins(); i; i++)
			if(!BACK_EDGE(*i))
				cons->addLeft(1, ipet::VAR(i));
	}

	virtual rel_t related(void) const { return _rel; }
	virtual inline void relate(const rel_t &rel) { _rel = rel; }
	virtual inline void setType(etime::type_t type) { _type = type; }

private:
	ot::time _cost;
	const icache::Access& _acc;
	Block *_b;
	category_t _cat;
	Block *_head;
	etime::type_t _type;
	rel_t _rel;
};


/**
 * Abstract class to generate the instruction cache access categories.
 */
class AbstractCategoryBuilder: public Processor {
	static p::declare reg;
	AbstractCategoryBuilder(void): Processor(reg), coll(0), mustpers(0), A(0), mem(0) { }

protected:

	virtual void processCategory(Block *b, icache::Access& acc, category_t cat, Block *hd = 0) = 0;

	virtual void processWorkSpace(WorkSpace *ws) {

		// gather needed information
		coll = icat3::LBLOCKS(ws);
		ASSERT(coll);
		A = coll->A();
		const CFGCollection *cfgs = *otawa::INVOLVED_CFGS(ws);
		ASSERT(cfgs);
		mem = hard::MEMORY(ws);
		ASSERT(mem);

		// process basic blocks
		for(int set = 0; set < coll->cache()->setCount(); set++)
			if((*coll)[set].count() > 0) {
				if(logFor(LOG_FUN))
					log << "\tprocessing set " << set << io::endl;
				MustPersDomain mustpers_inst(*coll, set);
				mustpers = &mustpers_inst;
				for(CFGCollection::BBIterator b(cfgs); b; b++)
					if(b->isBasic())
						for(BasicBlock::BasicIns e(b->toBasic()); e; e++) {
							if(logFor(LOG_BLOCK))
								log << "\t\tbasic edge " << (*e).source() << ", " << (*e).edge() << ", " << (*e).sink() << io::endl;
							make(e, set);
						}
			}
	}

private:

	bool use(const PropList *b, int set) {
		const Bag<icache::Access>& accs = icache::ACCESSES(b);
		for(int i = 0; i < accs.count(); i++)
			if(LBLOCK(accs[i])->set() == set)
				return true;
		return false;
	}

	void make(const BasicBlock::BasicEdge& e, int set) {

		// prepare ACS
		MustPersDomain::t a = mustpers->bot();
		if(e.source())
			for(Block::EdgeIter i = e.source()->ins(); i; i++)
				mustpers->join(a, MustPersDomain::t((*MUST_STATE(i))[set], (*PERS_STATE(i))[set]));
		else
			mustpers->join(a, mustpers->init());

		// process the source
		if(e.source() && use(e.source(), set))
			make(e.sink(), *e.edge(), icache::ACCESSES(e.source()), a, set, true);

		// process the edge
		if(e.edge() && use(e.edge(), set))
			make(e.sink(), *e.edge(), icache::ACCESSES(e.edge()), a, set, false);

		// process the
		a = MustPersDomain::t((*MUST_STATE(e.edge()))[set], (*PERS_STATE(e.edge()))[set]);
		make(e.sink(), *e.edge(), icache::ACCESSES(e.sink()), a, set, false);
	}

	void make(Block *b, Edge& site, Bag<icache::Access>& accs, MustPersDomain::t& acs, int set, bool prefix) {
		for(int i = 0; i < accs.count(); i++) {

			// obtain the access
			const icache::Access& acc = accs[i];
			LBlock *lb = LBLOCK(acc);
			if(lb->set() != set)
				continue;

			// compute the category
			category_t cat = NC;
			Block *ch = 0;
			age_t age = mustpers->must(acs)[lb->index()];
			if(0 <= age && age < A)
				cat = AH;
			else if(b) {
				LoopIter h(b);
				for(int i = mustpers->pers(acs).stack().length() - 1; i >= 0 && h; i--, h++) {
					age = mustpers->pers(acs).stack()[i][lb->index()];
					if(0 <= age && age < A) {
						ch = h;
						cat = PERS;
						break;
					}
				}
			}

			processCategory(b, accs[i], cat, ch);

			// if required, obtain the time
			time_t t = 0;
			if(cat != AH) {
				const hard::Bank *bnk = mem->get(lb->address());
				ASSERT(bnk);
				t = bnk->latency();
			}

			// build the event
			etime::Event *e = new ICacheEvent(acc, t, cat, b, ch);
			if(prefix) {
				if(logFor(LOG_INST))
					log << "\t\t\tprefix event " << e << io::endl;
				etime::PREFIX_EVENT(site).add(e);
			}
			else {
				if(logFor(LOG_INST))
					log << "\t\t\tblock event " << e << io::endl;
				etime::EVENT(site).add(e);
			}
			mustpers->update(acc, acs);
		}
	}

	const LBlockCollection *coll;
	MustPersDomain *mustpers;
	int A;
	const hard::Memory *mem;
};


/**
 */
class EdgeEventBuilder: public Processor {
public:
	static p::declare reg;
	EdgeEventBuilder(void): Processor(reg), coll(0), mustpers(0), A(0), mem(0) { }

protected:

	virtual void setup(WorkSpace *ws) {
	}

	virtual void processWorkSpace(WorkSpace *ws) {

		// gather needed information
		coll = icat3::LBLOCKS(ws);
		ASSERT(coll);
		A = coll->A();
		const CFGCollection *cfgs = *otawa::INVOLVED_CFGS(ws);
		ASSERT(cfgs);
		mem = hard::MEMORY(ws);
		ASSERT(mem);

		// process basic blocks
		for(int set = 0; set < coll->cache()->setCount(); set++)
			if((*coll)[set].count() > 0) {
				if(logFor(LOG_FUN))
					log << "\tprocessing set " << set << io::endl;
				MustPersDomain mustpers_inst(*coll, set);
				mustpers = &mustpers_inst;
				for(CFGCollection::BBIterator b(cfgs); b; b++)
					if(b->isBasic())
						for(BasicBlock::BasicIns e(b->toBasic()); e; e++) {
							if(logFor(LOG_BLOCK))
								log << "\t\tbasic edge " << (*e).source() << ", " << (*e).edge() << ", " << (*e).sink() << io::endl;
							make(e, set);
						}
			}
	}

private:

	bool use(const PropList *b, int set) {
		const Bag<icache::Access>& accs = icache::ACCESSES(b);
		for(int i = 0; i < accs.count(); i++)
			if(LBLOCK(accs[i])->set() == set)
				return true;
		return false;
	}

	void make(const BasicBlock::BasicEdge& e, int set) {

		// prepare ACS
		MustPersDomain::t a = mustpers->bot();
		if(e.source())
			for(Block::EdgeIter i = e.source()->ins(); i; i++)
				mustpers->join(a, MustPersDomain::t((*MUST_STATE(i))[set], (*PERS_STATE(i))[set]));
		else
			mustpers->join(a, mustpers->init());

		// process the source
		if(e.source() && use(e.source(), set))
			make(e.sink(), *e.edge(), icache::ACCESSES(e.source()), a, set, true);

		// process the edge
		if(e.edge() && use(e.edge(), set))
			make(e.sink(), *e.edge(), icache::ACCESSES(e.edge()), a, set, false);

		// process the
		a = MustPersDomain::t((*MUST_STATE(e.edge()))[set], (*PERS_STATE(e.edge()))[set]);
		make(e.sink(), *e.edge(), icache::ACCESSES(e.sink()), a, set, false);
	}

	void make(Block *b, Edge& site, const Bag<icache::Access>& accs, MustPersDomain::t& acs, int set, bool prefix) {
		for(int i = 0; i < accs.count(); i++) {

			// obtain the access
			const icache::Access& acc = accs[i];
			LBlock *lb = LBLOCK(acc);
			if(lb->set() != set)
				continue;

			// compute the category
			category_t cat = NC;
			Block *ch = 0;
			age_t age = mustpers->must(acs)[lb->index()];
			if(0 <= age && age < A)
				cat = AH;
			else if(b) {
				LoopIter h(b);
				for(int i = mustpers->pers(acs).stack().length() - 1; i >= 0 && h; i--, h++) {
					age = mustpers->pers(acs).stack()[i][lb->index()];
					if(0 <= age && age < A) {
						ch = h;
						cat = PERS;
						break;
					}
				}
			}

			// if required, obtain the time
			time_t t = 0;
			if(cat != AH) {
				const hard::Bank *bnk = mem->get(lb->address());
				ASSERT(bnk);
				t = bnk->latency();
			}

			// build the event
			etime::Event *e = new ICacheEvent(acc, t, cat, b, ch);
			if(prefix) {
				if(logFor(LOG_INST))
					log << "\t\t\tprefix event " << e << io::endl;
				etime::PREFIX_EVENT(site).add(e);
			}
			else {
				if(logFor(LOG_INST))
					log << "\t\t\tblock event " << e << io::endl;
				etime::EVENT(site).add(e);
			}
			//cerr << "DEBUG: OBSERVE: " << site.sink() << " <- " << site.source() << ": " << set << ": " << e << io::endl;
			mustpers->update(acc, acs);
		}
	}

	const LBlockCollection *coll;
	MustPersDomain *mustpers;
	int A;
	const hard::Memory *mem;
};

p::declare EdgeEventBuilder::reg = p::init("otawa::icat3::EdgeEventBuilder", Version(1, 0, 0))
	.extend<BBProcessor>()
	.make<EdgeEventBuilder>()
	.provide(EDGE_EVENTS_FEATURE)
	.require(LBLOCKS_FEATURE)
	.require(MUST_PERS_ANALYSIS_FEATURE)
	.require(hard::MEMORY_FEATURE)
	.require(COLLECTED_CFG_FEATURE);


/**
 * This feature ensures that event are build from MUST/PERS and/or MAY analysis
 * of the instruction cache. The events are generated from the classification
 * of instruction accesses as Always-Hit, Always-Hit, Persistent or Not-Classified.
 * These events are compatible with events of @ref etime.
 *
 * The events are all added to the edge for instructions from the source block and
 * from the sink block. This improves the precision considering that an instruction
 * cache blocks often spans over successive basic blocks.
 */
p::feature EDGE_EVENTS_FEATURE("otawa::icat3::EDGE_EVENTS_FEATURE", p::make<EdgeEventBuilder>());

} }	// otawa::icat3



