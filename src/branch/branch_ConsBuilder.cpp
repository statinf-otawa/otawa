/*
 *	ConsBuilder processor interface
 *	Copyright (c) 2011, IRIT UPS.
 *
 *	This file is part of OTAWA
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
#include <otawa/util/Dominance.h>
#include <otawa/util/PostDominance.h>
#include <otawa/util/HalfAbsInt.h>
#include <otawa/util/UnrollingListener.h>
#include <otawa/util/FirstUnrollingFixPoint.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/features.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>

#include <otawa/branch/ConsBuilder.h>
#include <otawa/branch/BranchBuilder.h>
#include <otawa/hard/BHT.h>
#include <otawa/branch/CondNumber.h>

namespace otawa { namespace branch {

using namespace ilp;
using namespace ipet;

/**
 * This feature adds to the objective function of the ILP system the raw cost of the BHT behaviour
 * (that is the branch misprediction penalties multiplied by the number of occurences).
 *
 * @ingroup branch
 */
p::feature SUPPORT_FEATURE("otawa::branch::SUPPORT_FEATURE", new Maker<ConsBuilder>());


/**
 * This feature adds to the ILP system the constraints modelling the number of misspredictions
 * and a variable representing this number.
 *
 * @par Properties
 * @li @ref MISSPRED_VAR
 *
 * @ingroup branch
 */
p::feature CONSTRAINTS_FEATURE("otawa::branch::CONSTRAINTS_FEATURE", new Maker<OnlyConsBuilder>);


/**
 * This properties gives the variables counting the number of miss-prediction
 * for a basic block ending with a control instruction.
 *
 * @par Feature
 * @li @ref SUPPORT_FEATURE
 *
 * @par Hook
 * @li @ref BasicBlock
 *
 * @ingroup branch
 */
Identifier<ilp::Var*> MISSPRED_VAR("otawa::branch::MISSPRED_VAR", NULL);


/**
 * @class OnlyConsBuilder
 * This processor add to the current ILP the constraint requires
 * to model the behavior of the BHT.
 *
 * @par Configuration
 *
 * @par Provided Features
 * @li @ref CONSTRAINTS_FEATURE
 *
 * @par Required Features
 * @li @ref ipet::ASSIGNED_VARS_FEATURE,
 * @li @ref LOOP_INFO_FEATURE,
 * @li @ref CATEGORY_FEATURE,
 *
 * @ingroup branch
 */

p::declare OnlyConsBuilder::reg =
		p::init("otawa::ConsBuilder", Version(1,0,0), BBProcessor::reg)
		.require(ipet::ASSIGNED_VARS_FEATURE)
		.require(LOOP_INFO_FEATURE)
		.require(CATEGORY_FEATURE)
		.provide(CONSTRAINTS_FEATURE)
		.maker<ConsBuilder>();


OnlyConsBuilder::OnlyConsBuilder(p::declare& r) : BBProcessor(r) {
}

