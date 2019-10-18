/*
 *	TimeUnitTimer class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2019, IRIT UPS.
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

#include <otawa/etime/features.h>
#include <otawa/etime/TimeUnitTimer.h>
#include <otawa/etime/StandardILPGenerator.h>
#include <otawa/ilp.h>

namespace otawa { namespace etime {

/**
 * ILP generator supporting time units.
 * @ingroup etime
 */
class TimeUnitGenerator: public ILPGenerator {

	class EventBounds {
	public:
		inline EventBounds(Event *e): evt(e) { }
		inline Event *event() const { return evt; }
		inline const List<ilp::Var *>& upperBounded() const { return up; }
		inline const List<ilp::Var *>& lowerBounded() const { return lo; }
		inline void addUpperBounded(ilp::Var *v) { up.add(v); }
		inline void addLowerBounded(ilp::Var *v) { lo.add(v); }
	private:
		Event *evt;
		List<ilp::Var *> up;
		List<ilp::Var *> lo;
	};

public:

	TimeUnitGenerator(Monitor& mon):
		ILPGenerator(mon),
		tu(nullptr),
		_t_lts(0),
		_t_lts_set(false),
		_x_e(nullptr),
		_x_hts(nullptr)
	{ }

	void process(WorkSpace *ws) override {

		// process each edge
		for(auto g: *CFGCollection::get(ws)) {
			if(logFor(LOG_FUN))
				log << "\tCFG " << g << io::endl;
			for(auto v: *g)
				if(v->isBasic()) {
					if(logFor(LOG_BLOCK))
						log << "\t\t" << v << io::endl;
					for(auto tu: TIME_UNIT.all(v))
						process(tu);
				}
		}

		// generate the bounding constraints of the events
		/*for(auto coll: colls) {
			coll->make(system());
			delete coll;
		}
		colls.clear();*/

	}

protected:

	/**
	 * Collect events.
	 * @param events	To store events to.
	 * @param props		Property list to look in.
	 * @param part		Part number in the unit.
	 * @param id		Identifier used to retrieve events.
	 */
	void collectEvents(Vector<EventCase>& events, PropList *props, part_t part, p::id<Event *>& id) {
		for(auto e: id.all(props))
			events.add(EventCase(e, part));
	}

	/**
	 * Collect instructions to build an instruction sequence.
	 * @param seq		Sequence to be completed with instructions of the given block.
	 * @param b			Block containing instructions to add to the sequence.
	 * @param part		ExeGraph part (PROLOGUE or BLOCK).
	 * @param index		Current instruction index.
	 */
	void collectInsts(ParExeSequence *seq, Block *b, code_part_t part, int& index) {
		if(!b->isBasic())
			return;
		BasicBlock *bb = b->toBasic();
		for(auto i: *bb)
			seq->addLast(new ParExeInst(i, bb, part, index++));
	}

	void process(Unit *_tu) {
		tu = _tu;
		Vector<EventCase> events;


		// collect BBs and events (for retro compatibility)
		collectEvents(events, tu->path().first()->source(), 0, EVENT);
		part_t p = 0;
		for(auto e: tu->path()) {
			collectEvents(events, e, p, PREFIX_EVENT);
			p++;
			collectEvents(events, e, p, EVENT);
			collectEvents(events, e->sink(), p, EVENT);
		}

		// numbers the dynamic events
		int cnt = 0;
		for(int i = 0; i < events.count(); i++)
			if(events[i].event()->occurrence() == SOMETIMES)
				events[i].setIndex(cnt++);


		// build the sequence
		ParExeSequence *seq = new ParExeSequence();
		int index = 0;
		collectInsts(seq, tu->path().first()->source(), PROLOGUE, index);
		code_part_t cpart = PROLOGUE;
		for(auto e: tu->path()) {
			if(e == tu->pivot())
				cpart = otawa::BLOCK;
			collectInsts(seq, e->sink(), cpart, index);
		}

		// build the graph
		//prepare(e, events, countDyn(events));
		ParExeGraph *g = build(seq);
		//solve(e, g, events);
		//finish(events);

		// clean all
		delete g;
		delete seq;

	}

	void configure(const PropList& props) override {

	}

	void contributeBase(ot::time time) override {

	}

	void contributeTime(ot::time time) override {

	}

	void contributePositive(EventCase event, bool prec) override {

	}

	void contributeNegative(EventCase event, bool prec) override {

	}


private:
	HashMap<Event *, EventBounds *> bounds;

	Unit *tu;
	ot::time _t_lts;
	bool _t_lts_set;
	ilp::Var *_x_e, *_x_hts;
	BitVector _done;
};


/**
 * @class TimeUnitTimer
 *
 * This class is used to compute execution time and to fulfill the IPET ILP
 * objective functions with the corresponding time based on time unit
 * data structures.
 *
 * Notice that the events are collected from time unit but also from
 * older event site, that is, edges and blocks. Time is recorded according
 * to the class LTS/HTS properties.
 *
 * @warning Current implementation only supports time unit made of two BBs.
 *
 * @ingroup etime
 */

/**
 */
p::declare TimeUnitTimer::reg = p::init("otawa::etime::TimeUnitTimer", Version(1, 0, 0))
	.extend<AbstractTimeBuilder>()
	.make<TimeUnitTimer>()
	.require(etime::UNIT_FEATURE);

/**
 */
TimeUnitTimer::TimeUnitTimer(): AbstractTimeBuilder(reg), gen(nullptr) {
	gen = new TimeUnitGenerator(*this);
	setGenerator(gen);
}

/**
 */
TimeUnitTimer::~TimeUnitTimer() {
	if(gen != nullptr)
		delete gen;
}

} }	// otawa::etime
