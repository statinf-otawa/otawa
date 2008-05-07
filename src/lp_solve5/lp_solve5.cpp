/*
 *	$Id$
 *	lp_solve5 ILP plugin implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <elm/assert.h>
#include <elm/io.h>
#include <otawa/ilp.h>
#include <elm/genstruct/HashTable.h>
#include <elm/genstruct/Vector.h>
#include <otawa/ilp/ILPPlugin.h>
#include <lp_lib.h>
#include <math.h>

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

namespace otawa { namespace lp_solve5 {

// Predeclaration
class Constraint;
class System;
class Var;


/**
 * Constraint implementation for the lp_solve library, version 5.
 * Only stores information about the constraint:
 * <ul>
 * <li>constant value,</li>
 * <li>comparator,</li>
 * <li>factor (variables and coefficient).</li>
 * </ul>
 */
class Constraint: public ilp::Constraint {
public:
	inline Constraint(System *sys, comparator_t comp, double constant,
		Constraint *next);
	~Constraint(void);
	inline double constant(void) const;
	inline Constraint *next(void) const;
	void fillRow(double *row);
	void resetRow(double *row);
	void dump(elm::io::Output& out);

	// ilp::Constraint overload	
	virtual double coefficient(ilp::Var *var = 0) const;
	virtual comparator_t comparator(void) const;
	virtual void add(double coef, ilp::Var *var = 0);
	virtual void sub(double coef, ilp::Var *var = 0);
	elm::IteratorInst<ilp::Constraint::Term> *terms(void);

private:
	friend class System;
	
	
	
	// Factor class
	class Factor {
	public:
		inline Factor(double coefficient, Var *variable, Factor *next, ilp::Var *_ilpvar);
		inline Var *variable(void) const;
		virtual ilp::Var *getVar(void) const;
		inline double coefficient(void) const;
		inline Factor *next(void) const;
		inline void add(double value);
		inline Factor& operator+=(double value);
		inline Factor& operator-=(double value);
		inline ~Factor() { }
	private:
		Factor *nxt;
		Var *var;
		ilp::Var *ilpvar;
		double coef;
	};
	
	Factor *getFacts();
	// TermIterInst class
	class TermIterInst: public elm::IteratorInst<ilp::Constraint::Term> {
	
		Factor *cur;
		public:
		TermIterInst(Constraint *_cons) : cur(_cons->getFacts()) {
		}
		
		bool ended (void) const {
			return (!cur);
		}
		
		ilp::Constraint::Term item(void) const {
			return(elm::pair(cur->getVar(), cur->coefficient()));

		}
		
		void next(void) {
			cur = cur->next();
			
		}
	};
	// Attributes
	System *sys;
	Constraint *nxt;
	Factor *facts;
	double cst;
	comparator_t cmp;
};


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
	string makeVarName(void) const {
		if(var->name())
			return var->name();
		else
			return _ << '_' << col;
	}
};


/**
 * This the lp_solve5 version of the ilp::System class.
 * As lp_solve is very bound in its API, this class mainly gather information
 * about the ILP system and, at the computation time, it builds the lp_solve
 * data structure and launch the computation.
 */
class System: public ilp::System {
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
	virtual int countVars(void);
	virtual int countConstraints(void);
	virtual void exportLP(io::Output& out = elm::cout);
	virtual void dumpSolution(io::Output& out = elm::cout);
	elm::IteratorInst<ilp::Constraint*> *constraints(void);
	elm::IteratorInst<ilp::Constraint::Term> *objTerms(void);
	class LocalVar: public ilp::Var {
	public:
		inline LocalVar(const string& name): ilp::Var(name) { }
	};
	virtual ilp::Var *newVar(const string& name) { return new LocalVar(name); }

private:	
	friend class Constraint;
	
	Constraint *getConss(void);
	// ConstIterInst
	class ConstIterInst: public elm::IteratorInst<ilp::Constraint*> {
		System *sys;
		Constraint *cur;
		public:
		ConstIterInst(System *_sys) : sys(_sys), cur(_sys->getConss()) {
		}
		
		bool ended (void) const {
			return (!cur);	
		}
		
		ilp::Constraint* item(void) const {
			return(cur);
		}
		
		void next(void) {
			cur = cur->next();
		}
	};	
	

	elm::genstruct::HashTable<ilp::Var *, Var *> vars;
	Constraint *conss;
	Constraint *ofun;
	int cols, rows;
	double val;

	void removeConstraint(Constraint *cons);
};

inline Constraint *System::getConss(void) {
	return(conss);
}

ilp::Var *Constraint::Factor::getVar(void) const {
	return ilpvar;
}

// Constraint::Factor inlines
inline Constraint::Factor::Factor(double coefficient, Var *variable, 
Factor *next, ilp::Var *_ilpvar) : nxt(next), var(variable), ilpvar(_ilpvar), coef(coefficient)   {
}

inline Var *Constraint::Factor::variable(void) const {
	return var;
}

inline double Constraint::Factor::coefficient(void) const {
	return coef;
}

inline void Constraint::Factor::add(double value) {
	coef += value;
}

inline Constraint::Factor& Constraint::Factor::operator+=(double value) {
	add(value);
	return *this;
}

