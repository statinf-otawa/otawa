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
#include <otawa/icat3/features.h>

#include "MustPersDomain.h"

namespace otawa { namespace icat3 {

/**
 */
class ICacheEvent: public etime::Event {
public:
	ICacheEvent(const icache::Access& acc, ot::time cost, category_t cat, const PropList *site, Block *head = 0,
			etime::type_t type = etime::LOCAL, rel_t rel = pair(null<Inst>(), null<const hard::PipelineUnit>()))
		: Event(acc.instruction()), _cost(cost), _acc(acc), _site(site), _cat(cat), _head(head), _type(type), _rel(rel) {	}

	virtual etime::kind_t kind(void) const { return etime::FETCH; }
	virtual ot::time cost(void) const { return _cost; }
	virtual etime::type_t type(void) const { return _type; }

	virtual etime::occurrence_t occurrence(void) const {
		switch(_cat) {
		case AH:	return etime::NEVER;
		case PE:	return etime::SOMETIMES;
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
		case NC:	return WEIGHT(_site);
		case PE:	{
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
		return on && _cat == PE;	// only when on = true!
	}

	virtual void estimate(ilp::Constraint *cons, bool on) {
		ASSERT(on);
		ASSERT(_cat == PE)
		for(Block::EdgeIter i = _head->ins(); i(); i++)
			if(!BACK_EDGE(*i))
				cons->addLeft(1, ipet::VAR(*i));
	}

	virtual rel_t related(void) const { return _rel; }
	virtual inline void relate(const rel_t &rel) { _rel = rel; }
	virtual inline void setType(etime::type_t type) { _type = type; }

private:
	ot::time _cost;
	const icache::Access& _acc;
	const PropList *_site;
	category_t _cat;
	Block *_head;
	etime::type_t _type;
	rel_t _rel;
};


/**
 */
class EdgeEventBuilder: public BBProcessor {
public:
	static p::declare reg;
	EdgeEventBuilder(void): BBProcessor(reg), coll(0), mustpers(0), acss(0), A(0), mem(0) { }

protected:

	static const bool PREFIX = true, BLOCK = false;
	typedef MustPersDomain::t acs_t;

	/**
	 */
	virtual void setup(WorkSpace *ws) {
		coll = icat3::LBLOCKS(ws);
		ASSERT(coll);
		A = coll->A();
		mem = hard::MEMORY_FEATURE.get(ws);
		ASSERT(mem);
		mustpers = new MustPersDomain *[coll->sets()];
		acss = new acs_t[coll->sets()];
		for(int i = 0; i < coll->sets(); i++)
			mustpers[i] = new MustPersDomain(*coll, i);
	}

	/**
	 */
	virtual void cleanup(WorkSpace *ws) {
		for(int i = 0; i < coll->sets(); i++)
			delete mustpers[i];
		delete [] mustpers;
		delete [] acss;
	}

	/**
	 */
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *v) {
		used.clear();
		for(Identifier<etime::Unit *>::Getter i(v, etime::TIME_UNIT); i(); i++) {
			etime::Unit *tu = *i;
			for(auto e: tu->path()) {
				processAccesses(*tu, etime::PREFIX, e->source(), e->source(), e->source());
				processAccesses(*tu, etime::PREFIX, otawa::LOOP_EXIT_EDGE(e) ? e->sink() : e->source(), e->source(), e);
				if(otawa::LOOP_EXIT_EDGE(e))
					used.clear();	// to support loop level popup in PERS
			}
			processAccesses(*tu, etime::BLOCK, v, v, v);
		}
	}

	/**
	 * Generate and store the events.
	 * @param tu	Time unit to put the event in.
	 * @param place	Place of the contribution.
	 * @param cv	Counting vertex.
	 * @param iv	Input vertex.
	 * @param cont	Contribution.
	 */
	void processAccesses(etime::Unit& tu, etime::place_t place, Block *cv, Block *iv, const PropList *cont) {
		const Bag<icache::Access>& accs = icache::ACCESSES(cont);
		for(int i = 0; i < accs.count(); i++) {
			LBlock *lb = LBLOCK(accs[i]);

			// need initialization?
			if(!used.contains(lb->set())) {
				used.add(lb->set());
				acss[lb->set()] = acs_t((*MUST_IN(iv))[lb->set()], (*PERS_IN(iv))[lb->set()]);
			}

			// build the events
			etime::Event *evt = makeEvent(cv, place, accs[i], acss[lb->set()]);
			if(logFor(LOG_INST)) {
				log	<< "\t\t\t"
					<< (place == etime::PREFIX ? "prefix" : "block")
					<< " event " << evt << io::endl;
			}
			//tu.add(evt);	!!TOFIX!!

			// update state
			mustpers[lb->set()]->update(accs[i], acss[lb->set()]);
		}
	}

	/**
	 * Generate the events for the given access.
	 * @param cv	Counting vertex.
	 * @param place	Place in the time unit.
	 * @param acc	Access to process.
	 * @param acs	ACS before the access.
	 * @return 		Built event.
	 */
	etime::Event *makeEvent(Block *cv, etime::place_t place, const icache::Access& acc, acs_t& acs) {
		LBlock *lb = LBLOCK(acc);
		ASSERT(lb);

		// compute the category
		category_t cat = NC;
		Block *ch = 0;
		age_t age = (mustpers[lb->set()]->must(acs))[lb->index()];
		if(0 <= age && age < A)
			cat = AH;
		else {
			LoopIter h(cv);
			for(int i = mustpers[lb->set()]->pers(acs).stack().length() - 1; i >= 0 && h(); i--, h++) {
				age = mustpers[lb->set()]->pers(acs).stack()[i][lb->index()];
				if(0 <= age && age < A) {
					ch = *h;
					cat = PE;
					break;
				}
			}
		}

		// if required, obtain the time
		ot::time t = 0;
		if(cat != AH) {
			const hard::Bank *bank = mem->get(lb->address());
			if(!bank)
				t = mem->worstReadTime();
			else
				t = bank->readLatency();
		}

		// build the event
		return new ICacheEvent(acc, t, cat, cv, ch);
	}

	const LBlockCollection *coll;
	MustPersDomain **mustpers;
	acs_t *acss;
	Vector<int> used;
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
	.require(COLLECTED_CFG_FEATURE)
	.require(etime::UNIT_FEATURE);


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



