/*
 *	$Id$
 *	Copyright (c) 2005, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/lp_solve/LPSolveSystem.cpp -- LPSolveSystem class definition.
 */

#include <assert.h>
#include <stdio.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
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
	while(cons)
		delete cons;
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
		add_constraint(lp, row, comps[comp + 2], cst + corr[comp + 2]);
		cons->resetRow(row);
	}
	
	// Launch the resolution
	set_print_sol(lp, FALSE);
	set_epsilon(lp, DEF_EPSILON);
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
	set_bb_rule(lp, FIRST_SELECT);
	int fail = ::solve(lp);
	
	// Record the result
	cout << "FAIL = " << fail << "\n";
	int result = false;
	if(fail == OPTIMAL) {
		//val = (double)lp->best_solution[0];
		val = get_objective(lp);
		for(elm::genstruct::HashTable<ilp::Var *, Var *>::ItemIterator var(vars);
		var; var++)
			var->setValue((double)lp->best_solution[lp->rows + var->column()]);
		result = true;
	}
	
	// Clean up
	/*print_lp(lp);
	//print_solution(lp);*/
	//write_MPS(lp, stdout);
	delete_lp(lp);
	return result;
}

/**
 * !!TODO!!
 * Variable by this way cannot be fried. Must be fixed.
 */
ilp::Var *System::newVar(elm::String name) {
	return new ilp::Var(name);
}


/**
 */
void System::dump(elm::io::OutStream& _out) {
	static CString texts[] = { "<", "<=", "=", ">=", ">" };
	elm::io::Output out(_out);
	
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
	
	// Output solution
	genstruct::Vector<ilp::Var *> ilp_vars;
	for(genstruct::HashTable<ilp::Var *, Var *>::KeyIterator var(vars); var; var++)
		ilp_vars.add(*var);
	for(int i = 0; i < ilp_vars.length(); i++) {
		out << ilp_vars[i]->name() << " = ";
		/*printf("!%g!\n", valueOf(ilp_vars[i]));
	}*/
		cout << (int)valueOf(ilp_vars[i]) << "\n";
	}
	
}

} }	// otawa::lp_solve