inline Constraint::Factor& Constraint::Factor::operator-=(double value) {
	add(-value);
	return *this;
}

// Constraint inlines
inline Constraint::Constraint(System *system, ilp::Constraint::comparator_t comp,
double constant, Constraint *next): sys(system), nxt(next), facts(0),
cst(constant), cmp(comp) {
	ASSERT(sys);
}

inline Constraint::Factor *Constraint::getFacts(void) {
	return facts;
}

inline double Constraint::constant(void) const {
	return cst;
}

inline Constraint::Factor *Constraint::Factor::next(void) const {
	return nxt;
}

elm::IteratorInst<ilp::Constraint::Term> *Constraint::terms(void) {
		 return(new TermIterInst(this)); 
}

inline Constraint *Constraint::next(void) const {
	return nxt;
}

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


/**
 * Free all ressources.
 */
Constraint::~Constraint(void) {
	Factor *fact = facts;
	while(fact) {
		Factor *next = fact->next();
		delete fact;
		fact = next;
	}
	sys->removeConstraint(this);
}


// Overload
double Constraint::coefficient(ilp::Var *var) const {
	ASSERT(var);
	Var *lvar = sys->getVar(var);
	ASSERT(lvar);
	for(Factor *fact = facts; fact; fact = fact->next())
		if(fact->variable() == lvar)
			return fact->coefficient();
	ASSERT(false);
	return 0;
}

// Overload
Constraint::comparator_t Constraint::comparator(void) const {
	return cmp;
}

// Overload
/* !!NOTE!!
 * Collection of factors would be more fast if they was stored in a dynamic
 * vector. Not done now for debugging purpose but should be performed.
 * Another interesting optimization should be to remove factor whose coefficient
 * become null.
 */
void Constraint::add(double coef, ilp::Var *var) {
	if(var == 0)
		cst -= coef;
	else {
		Var *lvar = sys->getVar(var);
		ASSERT(lvar);
		for(Factor *fact = facts; fact; fact = fact->next())
			if(fact->variable() == lvar) {
				fact->add(coef);
				return;
			}
		facts = new Factor(coef, lvar, facts, var);
	}
}

/**
 * Fill a row of the lp_solve matrice with the current constraint.
 * @param row	Row to fill.
 */
void Constraint::fillRow(double *row) {
	ASSERT(row);
	for(Factor *fact = facts; fact; fact = fact->next())
		row[fact->variable()->column()] = fact->coefficient();
}


/**
 * Reset the value used in the given row by the current constraint.
 * @param row	Row to reset.
 */
void Constraint::resetRow(double *row) {
	ASSERT(row);
	for(Factor *fact = facts; fact; fact = fact->next())
		row[fact->variable()->column()] = 0;
}


// Overload
void Constraint::sub(double coef, ilp::Var *var) {
	add(-coef, var);
}


/**
 * Dump the constraint to the given output.
 * @param out	Used output.
 */
void Constraint::dump(elm::io::Output& out) {
	bool first = true;
	for(Factor *fact = facts; fact; fact = fact->next()) {
		if(first)
			first = false;
		else
			out << " + ";
		out << (int)fact->coefficient() << " ";
		if(fact->variable()->variable()->name())
			out << fact->variable()->variable()->name();
		else
			out << "_" << fact->variable()->column();
	}
}

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

elm::IteratorInst<ilp::Constraint*> *System::constraints(void) {
	 return(new ConstIterInst(this)); 
}

elm::IteratorInst<ilp::Constraint::Term> *System::objTerms(void) {
	 return(new Constraint::TermIterInst(ofun)); 
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
bool System::solve(void) {
	static short comps[] = { LE, LE, EQ, GE, GE };
	static double corr[] = { -1, 0, 0, 0, +1 };
	
	// Allocate and initialize the lp_solve data structure
	lprec *lp = make_lp(0, cols);
	set_verbose(lp, IMPORTANT);
	REAL row[cols + 1];
	for(int i = 1; i <= cols; i++) {
		row[i] = 0;
		set_int(lp, i, TRUE);
	}
	
	// Build the object function
	ofun->fillRow(row);
	row[0] = 0;
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
		/*cout << "=> ";
		for(int i = 0; i <= cols; i++)
			cout << row[i] << '\t';
		cout << io::endl;*/
		cons->resetRow(row);
	}
	
	// Launch the resolution
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
		//cout << "=> " << get_objective(lp) << " <=> " << int(get_objective(lp)) << "<=\n";
		val = rint(get_objective(lp));
	}
	
	// Clean up
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


// LPSolvePlugin class
class Plugin: public ilp::ILPPlugin {
public:
	Plugin(void);
	
	// ILPPlugin overload
	virtual System *newSystem(void);
};


/**
 * Build the plugin.
 */
Plugin::Plugin(void)
: ILPPlugin("lp_solve5", Version(1, 1, 0), OTAWA_ILP_VERSION) {
}


/**
 */
System *Plugin::newSystem(void) {
	return new System();
}

} } // otawa::lp_solve5

/**
 * Define the actual plugin.
 */
otawa::lp_solve5::Plugin OTAWA_ILP_HOOK;
otawa::ilp::ILPPlugin& lp_solve5_plugin = OTAWA_ILP_HOOK;