void OnlyConsBuilder::processBB(WorkSpace* ws, CFG *cfg, BasicBlock *bb) {
	static string 	ad_msg = "always-D branch prediction constraint",
					ah_msg = "always-H branch prediction constraint",
					fu_msg = "first-unknown branch prediction constraint",
					nc_msg = "not-classified branch prediction constraint";

	if (branch::COND_NUMBER(bb) != -1) {
		int row = hard::BHT_CONFIG(ws)->line(bb->lastInst()->address());
		if(logFor(LOG_BLOCK))
			log << "\t\t\tprocess jump on bb " << bb->number() << " on row " << row << "\n";
		branch::category_t cat = branch::CATEGORY(bb);
		BasicBlock *cat_header = branch::HEADER(bb);
		ilp::System *sys = ipet::SYSTEM(ws);
		ilp::Constraint *cons; 
		ilp::Var *misspred;
		ilp::Var *bbvar = ipet::VAR(bb);
		ilp::Var *NTvar, *Tvar;
		
		// build the miss-prediction variable
        if(!_explicit)
                misspred = sys->newVar();
        else {
                StringBuffer buf1;
                buf1 << "x_mpred_" << bb->number() << "\n";
                String name1 = buf1.toString();
                misspred = sys->newVar(name1);
        }
        MISSPRED_VAR(bb) = misspred;
			
        // add constraint according to the category
		switch(cat) {

			// always default prediction (not-taken)
			case branch::ALWAYS_D:
			
				// get the not-taken edge
				Tvar = NULL;
            	for (BasicBlock::OutIterator outedge(bb); outedge; outedge++)
            		if (outedge->kind() == Edge::NOT_TAKEN) {
						ASSERT(Tvar == NULL);
						Tvar = VAR(outedge);                			
            		}

            	// x_misspred = e_not-taken
            	cons = sys->newConstraint(ad_msg, Constraint::EQ, 0);
            	cons->addLeft(1, misspred);
                cons->addRight(1, Tvar); 
				break;

			// always in the BHT
			case branch::ALWAYS_H:
				
				// find taken and not-taken edges
				NTvar = NULL;
				Tvar = NULL;
            	for (BasicBlock::OutIterator outedge(bb); outedge; outedge++) {
            		if (outedge->kind() == Edge::NOT_TAKEN) {
						ASSERT(NTvar == NULL);
						NTvar = VAR(outedge);                			
            		}
            		if (outedge->kind() == Edge::TAKEN) {
						ASSERT(Tvar == NULL);
						Tvar = VAR(outedge);                			
            		}            		
            	}
            	
				// x_misspred <= 2 * e_taken + 2
				cons = sys->newConstraint(ah_msg, Constraint::LE, 2);
				cons->addLeft(1, misspred);	
				cons->addRight(2, Tvar);
				
				// misspred <= 2 * e_not-taken + 2
				cons = sys->newConstraint(ah_msg, Constraint::LE, 2);
				cons->addLeft(1, misspred);	
				cons->addRight(2, NTvar);

				// x_misspred <= x_bb
				cons = sys->newConstraint(ah_msg, Constraint::LE, 0);
                cons->addLeft(1, misspred);
                cons->addRight(1, bbvar); 		
				break;

			case branch::FIRST_UNKNOWN:
				
				NTvar = NULL;
				Tvar = NULL;
				
            	for (BasicBlock::OutIterator outedge(bb); outedge; outedge++) {
            		if (outedge->kind() == Edge::NOT_TAKEN) {
						ASSERT(NTvar == NULL);
						NTvar = VAR(outedge);                			
            		}
            		if (outedge->kind() == Edge::TAKEN) {
						ASSERT(Tvar == NULL);
						Tvar = VAR(outedge);                			
            		}            		
            	}
            	/* misspred <= 2*taken + 2*(sum of entry-edges) */ 				
				cons = sys->newConstraint(fu_msg, Constraint::LE, 0);
				cons->addLeft(1, misspred);	
				cons->addRight(2, Tvar);
				for (BasicBlock::InIterator inedge(cat_header); inedge; inedge++)
					if (!Dominance::dominates(cat_header, inedge->source()))
						cons->addRight(2, VAR(inedge));
				
            	/* misspred <= 2*not-taken + 2*(sum of entry-edges) */ 								
				cons = sys->newConstraint(fu_msg, Constraint::LE, 0);
				cons->addLeft(1, misspred);	
				cons->addRight(2, NTvar);
				for (BasicBlock::InIterator inedge(cat_header); inedge; inedge++)
					if (!Dominance::dominates(cat_header, inedge->source()))
						cons->addRight(2, VAR(inedge));
				
				/* misspred <= bbvar */			
				cons = sys->newConstraint(fu_msg, Constraint::LE, 0);
                cons->addLeft(1, misspred);
                cons->addRight(1, bbvar);	
                			
				break;		
								
			case branch::NOT_CLASSIFIED:
			
				/* misspred <= bbvar */
				cons = sys->newConstraint(nc_msg, Constraint::LE, 0);
                cons->addLeft(1, misspred);
                cons->addRight(1, bbvar);
                				
				break;			
			default:
				cout << "unknown cat: " << cat << "\n";
				ASSERT(false);
				break;
		} 
	}
	
}

void OnlyConsBuilder::configure(const PropList &props) {
	BBProcessor::configure(props);
	_explicit = ipet::EXPLICIT(props);
}



/**
 * @class ConsBuilder
 * This processor add to the current ILP the constraint requires
 * to model the behaviour of the BHT.
 *
 * @par Configuration
 *
 * @par Provided Features
 * @li @ref SUPPORT_FEATURE
 *
 * @par Required Features
 * @li @ref branch::CONSTRAINTS_FEATURE,
 *
 * @ingroup branch
 */

p::declare ConsBuilder::reg =
		p::init("otawa::ConsBuilder", Version(1,0,0), BBProcessor::reg)
		.require(CONSTRAINTS_FEATURE)
		.provide(SUPPORT_FEATURE)
		.maker<ConsBuilder>();


ConsBuilder::ConsBuilder(p::declare& r) : BBProcessor(r) {
}

void ConsBuilder::processBB(WorkSpace* ws, CFG *cfg, BasicBlock *bb) {
	if(branch::COND_NUMBER(bb) != -1) {
		ilp::System *sys = ipet::SYSTEM(ws);
	    int penalty;
	    if (bb->lastInst()->isIndirect()) {
	    	if (bb->lastInst()->isConditional()) {
	    		penalty = hard::BHT_CONFIG(ws)->getCondIndirectPenalty();
	    	} else {
	    		penalty = hard::BHT_CONFIG(ws)->getIndirectPenalty();
	    	}
	    } else {
	    	penalty = hard::BHT_CONFIG(ws)->getCondPenalty();
	    }
	    ilp::Var *misspred = MISSPRED_VAR(bb);
		sys->addObjectFunction(penalty, misspred);
	}
}
                        
} }		// otawa::branch
