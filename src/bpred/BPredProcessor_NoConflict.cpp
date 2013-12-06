/*
 *	$Id$
 *	BPredProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <otawa/cfg.h>
#include <otawa/bpred/BPredProcessor.h>
#include <otawa/ipet/BasicConstraintsBuilder.h> // Contraintes de base feature
#include <otawa/ipet/ILPSystemGetter.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp/System.h>

using namespace otawa::ilp;
using namespace elm;

namespace otawa { namespace bpred {

/////// CREATION DE CONTRAINTES ET VARIABLES
#define NEW_BASIC_CONSTRAINT(cons_name) Constraint *cons_name = system->newConstraint(Constraint::EQ); \
		ASSERT(cons_name);

#define NEW_SPECIAL_CONSTRAINT(cons_name,op,val) 	Constraint *cons_name = system->newConstraint(Constraint::op, val); \
		ASSERT(cons_name);

#define NEW_VAR_FROM_BUFF(var_name,buff_expr)	{StringBuffer sb##var_name; \
												sb##var_name << buff_expr; \
												String s##var_name = sb##var_name.toString(); \
												if(this->explicit_mode) var_name = system->newVar(s##var_name); \
												else var_name = system->newVar(String("")); \
												ASSERT(var_name);}
////////////////////////////////////////////


/**
 * Generate the constraints system for a given BasicBlock and workspace according to the "no conflict 2bits-counter" method.
 * 
 * @param fw	current workspace.
 * @param bb	BasicBlock to create the constraints system from.
 */
