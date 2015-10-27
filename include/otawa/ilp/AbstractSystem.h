/*
 *	ilp::AbstractSystem class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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
#ifndef OTAWA_ILP_ABSTRACTSYSTEM_H_
#define OTAWA_ILP_ABSTRACTSYSTEM_H_

#include <elm/genstruct/FragTable.h>
#include <otawa/ilp/Expression.h>
#include <otawa/ilp/System.h>

namespace otawa { namespace ilp {

using namespace elm;

class AbstractSystem;

class AbstractConstraint: public Constraint {
public:
	AbstractConstraint(string label,  comparator_t comp, double cst);
	virtual ~AbstractConstraint(void);

	virtual double coefficient(Var *var = 0) const;
	virtual double constant(void) const;
	virtual comparator_t comparator(void) const;
	virtual const string& label(void) const;
	virtual void add(double coef, Var *var = 0);
	virtual void sub(double coef, Var *var = 0);
	virtual elm::datastruct::IteratorInst<Term> *terms(void);
	virtual void setComparator(comparator_t comp);
	virtual void setLabel(const string& label);

	class TermIter: public Expression::Iterator {
	public:
		inline TermIter(AbstractConstraint *cons): Expression::Iterator(&cons->_expr) { }
	};

private:
	friend class AbstractSystem;
	string _label;
	Expression _expr;
	comparator_t _comp;
	double _cst;
	int _idx;
	AbstractSystem *_sys;
};

class AbstractSystem: public System {
	class AbstractVar: public Var {
	public:
		AbstractVar(const string& name, type_t type);
		virtual ~AbstractVar(void);
		inline int index(void) const { return _idx; }
	private:
		friend class AbstractSystem;
		AbstractSystem *_sys;
		int _idx;
	};

public:
	AbstractSystem(void);
	virtual ~AbstractSystem(void);
	virtual Constraint *newConstraint(Constraint::comparator_t comp, double constant);
	virtual Constraint *newConstraint(const string& label, Constraint::comparator_t comp, double constant);
	virtual void addObjectFunction(double coef, Var *var);
	virtual Var *newVar(const string& name);
	virtual int countVars(void);
	virtual int countConstraints(void);
	virtual elm::datastruct::IteratorInst<ilp::Constraint*> *constraints(void);
	virtual elm::datastruct::IteratorInst<ilp::Constraint::Term> *objTerms(void);
	virtual void exportLP(io::Output& out);
	virtual void dumpSolution(io::Output& out);
	virtual Var *newVar(Var::type_t type, const string& name);

protected:
	int index(ilp::Var *var);

	class ConstIter: public genstruct::FragTable<AbstractConstraint *>::Iterator {
	public:
		inline ConstIter(AbstractSystem *sys)
			: genstruct::FragTable<AbstractConstraint *>::Iterator(sys->conss) { }
		inline void next(void) {
			do {
				genstruct::FragTable<AbstractConstraint *>::Iterator::next();
			} while(!ended() && !item());
		}
	};

	class VarIter: public genstruct::FragTable<AbstractVar *>::Iterator {
	public:
		inline VarIter(AbstractSystem *sys)
			: genstruct::FragTable<AbstractVar *>::Iterator(sys->vars) { }
		inline void next(void) {
			do {
				genstruct::FragTable<AbstractVar *>::Iterator::next();
			} while(!ended() && !item());
		}
	};

private:

	friend class AbstractConstraint;
	void remove(AbstractConstraint *cons);
	friend class AbstractVar;
	void remove(AbstractVar *var);

	genstruct::FragTable<AbstractVar *> vars;
	genstruct::FragTable<AbstractConstraint *> conss;
	Expression obj;
	bool cleaning;
};

} }		// otawa::ilp

#endif /* OTAWA_ILP_ABSTRACTSYSTEM_H_ */
