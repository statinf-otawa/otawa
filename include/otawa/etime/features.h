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
	class Stage;
}
class Inst;

namespace etime {

typedef enum {
	NONE,
	FETCH,	// no argument
	MEM,	// no argument
	BRANCH,	// no argument
	STAGE,	// stage argument
	FU		// FU argument
} kind_t;

typedef enum {
	NEVER = 0,
	SOMETIMES = 1,
	ALWAYS = 2
} occurrence_t;

typedef enum type_t {
	EDGE,
	BLOCK
} type_t;


// Event class
class Event: public PropList {
public:

	Event(Inst *inst);
	virtual ~Event(void);
	inline Inst *inst(void) const { return _inst; }

	// accessors
	virtual kind_t kind(void) const = 0;
	virtual ot::time cost(void) const = 0;
	virtual type_t type(void) const = 0;
	virtual occurrence_t occurrence(void) const;
	virtual cstring name(void) const;
	virtual string detail(void) const = 0;
	virtual const hard::Stage *stage(void) const;
	virtual const hard::FunctionalUnit *fu(void) const;

	// heuristic contribution
	virtual int weight(void) const;

	// ILP contribution
	virtual bool isEstimating(bool on);
	virtual void estimate(ilp::Constraint *cons, bool on);

private:
	Inst *_inst;
};

// event feature
extern p::feature STANDARD_EVENTS_FEATURE;
extern p::feature EVENTS_FEATURE;
extern Identifier<Event *> EVENT;

// configuration feature
extern p::feature EDGE_TIME_FEATURE;

} }	// otawa::etime

#endif /* OTAWA_ETIME_FEATURES_H_ */
