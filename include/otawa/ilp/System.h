/*
 *	$Id$
 *	ilp::System class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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
#ifndef OTAWA_ILP_SYSTEM_H
#define OTAWA_ILP_SYSTEM_H

#include <elm/dyndata/Collection.h>
#include <elm/io/OutStream.h>
#include <elm/string.h>
#include <otawa/ilp/Constraint.h>
#include <otawa/proc/Monitor.h>

#include "features.h"

namespace otawa {

class WorkSpace;

namespace ilp {

class ILPPlugin;

// System class
class System {
public:
	virtual ~System(void) { }

	// 1.0.0 & 1.1.0 interfaces
	virtual Constraint *newConstraint(Constraint::comparator_t comp, double constant = 0) = 0;
	virtual Constraint *newConstraint(const string& label, Constraint::comparator_t comp, double constant = 0) = 0;
	virtual bool solve(WorkSpace *ws = 0) = 0;
	virtual void addObjectFunction(double coef, Var *var = 0) = 0;
	virtual double valueOf(Var *var) = 0;
	virtual double value(void) = 0;
	virtual Var *newVar(const string& name = "") = 0;
	virtual int countVars(void) = 0;
	virtual int countConstraints(void) = 0;
	virtual dyndata::AbstractIter<ilp::Constraint*> *constraints(void) = 0;
	virtual dyndata::AbstractIter<ilp::Constraint::Term> *objTerms(void) = 0;
	virtual void exportLP(io::Output& out = elm::cout) = 0;
	virtual void dumpSystem(io::Output& out = elm::cout);
	virtual void dumpSolution(io::Output& out = elm::cout) = 0;
	virtual void dump(elm::io::OutStream& out = elm::io::out);

	// 1.2.0 interface
	virtual bool solve(WorkSpace *ws, otawa::Monitor& mon);
	virtual string lastErrorMessage(void);
	virtual ILPPlugin *plugin(void);
	virtual Var *newVar(Var::type_t type, const string& name = "") = 0;

	// 1.2.1 interface
	virtual void resetObjectFunction(void) = 0;

	// 1.3 interface
	virtual void remove(ilp::Constraint *c) = 0;

	// object function
	inline void addObject(const Term& t) { addObjectFunction(t.snd, t.fst); }
	inline void subObject(const Term& t) { addObjectFunction(-t.snd, t.fst); }
	void addObject(const Expression& e);
	void subObject(const Expression& e);
	void copyObject(Expression& e);
	void setObject(const Expression& e);

	// output methods
	bool hasDump(format_t fmt);
	void dump(format_t fmt, elm::io::OutStream& out = elm::io::out);
	void dumpLPSolve(elm::io::OutStream& out = elm::io::out);
	void dumpCPlex(elm::io::OutStream& out = elm::io::out);
	void dumpMOSEK(elm::io::OutStream& out = elm::io::out);

	class ConstIterator: public dyndata::Iter<Constraint *> {
	public:
		inline ConstIterator(dyndata::AbstractIter<Constraint*> *_inst):
			dyndata::Iter<Constraint *>(_inst) { }
		inline ConstIterator(System *_sys): dyndata::Iter<Constraint *>(_sys->constraints())
			{ }
	};

	class ObjTermIterator: public dyndata::Iter<Constraint::Term> {
	public:
		inline ObjTermIterator(dyndata::AbstractIter<Constraint::Term> *_inst):
			dyndata::Iter<Constraint::Term>(_inst) { }
		inline ObjTermIterator(System *_sys):
			dyndata::Iter<Constraint::Term>(_sys->objTerms()) { }
	};
};

} }	// otawa::ilp

#endif // OTAWA_ILP_SYSTEM_H
