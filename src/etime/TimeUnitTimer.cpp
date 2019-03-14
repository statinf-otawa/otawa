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
public:

	TimeUnitGenerator(const Monitor& mon):
		ILPGenerator(mon),
		tu(nullptr),
		base_time(0)
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

	void process(Unit *_tu) {

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
	Unit *tu;
	ot::time base_time;
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
p::declare reg = p::init("otawa::etime::TimeUnitTimer", Version(1, 0, 0))
	.extend<AbstractTimeBuilder>()
	.make<TimeUnitTimer>()
	.require(etime::TIME_UNIT_FEATURE);

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
