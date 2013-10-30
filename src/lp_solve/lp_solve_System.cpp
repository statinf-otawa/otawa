/*
 *	$Id$
 *	Copyright (c) 2005, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/lp_solve/LPSolveSystem.cpp -- LPSolveSystem class definition.
 */

#include <elm/assert.h>
#include <stdio.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include "System.h"
#include "Constraint.h"
extern "C" {
#	include <lpkit.h>
}

using namespace elm;

// HashKey for ilp::Var *
namespace elm {
	template <>
	class HashKey<otawa::ilp::Var *> {
	public:
		static unsigned long hash(otawa::ilp::Var *v)
			{ return (unsigned long)v; }
		static bool equals(otawa::ilp::Var *key1, otawa::ilp::Var *key2)
			{ return key1 == key2; }
	};
} // elm

namespace otawa { namespace lp_solve {

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
	ASSERT(var);
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
	while(conss)
		delete conss;
}


/**
 * Remove a constraint from the system.
 * @param cons	Constraint to remove.
 */
void System::removeConstraint(Constraint *cons) {
	if(cons == conss)
		conss = conss->nxt;
	else {
		for(Constraint *prev = conss, *cur = conss->nxt; cur;
		prev = cur, cur = cur->nxt)
			if(cur == cons) {
				prev->nxt = cur->nxt;
				break;
			}
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
ilp::Constraint *System::newConstraint(
	const string& label,
	ilp::Constraint::comparator_t comp,
	double constant)
{
	conss = new Constraint(this, label, comp, constant, conss);
	rows++;
	return conss;
}


// Overload
void System::addObjectFunction(double coef, ilp::Var *var) {
	ofun->add(coef, var);
}


// Overload
double System::valueOf(ilp::Var *var) {
	ASSERT(var);
	Var *lvar = getVar(var);
	ASSERT(lvar);
	return lvar->value();
}


// Overload
double System::value(void) {
	return val;
}


// Overload
bool System::solve(WorkSpace *ws) {
	static short comps[] = { LE, LE, EQ, GE, GE };
	static double corr[] = { -1, 0, 0, 0, +1 };

	// Allocate and initialize the lp_solve data structure
	lprec *lp = make_lp(0, cols);
	REAL row[cols + 1];
	for(int i = 0; i < cols; i++) {
		row[i] = 0;
		set_int(lp, i, TRUE);
	}

	// Build the object function
	ofun->fillRow(row);
	row[cols] = 0;
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
		add_constraint(lp, row,
			comps[comp - Constraint::LT],
			cst + corr[comp - Constraint::LT]);
		cons->resetRow(row);
	}

	// Launch the resolution
	set_print_sol(lp, FALSE);
	/*set_epsilon(lp, DEF_EPSILON);
	set_epspivot(lp, DEF_EPSPIVOT);
	set_print_duals(lp, FALSE);
	set_debug(lp, FALSE);
	set_floor_first(lp, TRUE);
	set_print_at_invert(lp, FALSE);
	set_trace(lp, FALSE);
	set_anti_degen(lp, FALSE);
	set_do_presolve(lp, FALSE);
	set_improve(lp, IMPROVE_NONE);
	set_scalemode(lp, MMSCALING);
	set_bb_rule(lp, FIRST_SELECT);*/
	int fail = ::solve(lp);

	// Record the result
	int result = false;
	if(fail == OPTIMAL) {
		result = true;

		// Record variables values
		for(elm::genstruct::HashTable<ilp::Var *, Var *>::ItemIterator var(vars);
		var; var++)
			var->setValue((double)lp->best_solution[lp->rows + var->column()]);

		// Get optimization result
		val = (double)lp->best_solution[0];

		// lp_solve seems to be buggy, so we recompute the max
		//val = get_objective(lp);

		double sum = 0;
		for(Constraint::Factor *fact = ofun->facts; fact; fact = fact->next()) {
			if(fact->variable())
				sum += fact->coefficient() * fact->variable()->value();
			else
				sum += fact->coefficient();
		}
		val = sum;
	}

	// Clean up
	/*print_lp(lp);*/
	//print_solution(lp);
	//write_MPS(lp, stdout);
	delete_lp(lp);
	return result;
}


/**
 */
int System::countVars(void) {
	return cols;
}


/**
 */
int System::countConstraints(void) {
	return rows;
}


/**
 */
void System::exportLP(io::Output& out) {
	static CString texts[] = { "<", "<=", "=", ">=", ">" };
	out << "/* IPET system */\n";

	// Output the objective function
	if(ofun->comparator() >= 0)
		out << "max: ";
	else
		out << "min: ";
	ofun->dump(out);
	out << ";\n";

	// Output the constraints
	for(Constraint *cons = conss; cons; cons = cons->next()) {
		cons->dump(out);
		out << " " << texts[cons->comparator() + 2]
			<< " " << (int)cons->constant() << ";\n";
	}

	// Output int constraints
	for(genstruct::HashTable<ilp::Var *, Var *>::ItemIterator var(vars);
	var; var++)
		out << "int " << var->makeVarName() << ";\n";
}


/**
 */
void System::dumpSolution(io::Output& out) {
	out << "/* IPET solution */\n";
	for(genstruct::HashTable<ilp::Var *, Var *>::ItemIterator var(vars);
	var; var++)
		out << var->makeVarName() << " = "
			<< (int)var->value() << io::endl;
}


class LocalVar: public ilp::Var {
public:
	inline LocalVar(const string& name): ilp::Var(name) { }
};


/**
 */
ilp::Var *System::newVar(const string& name) {
	return new LocalVar(name);
}

} }	// otawa::lp_solve

