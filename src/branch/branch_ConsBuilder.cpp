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

#include <otawa/branch/BranchBuilder.h>
#include <otawa/branch/CondNumber.h>
#include <otawa/branch/ConsBuilder.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/PostDominance.h>
#include <otawa/dfa/hai/FirstUnrollingFixPoint.h>
#include <otawa/dfa/hai/HalfAbsInt.h>
#include <otawa/dfa/hai/UnrollingListener.h>
#include <otawa/hard/BHT.h>
#include <otawa/ilp.h>
#include <otawa/ilp/expr.h>
#include <otawa/ipet.h>
#include <otawa/prog/WorkSpace.h>


namespace otawa { namespace branch {

using namespace ilp;
using namespace ipet;

/**
 * This feature adds to the objective function of the ILP system the raw cost of the BHT behaviour
 * (that is the branch misprediction penalties multiplied by the number of occurrences).
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

/**
 */
p::declare OnlyConsBuilder::reg =
		p::init("otawa::ConsBuilder", Version(1,0,0), BBProcessor::reg)
		.require(ipet::ASSIGNED_VARS_FEATURE)
		.require(LOOP_INFO_FEATURE)
		.require(CATEGORY_FEATURE)
		.require(hard::BHT_FEATURE)
		.provide(CONSTRAINTS_FEATURE)
		.maker<ConsBuilder>();

/**
 */
OnlyConsBuilder::OnlyConsBuilder(p::declare& r) : BBProcessor(r), _explicit(false), bht(0) {
}

/**
 */
void OnlyConsBuilder::setup(WorkSpace *ws) {
	bht = hard::BHT_CONFIG(ws);
}

/**
 * Build constraints for always-default predicted.
 * @param model		ILP model to use.
 * @param x_mp		Miss prediction count variable.
 * @param bb		Current basic block.
 */
void OnlyConsBuilder::genAlwaysDefault(ilp::model& model, ilp::var x_mp, BasicBlock *bb) {
	static string 	ad_msg = "always-D branch prediction constraint";

	// direct control or not direct prediction
	if(bb->control()->target() || bht->getDefaultPrediction() != hard::PREDICT_DIRECT) {
		int def = bht->actualDefaultPrediction(bb->control()->address(), bb->control()->target()->address());

		// if default = not-taken then x^mp_i = x_T,i
		if(def == hard::PREDICT_NOT_TAKEN) {
			ilp::cons c = model(ad_msg) + x_mp == 0;
			for(Block::EdgeIter e = bb->outs(); e(); e++)
				if(e->isTaken())
					c += *VAR(*e);
		}

		// else (default = taken) x^mp_i = x_NT,i
		else {
			ilp::cons c = model(ad_msg) + x_mp == 0;
			for(Block::EdgeIter e = bb->outs(); e(); e++)
				if(e->isNotTaken())
					c += *VAR(*e);
		}

	}

	// indirect control and default = direct
	else {

		// x^mp >= x_T,F
		cons c1 = model(ad_msg) + x_mp >= 0;
		for(Block::EdgeIter e = bb->outs(); e(); e++)
			if(e->isTaken() && e->isForward())
				c1 += *VAR(*e);

		// x^mp <= x_NT + x_T,F
		cons c2 = model(ad_msg) + x_mp <= 0;
		for(Block::EdgeIter e = bb->outs(); e(); e++)
			if(e->isNotTaken() || e->isForward())
				c2 += *VAR(*e);
	}

}

/**
 * Generate constraint for Always-Hit category.
 * @param model		ILP model to complete.
 * @param x_mp		Miss-prediction counter variable.
 * @param bb		Current basic block.
 */
void OnlyConsBuilder::genAlwaysHit(ilp::model& model, ilp::var x_mp, BasicBlock *bb) {
	static string ah_msg = "always-H branch prediction constraint";

	// x_mp <= 2 x_T + 2
	cons c1 = model(ah_msg) + x_mp <= 2;
	for(Block::EdgeIter e = bb->outs(); e(); e++)
		if(e->isTaken())
			c1 += 2 * var(VAR(*e));

	// x_mp <= 2 x_NT + 2
	cons c2 = model(ah_msg) + x_mp <= 2;
	for(Block::EdgeIter e = bb->outs(); e(); e++)
		if(e->isNotTaken())
			c2 += 2 * var(VAR(*e));

	// x_mp <= x_i
	model(ah_msg) + x_mp <= *VAR(bb);
}

/**
 * Generate constraints for category First-Unknown.
 * @param model		ILP model to complete.
 * @param x_mp		Miss-prediction counter variable.
 * @param bb		Current basic block.
 */
void OnlyConsBuilder::genFirstUnknown(ilp::model& model, ilp::var x_mp, BasicBlock *bb) {
	static string fu_msg = "first-unknown branch prediction constraint";
	Block *h = HEADER(bb);
	ASSERT(h);

	// x_mp <= 2 x_T + 2 sum{(i,h) /\ h not-dom i} x_i,h
	cons c1 = model(fu_msg) + x_mp <= 0;
	for(Block::EdgeIter e = bb->outs(); e(); e++)
		if(e->isTaken())
			c1 += 2 * var(VAR(*e));
	for(Block::EdgeIter e = h->ins(); e(); e++)
		if(!Dominance::dominates(h, e->sink()))
			c1 += 2 * var(VAR(*e));

	// x_mp <= 2 x_NT + + 2 sum{(i,h) /\ h not-dom i} x_i,h
	cons c2 = model(fu_msg) + x_mp <= 0;
	for(Block::EdgeIter e = bb->outs(); e(); e++)
		if(e->isNotTaken())
			c2 += 2 * var(VAR(*e));
	for(Block::EdgeIter e = h->ins(); e(); e++)
		if(!Dominance::dominates(h, e->sink()))
			c2 += 2 * var(VAR(*e));

	// x_mp <= x_i
	model(fu_msg) + x_mp <= *VAR(bb);
}

/**
 * Generate constraints for unknown category.
 * @param model		ILP model to complete.
 * @param x_mp		Miss-prediction counter variable.
 * @param bb		Current basic block.
 */
void OnlyConsBuilder::genNotClassified(ilp::model& model, ilp::var x_mp, BasicBlock *bb) {
	static string nc_msg = "not-classified branch prediction constraint";
	// x_mp <= x_i
	model(nc_msg) + x_mp <= *VAR(bb);
}

/**
 */
void OnlyConsBuilder::processBB(WorkSpace* ws, CFG *cfg, Block *b) {

	// only for basic block with a conditional branch
	if(!b->isBasic() || branch::COND_NUMBER(b) == -1)
		return;
	BasicBlock *bb = b->toBasic();

	// display log
	if(logFor(LOG_BLOCK))
		log << "\t\t\tprocess jump on " << *bb << " on row " << bht->line(bb->control()->address())
			<< " (" << *CATEGORY(bb) << ", " << *HEADER(bb) << ")\n";

	// build x_mp
	ilp::model model(ipet::SYSTEM(ws));
	string name;
	if(_explicit)
		name << "x_mp_" << bb->index() << "_" << bb->cfg()->label();
	var x_mp = model.var(name);
	MISSPRED_VAR(bb) = x_mp;

	// add constraint according to the category
	switch(CATEGORY(bb)) {
	case branch::ALWAYS_D:
		genAlwaysDefault(model, x_mp, bb);
		break;
	case branch::ALWAYS_H:
		genAlwaysHit(model, x_mp, bb);
		break;
	case branch::FIRST_UNKNOWN:
		genFirstUnknown(model, x_mp, bb);
		break;
	case branch::NOT_CLASSIFIED:
		genNotClassified(model, x_mp, bb);
		break;
	default:
		cout << "unknown cat: " << *CATEGORY(bb) << "\n";
		ASSERT(false);
		break;
	}
}

/**
 */
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

p::declare ConsBuilder::reg = p::init("otawa::ConsBuilder", Version(1,0,0), BBProcessor::reg)
	.require(CONSTRAINTS_FEATURE)
	.require(hard::BHT_FEATURE)
	.provide(SUPPORT_FEATURE)
	.maker<ConsBuilder>();


/**
 */
ConsBuilder::ConsBuilder(p::declare& r) : BBProcessor(r) {
}


/**
 */
void ConsBuilder::processBB(WorkSpace* ws, CFG *cfg, Block *b) {
	if(!b->isBasic())
		return;
	BasicBlock *bb = b->toBasic();
	if(branch::COND_NUMBER(bb) != -1) {
		ilp::System *sys = ipet::SYSTEM(ws);
	    int penalty;
	    if(bb->control()->isUnknown()) {
	    	if(bb->control()->isConditional())
	    		penalty = hard::BHT_CONFIG(ws)->getCondIndirectPenalty();
	    	else
	    		penalty = hard::BHT_CONFIG(ws)->getIndirectPenalty();
	    }
	    else
	    	penalty = hard::BHT_CONFIG(ws)->getCondPenalty();
	    ilp::Var *misspred = MISSPRED_VAR(bb);
		sys->addObjectFunction(penalty, misspred);
	}
}
                        
} }		// otawa::branch
