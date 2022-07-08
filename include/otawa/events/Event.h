/*
 *	Event class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2021, IRIT UPS.
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
#ifndef OTAWA_EVENTS_EVENT_H
#define OTAWA_EVENTS_EVENT_H

#include <otawa/prop.h>

namespace otawa {

// predeclarations
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


// Event class
class Event: public PropList {
public:
	typedef Pair<Inst *, const hard::PipelineUnit *> rel_t;

	typedef enum kind_t {
		NONE = 0,
		FETCH = 1,	// no argument
		MEM = 2,	// no argument
		BRANCH = 3,	// no argument
		CUSTOM = 4,	// unit argument
	} kind_t;

	typedef enum occurrence_t {
		NO_OCCURRENCE = -1,
		NEVER = 0,
		SOMETIMES = 1,
		ALWAYS = 2
	} occurrence_t;

	typedef enum type_t {
		LOCAL = 0,
		AFTER = 1,
		NOT_BEFORE = 2,
	} type_t;

	Event(Inst *inst);
	virtual ~Event();
	inline Inst *inst() const { return _inst; }

	// accessors
	virtual cstring name() const;
	virtual string detail() const;

	virtual kind_t kind() const = 0;
	virtual ot::time cost() const = 0;
	virtual occurrence_t occurrence() const;

	virtual type_t type() const = 0;
	virtual Pair<Inst *, const hard::PipelineUnit *> related(void) const;
	virtual const hard::PipelineUnit *unit() const;

	// heuristic contribution
	virtual int weight() const;

	// ILP contribution
	virtual bool isEstimating(bool on) const;
	virtual void estimate(ilp::Constraint *cons, bool on) const;

private:
	Inst *_inst;
};
io::Output& operator<<(io::Output& out, Event *event);
io::Output& operator<<(io::Output& out, Event::occurrence_t occ);
Event::occurrence_t operator|(Event::occurrence_t x, Event::occurrence_t y);

}	// otawa

#endif // OTAWA_EVENTS_EVENT_H
