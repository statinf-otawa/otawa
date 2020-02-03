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

typedef enum kind_t {
	NONE,
	FETCH,	// no argument
	MEM,	// no argument
	BRANCH,	// no argument
	CUSTOM,	// unit argument
} kind_t;

typedef enum occurence_t {
	NEVER = 0,
	SOMETIMES = 1,
	ALWAYS = 2
} occurrence_t;
io::Output& operator<<(io::Output& out, occurrence_t occ);

typedef enum type_t {
	LOCAL = 0,
	AFTER = 1,
	NOT_BEFORE = 2,
} type_t;


// Event class
class Event: public PropList {
public:
	typedef Pair<Inst *, const hard::PipelineUnit *> rel_t;

	Event(Inst *inst);
	virtual ~Event(void);
	inline Inst *inst(void) const { return _inst; }

	// accessors
	virtual cstring name(void) const;
	virtual string detail(void) const;

	virtual kind_t kind(void) const = 0;
	virtual ot::time cost(void) const = 0;
	virtual occurrence_t occurrence(void) const;

	virtual type_t type(void) const = 0;
	virtual Pair<Inst *, const hard::PipelineUnit *> related(void) const;
	virtual const hard::PipelineUnit *unit(void) const;

	// heuristic contribution
	virtual int weight(void) const;

	// ILP contribution
	virtual bool isEstimating(bool on);
	virtual void estimate(ilp::Constraint *cons, bool on);

private:
	Inst *_inst;
};
io::Output& operator<<(io::Output& out, Event *event);


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
extern p::id<Event *> EVENT;
extern p::id<Event *> PREFIX_EVENT;

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
