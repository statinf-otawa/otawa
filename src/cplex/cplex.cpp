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
#include <otawa/prog/Manager.h>
#include <otawa/prog/WorkSpace.h>
#include <elm/debug.h>

#include <ilcplex/ilocplex.h>
#include <sstream>

namespace otawa { namespace cplex {

class OStreamBuf: public streambuf {
public:
	OStreamBuf(Monitor& m): mon(m), eol(true) { }
	virtual ~OStreamBuf(void) { }

	virtual int overflow(int ch) {
		if(!mon.logFor(Monitor::LOG_FILE))
			return ch;
		if(eol)
			mon.log << "\t\t";
		mon.log << char(ch);
		eol = ch == '\n';
		return ch;
	}
	virtual int underflow(void) { return EOF; }
	virtual int sync(void) { return 0; }
private:
	Monitor& mon;
	bool eol;
};

/**
 * CPlex system class.
 */
class System: public ilp::AbstractSystem {
public:

	/**
	 */
	System(ilp::ILPPlugin *plugin, bool max): vals(0), result(0), _plugin(plugin) {
	}

	/**
	 */
	~System(void) {
		if(vals)
			delete vals;
	}

	/**
	 */
	bool solve(WorkSpace *ws) override {
		Monitor mon;
		return solve(ws, mon);
	}


	/**
	 */
	elm::string lastErrorMessage(void) override {
		return this->last_message;
	}

	/**
	 */
	ilp::ILPPlugin *plugin(void) override {
		return _plugin;
	}

	bool solve(WorkSpace *ws, otawa::Monitor& mon) override {

		// prepare environment
		IloEnv _env;
		_env.setNormalizer(IloTrue);
		OStreamBuf sbuf(mon);
		iostream out(&sbuf);
		_env.setError(out);
		_env.setOut(out);

		// prepare environment
		IloModel model(_env);

		// build the variable
		IloNumVarArray vars(_env);
		for(VarIter var(this); var(); var++) {
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
			for(ObjTermIterator term(this); term(); term++)
				expr += (*term).snd * vars[index((*term).fst)];
			IloObjective obj;
			if(isMaximizing())
				obj = IloMaximize(_env, expr);
			else
				obj = IloMinimize(_env, expr);
			model.add(obj);
		}

		// build the constraints
		for(AbstractSystem::ConstIter cons(this); cons(); cons++) {
			IloNumExpr expr(_env);
			for(ilp::AbstractConstraint::TermIter term(*cons); term(); term++) {
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
		IloCplex solver(model);

		// look for the solution
		try {
			bool res = solver.solve();

			// process error
			if(!res) {
				stringstream s;
				mon.log << "ERROR: cannot resolve: " << solver.getCplexStatus();
				last_message = s.str().c_str();
			}

			// else get the results
			else {
				result = solver.getObjValue();
				vals = new IloNumArray(_env);
				solver.getValues(vars, *vals);
			}

			// return now
			return res;
		}
		catch(IloException& e) {
			last_message = e.getMessage();
			mon.log << "ERROR: " << last_message << io::endl;
			return false;
		}
	}

	double valueOf(ilp::Var *var) override {
		ASSERTP(vals, "cplex: unsolved system");
		return (*vals)[index(var)];
	}

	double value() override {
		ASSERTP(vals, "cplex: solution not computed");
		return result;
	}

private:
    IloNumArray *vals;
	double result;
    elm::string last_message;
    ilp::ILPPlugin *_plugin;
};

// Plugin class
class Plugin: public ilp::ILPPlugin {
public:
	Plugin(void): ILPPlugin(make("cplex", OTAWA_ILP_VERSION)
		.version(Version(1, 0, 0))
		.description("ILP plugin based on CPlex")
		.license(otawa::Manager::copyright)) { }

	ilp::System *newSystem(bool max) override { return new System(this, max); }

};

} }		// otawa::cplex

otawa::cplex::Plugin otawa_cplex;
ELM_PLUGIN(otawa_cplex, OTAWA_ILP_HOOK);
int cplex_plugin;
