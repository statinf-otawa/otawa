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

#include <elm/data/FragTable.h>
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
	virtual dyndata::AbstractIter<Term> *terms(void);
	virtual void setComparator(comparator_t comp);
	virtual void setLabel(const string& label);

	class TermIter: public Expression::Iter {
	public:
		inline TermIter(AbstractConstraint *cons): Expression::Iter(&cons->_expr) { }
	};

	void reset(void) override;

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
	AbstractSystem(bool max = true);
	~AbstractSystem();
	inline bool isMaximizing() const { return _max; }
	inline bool isMinimizing() const { return !_max; };

	Constraint *newConstraint(Constraint::comparator_t comp, double constant) override;
	Constraint *newConstraint(const string& label, Constraint::comparator_t comp, double constant) override;
	void addObjectFunction(double coef, Var *var) override;
	Var *newVar(const string& name) override;
	int countVars() override;
	int countConstraints() override;
	dyndata::AbstractIter<ilp::Constraint*> *constraints() override;
	dyndata::AbstractIter<ilp::Constraint::Term> *objTerms() override;
	void exportLP(io::Output& out) override;
	void dumpSolution(io::Output& out) override;
	Var *newVar(Var::type_t type, const string& name) override;
	void resetObjectFunction() override;
	void remove(otawa::ilp::Constraint*) override;

protected:
	int index(ilp::Var *var);

	class ConstIter: public FragTable<AbstractConstraint *>::Iter {
	public:
		inline ConstIter(AbstractSystem *sys)
			: FragTable<AbstractConstraint *>::Iter(sys->conss) { }
		inline void next(void) {
			do {
				FragTable<AbstractConstraint *>::Iter::next();
			} while(!ended() && !item());
		}
	};

	class VarIter: public FragTable<AbstractVar *>::Iter {
	public:
		inline VarIter(AbstractSystem *sys)
			: FragTable<AbstractVar *>::Iter(sys->vars) { }
		inline void next(void) {
			do {
				FragTable<AbstractVar *>::Iter::next();
			} while(!ended() && !item());
		}
	};

private:

	friend class AbstractConstraint;
	void remove(AbstractConstraint *cons);
	friend class AbstractVar;
	void remove(AbstractVar *var);

	FragTable<AbstractVar *> vars;
	FragTable<AbstractConstraint *> conss;
	Expression obj;
	bool cleaning;
	bool _max;
	List<int> free;
};

} }		// otawa::ilp

#endif /* OTAWA_ILP_ABSTRACTSYSTEM_H_ */