void BPredProcessor::CS__NoConflict_2bCounter(WorkSpace* fw,BasicBlock* bb) {
	// Recuperation de l'ensemble des contraintes
	System *system = ipet::SYSTEM(fw);
	ASSERT(system);

	// Recuperation de la variable associee au BB
	Var *Xi = ipet::VAR(bb);
	ASSERT(Xi);
	
	// Creation des nouvelles contraintes
		// BLOC
		NEW_BASIC_CONSTRAINT(cons_bloc);
		// OUT
		NEW_BASIC_CONSTRAINT(cons_out_00);
		NEW_BASIC_CONSTRAINT(cons_out_01);
		NEW_BASIC_CONSTRAINT(cons_out_10);
		NEW_BASIC_CONSTRAINT(cons_out_11);
		// IN
		NEW_BASIC_CONSTRAINT(cons_in_00);
		NEW_BASIC_CONSTRAINT(cons_in_01);
		NEW_BASIC_CONSTRAINT(cons_in_10);
		NEW_BASIC_CONSTRAINT(cons_in_11);
		// EDGES
		NEW_BASIC_CONSTRAINT(cons_edge_T);
		NEW_BASIC_CONSTRAINT(cons_edge_NT);
		// STARTS
		NEW_SPECIAL_CONSTRAINT(cons_start,EQ,1);
		// ENDS
		NEW_SPECIAL_CONSTRAINT(cons_end,EQ,1);
		// ENDS_XX
		NEW_BASIC_CONSTRAINT(cons_end_00);
		NEW_BASIC_CONSTRAINT(cons_end_01);
		NEW_BASIC_CONSTRAINT(cons_end_10);
		NEW_BASIC_CONSTRAINT(cons_end_11);
		// WRONGS
		NEW_BASIC_CONSTRAINT(cons_wrong_T);
		NEW_BASIC_CONSTRAINT(cons_wrong_NT);
		
	Var *XiCYY, 
		*XiCYYd0,		*XiCYYd1, 
		*XiCYYstart,	*XiCYYend, 
		*XiCYYd0end,
		*XiCYYd1end, 
		*EdgeT,			*EdgeNT,
		*WrongT,		*WrongNT;

	

	// Definition des contraintes
		// bloc
		cons_bloc->addLeft(1,Xi);

		// edge
			// creation des variables
			for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
				if(edge->kind() == Edge::TAKEN) {// WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
					EdgeT = ipet::VAR( edge);
					assert(EdgeT);
				}
				else if(edge->kind() == Edge::NOT_TAKEN) {// WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
					EdgeNT = ipet::VAR(edge);
					ASSERT(EdgeNT);
					//					NEW_VAR_FROM_BUFF(EdgeNT, 	"e" << bb->number() << "_" << edge->target()->number());
				}
			}
			// ajout des variables aux contraintes edge_T et edge_NT
			cons_edge_T->addLeft(1,EdgeT);
			cons_edge_NT->addLeft(1,EdgeNT);
		
		// wrong
			// creation des variables
			for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
				if(edge->kind() == Edge::TAKEN) { // WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
					NEW_VAR_FROM_BUFF(WrongT,	"m" << bb->number() << "_" << edge->target()->number())
				}
				else if(edge->kind() == Edge::NOT_TAKEN) {// WARNING ces accolades sont IMPERATIVES car NEW_VAR_FROM_BUFF definit un bloc
					NEW_VAR_FROM_BUFF(WrongNT,	"m" << bb->number() << "_" << edge->target()->number())
				}
			}
			
			system->addObjectFunction(5.0, WrongT);
			system->addObjectFunction(5.0, WrongNT);
			// ajout des variables aux contraintes wrong_T et wrong_NT
			cons_wrong_T->addLeft(1,WrongT);
			cons_wrong_NT->addLeft(1,WrongNT);
			
			
	for(int i = 0 ; i < 4 ; ++i) {
		// Creation des nouvelles variables
		NEW_VAR_FROM_BUFF(XiCYY ,		Xi->name() << "C" << bin_to_str(i) );
		NEW_VAR_FROM_BUFF(XiCYYd0,		Xi->name() << "C" << bin_to_str(i) << "d0");
		NEW_VAR_FROM_BUFF(XiCYYd1,		Xi->name() << "C" << bin_to_str(i) << "d1");
		NEW_VAR_FROM_BUFF(XiCYYstart,	Xi->name() << "C" << bin_to_str(i) << "start");
		NEW_VAR_FROM_BUFF(XiCYYend,		Xi->name() << "C" << bin_to_str(i) << "end");
		NEW_VAR_FROM_BUFF(XiCYYd0end,	Xi->name() << "C" << bin_to_str(i) << "d0end");
		NEW_VAR_FROM_BUFF(XiCYYd1end,	Xi->name() << "C" << bin_to_str(i) << "d1end");
		
		// Ajout des variables aux contraintes
			// bloc
			cons_bloc->addRight(1,XiCYY);
			// edge
			cons_edge_T->addRight(1,XiCYYd1);
			cons_edge_T->addRight(1,XiCYYd1end);
			cons_edge_NT->addRight(1,XiCYYd0);
			cons_edge_NT->addRight(1,XiCYYd0end);
			// start
			cons_start->addLeft(1,XiCYYstart);
			// end
			cons_end->addLeft(1,XiCYYend);
			
			switch(i) {
			case 0: // C00
				// in
				cons_in_00->addLeft(1,XiCYY);
				cons_in_00->addRight(1,XiCYYd0);
				cons_in_01->addRight(1,XiCYYd1);
				cons_in_00->addRight(1,XiCYYstart);
				// out
				cons_out_00->addLeft(1,XiCYY);
				cons_out_00->addRight(1,XiCYYd0);
				cons_out_00->addRight(1,XiCYYd1);
				cons_out_00->addRight(1,XiCYYend);
				// end_XX
				cons_end_00->addLeft(1,XiCYYend);
				cons_end_00->addRight(1,XiCYYd0end);
				cons_end_00->addRight(1,XiCYYd1end);
				// wrongs
				cons_wrong_NT->addRight(1,XiCYYd1);
				cons_wrong_NT->addRight(1,XiCYYd1end);							
				break;
			case 1: // C01
				// in
				cons_in_01->addLeft(1,XiCYY);
				cons_in_00->addRight(1,XiCYYd0);
				cons_in_10->addRight(1,XiCYYd1);
				cons_in_01->addRight(1,XiCYYstart);
				// out
				cons_out_01->addLeft(1,XiCYY);
				cons_out_01->addRight(1,XiCYYd0);
				cons_out_01->addRight(1,XiCYYd1);
				cons_out_01->addRight(1,XiCYYend);
				// end_XX
				cons_end_01->addLeft(1,XiCYYend);
				cons_end_01->addRight(1,XiCYYd0end);
				cons_end_01->addRight(1,XiCYYd1end);
				// wrongs
				cons_wrong_NT->addRight(1,XiCYYd1);
				cons_wrong_NT->addRight(1,XiCYYd1end);							
				break;
			case 2: // C10
				// in
				cons_in_10->addLeft(1,XiCYY);
				cons_in_01->addRight(1,XiCYYd0);
				cons_in_11->addRight(1,XiCYYd1);
				cons_in_10->addRight(1,XiCYYstart);
				// out
				cons_out_10->addLeft(1,XiCYY);
				cons_out_10->addRight(1,XiCYYd0);
				cons_out_10->addRight(1,XiCYYd1);
				cons_out_10->addRight(1,XiCYYend);
				// end_XX
				cons_end_10->addLeft(1,XiCYYend);
				cons_end_10->addRight(1,XiCYYd0end);
				cons_end_10->addRight(1,XiCYYd1end);
				// wrongs
				cons_wrong_T->addRight(1,XiCYYd0);
				cons_wrong_T->addRight(1,XiCYYd0end);							
				break;
			case 3: // C11
				// in
				cons_in_11->addLeft(1,XiCYY);
				cons_in_10->addRight(1,XiCYYd0);
				cons_in_11->addRight(1,XiCYYd1);
				cons_in_11->addRight(1,XiCYYstart);
				// out
				cons_out_11->addLeft(1,XiCYY);
				cons_out_11->addRight(1,XiCYYd0);
				cons_out_11->addRight(1,XiCYYd1);
				cons_out_11->addRight(1,XiCYYend);
				// end_XX
				cons_end_11->addLeft(1,XiCYYend);
				cons_end_11->addRight(1,XiCYYd0end);
				cons_end_11->addRight(1,XiCYYd1end);
				// wrongs
				cons_wrong_T->addRight(1,XiCYYd0);
				cons_wrong_T->addRight(1,XiCYYd0end);							
				break;
			}
	}

}


/**
 * Gets all the CFG branches and calls CS__NoConflict_2bCounter() for for each of them.
 * 
 * @param fw	Current Workspace.
 * @param cfg	CFG to extract the branches from.
 */
void BPredProcessor::processCFG__NoConflict_2bCounter(WorkSpace *fw, CFG *cfg) {
	for(CFG::BBIterator bb(cfg); bb; bb++) {

		unsigned int nb_OE = 0;
		// Parcours des OutEdges
		for(BasicBlock::OutIterator edge(bb); edge ; edge++ ) {
			// on incremente que s'il s'agit d'un edge TAKEN ou NOT_TAKEN
			if(edge->kind() == Edge::TAKEN) nb_OE++;
			else if(edge->kind() == Edge::NOT_TAKEN) nb_OE++;
		}

		// Si un branchement a ete trouve ...
		if(nb_OE == 2 ) {
			//cerr << " - Branchement trouve : x" << bb->number() << "@" << cfg->label() << io::endl; 

			CS__NoConflict_2bCounter(fw,bb);

		}

	}

}

} // ::ipet
} // ::otawa

