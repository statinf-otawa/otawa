/*
 *	$Id$
 *	Copyright (c) 2005, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/lp_solve/LPSolveSystem.cpp -- LPSolveSystem class definition.
 */

#include <assert.h>
#include <otawa/lp_solve/System.h>
#include <otawa/lp_solve/Constraint.h>
extern "C" {
#	include <lpkit.h>
}

using namespace elm;

namespace otawa { namespace lp_solve {

// HashKey for ilp::Var *
class VarHashKey: public HashKey<ilp::Var *> {
	virtual unsigned long hash(ilp::Var *v) {
		return (unsigned long)v;
	};
	virtual bool equals(ilp::Var *key1, ilp::Var *key2) {
		return key1 == key2;
	};
};
static VarHashKey var_hkey_obj;
template <> HashKey<ilp::Var *>& HashKey<ilp::Var *>::def = otawa::lp_solve::var_hkey_obj;


/**
 * @class System
 * This the lp_solve version of the ilp::System class.
 * As lp_solve is very bound in its API, this class mainly gather information
 * about the ILP system and, at the computation time, it builds the lp_solve
 * data structure and launch the computation.
 */

/**
 * Build a new lp_solve system.
 * @param max	True for maximizing, false for minimizing.
 */
System::System(bool max): conss(0), cols(0), rows(0) {
	 ofun = new Constraint(this, max ? Constraint::GT : Constraint::LT, 0, 0);
}


/**
 * Look for a lp_solve variable matching the given ilp variable.
 * @param	var	ilp variable.
 * @return	Mathcing lp_solve variable or null.
 */
Var *System::findVar(ilp::Var *var) {
	assert(var);
	return vars.get(var, 0);
}


/**
 * Find or create the lp_solve variable matching the ilp variable.
 * @param var	ilp variable.
 * @return		Found or created matching lp_solve variable.
 */
Var *System::getVar(ilp::Var *var) {
	Var *lvar = findVar(var);
	if(lvar == 0) {
		lvar = new Var(var, ++cols);
		vars.put(var, lvar);
	}
	return lvar;
}


/**
 * Free all ressources.
 */
System::~System(void) {
	Constraint *cons = conss;
	while(cons) {
		Constraint *next = cons->next();
		delete cons;
		cons = next;
	}
}


// Overload
ilp::Constraint *System::newConstraint(ilp::Constraint::comparator_t comp,
double constant) {
	conss = new Constraint(this, comp, constant, conss);
	rows++;
	return conss;
}


// Overload
void System::addObjectFunction(double coef, ilp::Var *var) {
	ofun->add(coef, var);
}


// Overload
double System::valueOf(ilp::Var *var) {
	assert(var);
	Var *lvar = getVar(var);
	assert(lvar);
	return lvar->value();
}


// Overload
double System::value(void) {
	return val;
}


// Overload
bool System::solve(void) {
	static short comps[] = { LE, LE, EQ, GE, GE };
	static double corr[] = { 1, 0, 0, 0, -1 };
	
	// Allocate and initialize the lp_solve data structure
	lprec *lp = make_lp(0, cols);
	REAL row[cols + 1];
	for(int i = 0; i < cols; i++) {
		row[i] = 0;
		//set_int(lp, i, TRUE);
	}
	
	// Build the object function
	ofun->fillRow(row);
	set_obj_fn(lp, row);
	ofun->resetRow(row);
	if(ofun->comparator() >= 0)
		set_maxim(lp);
	else
		set_minim(lp);
	
	// Build the matrix
	for(Constraint *cons = conss; cons; cons = cons->next()) {
		cons->fillRow(row);
		Constraint::comparator_t comp = cons->comparator();
		double cst = cons->constant();
		add_constraint(lp, row, comps[comp + 2], cst + corr[comp + 2]);
		cons->resetRow(row);
	}
	
	// Launch the resolution
	int fail = ::solve(lp);
	
	// Record the result
	if(!fail) {
		val = (double)lp->best_solution[0];
		for(elm::genstruct::HashTable<ilp::Var *, Var *>::ItemIterator var(vars);
		var; var++)
			var->setValue((double)lp->best_solution[lp->rows + var->column()]);
	}
	
	// Clean up
	/*print_lp(lp);
	print_solution(lp);*/
	delete_lp(lp);
	return !fail;
}

} }	// otawa::lp_solve

