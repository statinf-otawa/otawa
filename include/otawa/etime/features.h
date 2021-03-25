/*
 *	etime plugin features
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
#ifndef OTAWA_ETIME_FEATURES_H_
#define OTAWA_ETIME_FEATURES_H_

#include <elm/data/List.h>
#include <elm/data/Vector.h>
#include <otawa/cfg/CFG.h>
#include <otawa/events/features.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// pre-declarations
namespace ilp {
	class Constraint;
	class System;
	class Var;
}	// ilp
namespace hard {
	class FunctionalUnit;
	class PipelineUnit;
	class Stage;
}
class Inst;

namespace etime {

// Event transition declarations
typedef Event::kind_t kind_t;
constexpr kind_t NONE = Event::NONE;
constexpr kind_t FETCH = Event::FETCH;
constexpr kind_t MEM = Event::MEM;
constexpr kind_t BRANCH = Event::BRANCH;
constexpr kind_t CUSTOM = Event::CUSTOM;

typedef Event::occurrence_t occurrence_t;
constexpr occurrence_t NEVER = Event::NEVER;
constexpr occurrence_t SOMETIMES = Event::SOMETIMES;
constexpr occurrence_t ALWAYS = Event::ALWAYS;

typedef Event::type_t type_t;
constexpr type_t LOCAL = Event::LOCAL;
constexpr type_t AFTER = Event::AFTER;
constexpr type_t NOT_BEFORE = Event::NOT_BEFORE;

typedef otawa::Event Event;


class Unit: public PropList {
public:

	inline Unit(void): _e(0) { }
	inline Unit(Edge *e): _e(e) { }
	inline Edge *pivot(void) const { return _e; }
	inline const List<Edge *> path(void) const { return _cs; }
	inline const List<Pair<Event *, int> > events(void) const { return _es; }

	inline void add(Edge *e) { _cs.add(e); }

	class BlockIter: public PreIterator<BlockIter, BlockIter> {
	public:
		inline BlockIter(): u(nullptr), i(0) { }
		inline BlockIter(Unit& unit): u(&unit), e(u->_cs), i(0) { }
		inline const BlockIter& item() const { return *this; }
		inline bool ended() const { return e.ended(); }
		inline void next() { if(i != 0) e.next(); i++; }
		inline Block *block() const { if(i == 0) return e->source(); else return e->sink(); }
		inline void add(Event *e) {  }
 	private:
		Unit *u;
		List<Edge *>::Iter e;
		int i;
	};

	// deprecated
	inline const List<Edge *> contribs(void) const { return _cs; }
	inline Edge *edge(void) const { return _e; }

private:
	Edge *_e;
	List<Edge *> _cs;
	List<Pair<Event *, int> > _es;
};


// useful to manage places in time computation
typedef enum place_t {
	NO_PLACE = 0,
	PREFIX = 1,
	BLOCK = 2
} place_t;
io::Output& operator<<(io::Output& out, place_t p);

// time feature
class TimeUnitBuilder;
extern p::feature UNIT_FEATURE;
extern p::id<Unit *> TIME_UNIT;

// event feature
extern p::feature STANDARD_EVENTS_FEATURE;
extern p::feature EVENTS_FEATURE;
extern p::alias<Event *> EVENT;
extern p::alias<Event *> PREFIX_EVENT;

// configuration feature
extern p::id<bool> PREDUMP;
extern p::id<int> EVENT_THRESHOLD;
extern p::id<bool> RECORD_TIME;
extern p::feature EDGE_TIME_FEATURE;
extern p::id<ot::time> LTS_TIME;
extern p::id<Pair<ot::time, ilp::Var *> > HTS_CONFIG;
extern p::id<bool> ONLY_START;
extern p::id<bool> NO_ILP_OBJECTIVE;

} }	// otawa::etime

#endif /* OTAWA_ETIME_FEATURES_H_ */
