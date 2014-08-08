/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
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
 
#include <otawa/branch/BranchBuilder.h>
#include <otawa/branch/BranchProblem.h>
#include <otawa/hard/BHT.h>
#include <otawa/branch/CondNumber.h>

namespace otawa { namespace branch {

/**
 * Branch prediction category: it is put on the BB containing
 * the branch instruction.
 *
 * @par Feature
 * @li @ref CATEGORY_FEATURE
 * @par Hooks
 * @li @ref BasicBlock
 *
 * @ingroup branch
 */
Identifier<category_t> CATEGORY("otawa::branch::CATEGORY", branch::NOT_CLASSIFIED);


/**
 * Loop header of FIRST_UNKNOWN branch prediction category.
 * If the loop iterates at most N times, the first time has an unknown behaviour
 * and, in the N-1 next iterations, the predicted target is in the BHT.
 *
 * @par Feature
 * @li @ref CATEGORY_FEATURE
 * @par Hooks
 * @li @ref BasicBlock
 *
 * @ingroup branch
 */
Identifier<BasicBlock*> HEADER("otawa::branch::HEADER", NULL);


/**
 * This category assigns to each branch a category according to its branch prediction  behavior.
 * The following branch prediction categories are supported:
 * @li ALWAYS_D -- always predicted with default branch prediction
 * @li ALWAYS_H -- always prediction is in the BHT
 * @li FIRST_UNKNOWN -- first prediction unknown, other prediction in the BHT.
 * @li NOT_CLASSIFIED -- the behavior cannot precisely be analyzed.
 *
 * @par Properties
 * @li @ref CATEGORY
 * @li @ref HEADER
 *
 *
 * @ingroup branch
 */
p::feature CATEGORY_FEATURE("otawa::branch::CATEGORY_FEATURE", new Maker<BranchBuilder>());


/**
 */
Output& operator<<(Output& out, category_t cat) {
	static cstring labels[] = {
		"undef",			// UNDEF = 0
		"always-D",			// ALWAYS_D = 1,
		"always-H",			// ALWAYS_H = 2,
		"first-unknown",	// FIRST_UNKNOWN = 3,
		"not-classified"	// NOT_CLASSIFIED = 4
	};
	ASSERTP(cat < MAX, "bad branch category");
	out << labels[cat];
	return out;
}

/**
 * @class BranchBuilder
 * Compute the categories for branch prediction.
 *
 * @par Configuration
 *
 * @par Provided Features
 * @li  @ref CATEGORY_FEATURE
 *
 * @par Required Features
 * @li @ref COLLECTED_CFG_FEATURE,
 * @li @ref LOOP_INFO_FEATURE,
 * @li @ref DOMINANCE_FEATURE,
 * @li @ref NUMBERED_CONDITIONS_FEATURE,
 *
 * @ingroup branch
 */
p::declare BranchBuilder::reg =
	p::init("otawa::branch::BranchBuilder", Version(1,0,0))
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(DOMINANCE_FEATURE)
	.require(NUMBERED_CONDITIONS_FEATURE)
	.provide(CATEGORY_FEATURE)
	.maker<BranchBuilder>();


/**
 */
BranchBuilder::BranchBuilder(void) : Processor(reg) {
}

/**
 * Assign categories to the basic blocks.
 */
void BranchBuilder::categorize(BasicBlock *bb, BranchProblem::Domain *dom, BasicBlock* &cat_header, category_t &cat) {
	BasicBlock *current_header;
	int id = COND_NUMBER(bb);
	
	if (dom->getMust().contains(id)) {
		cat = ALWAYS_H;
		//cout << "always history: " << cat << "\n";
	} else if (!dom->getMay().contains(id)) {
		cat = ALWAYS_D;
		//cout << "always default: " << cat << "\n";
	} else {
		cat = NOT_CLASSIFIED;
		if (Dominance::isLoopHeader(bb))
			current_header = bb;
		else current_header = ENCLOSING_LOOP_HEADER(bb);
		  	
		int bound = 0;
		cat_header = NULL;
				
		  for (int k = dom->getPers().length() - 1 ; (k >= bound) && (current_header != NULL); k--) {
				if (dom->getPers().isPersistent(id, k)) {
					cat = FIRST_UNKNOWN;
					//cout << "first unknown\n";
					cat_header = current_header;
				}
				current_header = ENCLOSING_LOOP_HEADER(current_header);
		  }
	}
													
	if(logFor(LOG_BB))
		log << "\t\tcat result: " << cat << "\n";
	
}

/**
 */
void BranchBuilder::processWorkSpace(WorkSpace* ws) {
	int size;
	int row;

        
	for (row = 0; row < hard::BHT_CONFIG(ws)->rowCount(); row++) {
              size = COND_MAX(ws)[row];

		BranchProblem prob(size, ws, hard::BHT_CONFIG(ws)->wayCount(), row);
		UnrollingListener<BranchProblem> list( ws, prob);
		FirstUnrollingFixPoint<UnrollingListener<BranchProblem> > fixp(list);
		util::HalfAbsInt<FirstUnrollingFixPoint<UnrollingListener<BranchProblem> > > hai(fixp, *ws);
		hai.solve();
	
		for (CFGCollection::Iterator cfg(*INVOLVED_CFGS(ws)); cfg; cfg++) {
			for (CFG::BBIterator bb(*cfg); bb; bb++) {
				if ((COND_NUMBER(bb) != -1) && (hard::BHT_CONFIG(ws)->line(bb->lastInst()->address()) == row)) {

					if(logFor(LOG_BB))
						log << "\tcategorize jump on bb " << bb->number() << " on row " << row << "\n";
					BasicBlock *header = NULL;
					category_t cat = NOT_CLASSIFIED;
					BranchProblem::Domain *dom = list.results[cfg->number()][bb->number()];

					categorize(bb, dom, header, cat);

					CATEGORY(bb) = cat;
					HEADER(bb) = header;

				}
			}
		}
	}
}

} }		// otawa::branch

