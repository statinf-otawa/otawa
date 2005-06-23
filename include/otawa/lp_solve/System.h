/*
 *	$Id$
 *	Copyright (c) 2005, Institut de Recherche en Informatique de Toulouse.
 *
 *	otawa/lp_solve/System.h -- lp_solve::System class interface.
 */
#ifndef OTAWA_LP_SOLVE_SYSTEM_H
#define OTAWA_LP_SOLVE_SYSTEM_H

#include <elm/genstruct/HashTable.h>
#include <otawa/ilp.h>

namespace otawa { namespace lp_solve {

// External classes
class Constraint;

// Var class
class Var {
	ilp::Var *var;
	int col;
	double val;
public:
	inline Var(ilp::Var *variable, int column);
	inline ilp::Var *variable(void) const;
	inline int column(void) const;
	inline double value(void) const;
	inline void setValue(double value);
};

// System class
class System: public ilp::System {
	friend class Constraint;
	
	elm::genstruct::HashTable<ilp::Var *, Var *> vars;
	Constraint *conss;
	Constraint *ofun;
	int cols, rows;
	double val;

	void removeConstraint(Constraint *cons);
public:
	System(bool max = true);
	~System(void);
	Var *findVar(ilp::Var *var);
	Var *getVar(ilp::Var *var);
	
	// ilp::System overload
	virtual ilp::Constraint *newConstraint(ilp::Constraint::comparator_t comp,
		double constant = 0);
	virtual bool solve(void);
	virtual void addObjectFunction(double coef, ilp::Var *var = 0);
	virtual double valueOf(ilp::Var *var);
	virtual double value(void);
	virtual ilp::Var *newVar(elm::String name);
	virtual void dump(elm::io::OutStream& out = elm::io::stdout);
	virtual int countVars(void);
	virtual int countConstraints(void);
	
};

// System::Var Inlines
inline Var::Var(ilp::Var *variable, int column): var(variable) {
	col = column;
	val = 0;
}

inline ilp::Var *Var::variable(void) const {
	return var;
}

inline int Var::column(void) const {
	return col;
}

inline double Var::value(void) const {
	return val;
}

inline void Var::setValue(double value) {
	val = value;
}

} } // otawa::lp_solve

#endif	// OTAWA_LP_SOLVE_SYSTEM_H
