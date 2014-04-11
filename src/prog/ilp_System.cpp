/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ilp_System.h -- documentation of the system interface.
 */

#include <otawa/otawa.h>
#include <otawa/ilp/System.h>
#include <math.h>

namespace otawa { namespace ilp {

/**
 * @class System
 * An ILP system is a colletion of ILP constraint that may maximize or minimize
 * some object function.
 */


/**
 * @fn Constraint *System::newConstraint(Constraint::comparator_t comp, double constant = 0);
 * Build a new constraint that may be initialized by the user. As the ILP system
 * manage the memory of the constraint, the constraint must never be deleted
 * by the user.
 * @param comp		Comparator used (one of Constraint::LT, LE, EQ, GT, GE).
 * @param constant	Constant value.
 * @return			Built constraint.
 */


/**
 * @fn boolean System::solve(WorkSpace *ws);
 * Solve the ILP system.
 * @param ws	If passed (default to null), the given workspace is used to test
 * 				if the computation is cancelled in order to stop the solver.
 * @return		True if the resolution is succesful or false else (mainly due
 * 				to lack of constraint).
 */


/**
 * @fn void System::addObjectFunction(double coef, Var *var = 0);
 * Add a factor to the object function.
 * @param coef	Coefficient of the factor.
 * @param var	Variable of the factor.
 */


/**
 * @fn double System::valueOf(Var *var);
 * This method can only be called after the resolution of the system and returns
 * the value of a variable. It is an error to pass a variable not involved in
 * the system.
 * @param var	Variable whose value is looked for.
 * @return		Value of the variable.
 */


/**
 * @fn double System::value(void);
 * Return the value of the optimized object function.
 * @return	Object function optimum.
 */


/**
 * Dump the system to the given output. The dumping format depends upon the
 * actual used ILP engine. Usually, it is compatible with other tools of
 * the ILP engine.
 * @param out	Used output.
 * @deprecated Use dumpSystem() and dumpSolution() instead.
 */
void System::dump(elm::io::OutStream& out) {
	io::Output output(out);
	dumpSystem(output);
	dumpSolution(output);
}

/**
 * Dump the system to the given output and format. 
 * @param out	Used output.
 * @param fmt 	Used format
 */
void System::dump(format_t fmt, elm::io::OutStream& out) {
	io::Output output(out);
	switch(fmt) {
		case DEFAULT:
		case LP_SOLVE: 

			dumpSystem(output);
			break;
		case CPLEX:
		{
			
			HashTable<Var*, String*> rename;
			int idx = 0;
			/* Rename the variables for cplex */ 
			
			
			output << "\\* Objective function *\\\n";
			output << "Maximize\n";
			/* dump the objective function */
			
			for (ObjTermIterator term(this); term; term++) {
				t::int32 val = lrint((*term).snd);
				if (!rename.hasKey((*term).fst)) {
					rename.put((*term).fst, new String((_ << "x" << idx)));
					idx++;
				}
				if (val == 0)
					continue;
				
				output << " ";
				if (val == 1) {
					output << "+" << *rename.get((*term).fst, NULL);
				} else if (val == -1) {
					output << "-" << *rename.get((*term).fst, NULL);
				} else {
					output << io::sign(val) << " " << *rename.get((*term).fst, NULL);
				}
				output << "\n";
							
			} 
				

			output << "\n\n";
			output << "\\* Constraints *\\\n";
			output << "Subject To\n";		
			/* dump the constraints */
			for (ConstIterator cons2(this); cons2; cons2++) {
				bool bound = true;
				int numvar = 0;
				for (Constraint::TermIterator term(cons2); term; term++) {
					if ((*term).snd != 1)
						bound = false;
					numvar++;
				}
				if (numvar != 1)
					bound = false;
				
				if (bound)
					continue;
				
				for (Constraint::TermIterator term(cons2); term; term++) {
					t::int32 val = lrint((*term).snd);
					if (!rename.hasKey((*term).fst)) {
						rename.put((*term).fst, new String((_ << "x" << idx)));
						idx++;
					}					
					if (val == 0)
						continue;
					
					output << " ";
					if (val == 1) {
						output << "+" << *rename.get((*term).fst, NULL);
					} else if (val == -1) {
						output << "-" << *rename.get((*term).fst, NULL);
					} else {
						output << io::sign(val) << " " << *rename.get((*term).fst, NULL);
					}
							
				}
				switch(cons2->comparator()) {
					case Constraint::LT:
						output << " < ";
						break;
					case Constraint::LE:
						output << " <= ";
						break;
					case Constraint::EQ:
						output << " = ";
						break;
					case Constraint::GE:
						output << " >= ";
						break;
					case Constraint::GT:
						output << " > ";
						break;
					default:
						output << " ?? ";; 
						break;
				}
				
				output << cons2->constant() << "\n";
			}				
			output << "\n\n";
			output << "\\* Variable bounds *\\\n";
			output << "Bounds\n";		
			/* dump the bounds */
			for (ConstIterator cons2(this); cons2; cons2++) {
				bool bound = true;
				int numvar = 0;
				Var *var;
				for (Constraint::TermIterator term(cons2); term; term++) {
					if ((*term).snd != 1)
						bound = false;
					numvar++;
					var = (*term).fst;
				}
				if (numvar != 1)
					bound = false;
				
				if (!bound)
					continue;
				
				output << " " << *rename.get(var, NULL);
				switch(cons2->comparator()) {
					case Constraint::LT:
						output << " < ";
						break;
					case Constraint::LE:
						output << " <= ";
						break;
					case Constraint::EQ:
						output << " = ";
						break;
					case Constraint::GE:
						output << " >= ";
						break;
					case Constraint::GT:
						output << " > ";
						break;
					default:
						output << " ?? ";; 
						break;
				}
				
				output << cons2->constant() << "\n";
			}				

			output << "\n\n";
			output << "\\* Integer definitions *\\\n";
			output << "Integer\n";
				
			/* dump the integer variable definition */
			
			for (HashTable<Var*, String*>::Iterator item(rename); item; item++) {
				String *str = *item;				
				output << " ";
				output << *str;
				delete str;			
				output << "\n";

			}

				
			output << "\n\n";
			output << "End\n";
			break;
		}
case MOSEK:
		{
			HashTable<Var*, String*> rename;
			int idx = 0;
			/* Rename the variables for cplex */ 
			
			
			output << "[objective maximize 'obj']\n";
			
			/* dump the objective function */
			
			for (ObjTermIterator term(this); term; term++) {
				t::int32 val = lrint((*term).snd);
				if (!rename.hasKey((*term).fst)) {
					rename.put((*term).fst, new String((_ << "x" << idx)));
					idx++;
				}
				if (val == 0)
					continue;
				
				output << " ";
				if (val == 1) {
					output << "+ " << *rename.get((*term).fst, NULL);
				} else if (val == -1) {
					output << "- " << *rename.get((*term).fst, NULL);
				} else {
					output << io::sign(val) << " " << *rename.get((*term).fst, NULL);
				}				
			} 
				
			output << "\n";
			output << "[/objective]\n";
			output << "[constraints]\n";		
			/* dump the constraints */
			for (ConstIterator cons2(this); cons2; cons2++) {
				output << "[con]";
				bool bound = true;
				int numvar = 0;
				for (Constraint::TermIterator term(cons2); term; term++) {
					if ((*term).snd != 1)
						bound = false;
					numvar++;
				}
				if (numvar != 1)
					bound = false;
				
				for (Constraint::TermIterator term(cons2); term; term++) {
					t::int32 val = lrint((*term).snd);
					if (!rename.hasKey((*term).fst)) {
						rename.put((*term).fst, new String((_ << "x" << idx)));
						idx++;
					}					
					if (val == 0)
						continue;
					
					output << " ";
					if (val == 1) {
						output << "+" << *rename.get((*term).fst, NULL);
					} else if (val == -1) {
						output << "-" << *rename.get((*term).fst, NULL);
					} else {
						output << io::sign(val) << " " << *rename.get((*term).fst, NULL);
					}
							
				}
				switch(cons2->comparator()) {
					case Constraint::LT:
						output << " < ";
						break;
					case Constraint::LE:
						output << " <= ";
						break;
					case Constraint::EQ:
						output << " = ";
						break;
					case Constraint::GE:
						output << " >= ";
						break;
					case Constraint::GT:
						output << " > ";
						break;
					default:
						output << " ?? ";; 
						break;
				}
				
				output << cons2->constant();
				output << " [/con] \n";
			}	
			output << "\n";
			output << "[/constraints]\n";
			output << "[bounds]\n";
			output << "[b] 0 <= * [/b]\n";
			output << "[/bounds]\n";			
			
			output << "[variables]\n";	
			/* dump the integer variable definition */
			
			for (HashTable<Var*, String*>::Iterator item(rename); item; item++) {
				String *str = *item;				
				output << " ";
				output << *str;	
				output << "\n";

			}	
			output << "\n";			
			output << "[/variables]\n";
			output << "[integer]\n";
				
			/* dump the integer variable definition */
			
			for (HashTable<Var*, String*>::Iterator item(rename); item; item++) {
				String *str = *item;				
				output << " ";
				output << *str;
				delete str;			
				output << "\n";

			}	
			output << "\n";			
			output << "[/integer]\n";			
			break;
		}		
		default:
			ASSERTP(false, "Unsopported ILP system format.");
			break;
	}
}

/**
 * Tests if it is possible to dump in the given format.
 * @param fmt 	Used format
 * @return	Boolean
 */
bool System::hasDump(format_t fmt) {
	switch(fmt) {
		case DEFAULT:
		case LP_SOLVE:
		case CPLEX:
			return true;
			break;
		default:
			return false;
			break;
	}
}

/**
 * @fn int System::countVars(void);
 * Count the number of variables in the system.
 * @return	Variable count.
 */


/**
 * @fn int System::countConstraints(void);
 * Count the number of constraints in the system.
 * @return	Constraint count.
 */


/**
 * @fn void System::exportLP(io::Output& out);
 * Export the system to the given output using the LP format (lp_solve).
 * @param out	Stream to export to (default to cout).
 */


/**
 * Dump the system in text format (as a default, call exportLP).
 * @param out	Stream to dump to (default to cout).
 */
void System::dumpSystem(io::Output& out) {
	exportLP(out);
}


/**
 * @fn void System::dumpSolution(io::Output& out);
 * Dump the solution textually to the given output.
 * @param out	Stream to output to (default to cout).
 */


/**
 * @fn Var *System::newVar(cstring name = "");
 * Build a new variable.
 * @param name	Name of the variable (may be an empty string for an anonymous
 *				variable).
 * @return		Built variable.
 */


/**
 * Var *System::newVar(const string& name);
 * Build a new variable.
 * @param name	Name of the variable (may be an empty string for an anonymous
 *				variable).
 * @return		Built variable.
 */


/**
 * Build a named constraint.
 * @param label		Label of the constraint.
 * @param comp		Used comparator (one of Constraint::LT, LE, EQ, GT, GE).
 * @param constant	Used constant.
 * @return			Built constraint.
 */
/*Constraint *System::newConstraint(const string& label, Constraint::comparator_t comp, double constant) {
	return newConstraint(comp, constant);
}*/

} // ilp

} // otawa
