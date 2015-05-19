/*
 *	cplex plugin implementation
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

#include <elm/assert.h>
#include <elm/io.h>
#include <otawa/ilp.h>
#include <otawa/ilp/AbstractSystem.h>
#include <otawa/ilp/ILPPlugin.h>
#include <otawa/prog/WorkSpace.h>
#include <elm/debug.h>

#include <ilcplex/ilocplex.h>
#include <sstream>

namespace otawa { namespace cplex {

class System: public ilp::AbstractSystem {
public:
	System(IloEnv& env): _env(env), solver(0), vals(0) {
	}

	virtual ~System(void) {
		if(solver)
			delete solver;
		if(vals)
			delete vals;
	}

	virtual bool solve(WorkSpace *ws) {
		IloModel model(_env);

		// build the variable
		IloNumVarArray vars(_env);
		for(VarIter var(this); var; var++) {
			switch(var->type()) {
			case ilp::Var::BIN:		vars.add(IloNumVar(_env, 0, 1, IloNumVar::Int)); break;
			case ilp::Var::INT:		vars.add(IloNumVar(_env, 0, +IloInfinity, IloNumVar::Int)); break;
			case ilp::Var::FLOAT:	vars.add(IloNumVar(_env)); break;
			default:				ASSERTP(false,  "unknown type " << int(var->type()) << " for " << var->name()); break;
			}
			const elm::string& name = var->name();
			if(name)
				vars[vars.getSize() - 1].setName(name.toCString());
		}

		// build the objective function
		{
			IloNumExpr expr(_env, 0);
			for(ObjTermIterator term(this); term; term++)
				expr += (*term).snd * vars[index((*term).fst)];
			IloObjective obj = IloMaximize(_env, expr);
			//std::cout << "DEBUG: " << obj << io::endl;
			model.add(obj);
			//std::cout << "DEBUG: " << model << io::endl;
		}

		// build the constraints
		for(AbstractSystem::ConstIter cons(this); cons; cons++) {
			IloNumExpr expr(_env);
			for(ilp::AbstractConstraint::TermIter term(cons); term; term++) {
				ASSERT(index((*term).fst) < vars.getSize());
				expr += (*term).snd * vars[index((*term).fst)];
			}
			switch(cons->comparator()) {
			case ilp::Constraint::LT:	model.add(expr <  cons->constant()); break;
			case ilp::Constraint::LE:	model.add(expr <= cons->constant()); break;
			case ilp::Constraint::EQ:	model.add(expr == cons->constant()); break;
			case ilp::Constraint::GE:	model.add(expr >  cons->constant()); break;
			case ilp::Constraint::GT:	model.add(expr >= cons->constant()); break;
			default:					ASSERTP(false, "cplex: unknown comparator " << cons->comparator());
			}
		}

		// create the solver
		if(solver)
			delete solver;
		solver = new IloCplex(model);

		// look for the solution
		try {
			TRACE;
			bool res = solver->solve();

			// process error
			if(!res) {
				stringstream s;
				s << "ERROR: cannot resolve: " << solver->getCplexStatus();
				last_message = s.str().c_str();
			}

			// else get the results
			else {
				vals = new IloNumArray(_env);
				solver->getValues(vars, *vals);
			}

			// return now
			return res;
		}
		catch(IloException& e) {
			last_message = e.getMessage();
			return false;
		}
	}

	virtual double valueOf(ilp::Var *var) {
		ASSERTP(vals, "cplex: unsolved system");
		return (*vals)[index(var)];
	}

	virtual double value(void) {
		ASSERTP(vals, "cplex: solution not computed");
		return solver->getObjValue();
	}

private:
	IloEnv& _env;
    IloCplex *solver;
    IloNumArray *vals;
    elm::string last_message;
};

// Plugin class
class Plugin: public ilp::ILPPlugin {
public:
	Plugin(void): ILPPlugin("cplex", Version(1, 0, 0), OTAWA_ILP_VERSION) {
		_env.setNormalizer(IloTrue);
	}

	// ILPPlugin overload
	virtual ilp::System *newSystem(void) {
		elm::cerr << "DEBUG: making the system\n";
		return new System(_env);
	}

private:
	IloEnv _env;
};

} }		// otawa::cplex

otawa::cplex::Plugin OTAWA_ILP_HOOK;
otawa::ilp::ILPPlugin& cplex_plugin = OTAWA_ILP_HOOK;
