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

	// deprecated
	STAGE,	// stage argument
	FU		// FU argument
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

typedef enum place_t {
	NO_PLACE = 0,
	PREFIX = 1,
	BLOCK = 2
} place_t;
io::Output& operator<<(io::Output& out, place_t p);


// Event class
class Event: public PropList {
public:
	typedef Pair<Inst *, const hard::PipelineUnit *> rel_t;

	Event(Inst *inst, place_t place);
	virtual ~Event(void);
	inline Inst *inst(void) const { return _inst; }
	inline place_t place(void) const { return _place; }

	// accessors
	virtual kind_t kind(void) const = 0;
	virtual ot::time cost(void) const = 0;
	virtual type_t type(void) const = 0;
	virtual occurrence_t occurrence(void) const;
	virtual cstring name(void) const;
	virtual string detail(void) const;
	virtual Pair<Inst *, const hard::PipelineUnit *> related(void) const;
	virtual const hard::PipelineUnit *unit(void) const;

	virtual void relate(const rel_t &rel);
	virtual void setType(type_t type);

	// deprecated
	virtual const hard::Stage *stage(void) const;
	virtual const hard::FunctionalUnit *fu(void) const;

	// heuristic contribution
	virtual int weight(void) const;

	// ILP contribution
	virtual bool isEstimating(bool on);
	virtual void estimate(ilp::Constraint *cons, bool on);

private:
	Inst *_inst;
	place_t _place;
};
io::Output& operator<<(io::Output& out, place_t place);
io::Output& operator<<(io::Output& out, Event *event);


class Unit: public PropList {
public:
	typedef List<Edge *>::Iter ContribIter;
	typedef List<Event *>::Iter EventIter;

	inline Unit(void): _e(0) { }
	inline Unit(Edge *e): _e(e) { }
	inline Edge *edge(void) const { return _e; }
	inline ContribIter contribs(void) const { return ContribIter(_cs); }
	inline EventIter events(void) const { return EventIter(_es); }

	inline void add(Edge *e) { _cs.add(e); }
	inline void add(Event *e) { _es.add(e); }

private:
	Edge *_e;
	List<Edge *> _cs;
	List<Event *> _es;
};


// time feature
extern p::feature TIME_UNIT_FEATURE;
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
extern p::id<ot::time> HTS_OFFSET;

} }	// otawa::etime

#endif /* OTAWA_ETIME_FEATURES_H_ */
