/*
 *	BranchBuilder class implementation
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

#include <otawa/branch/BranchBuilder.h>
#include <otawa/branch/BranchProblem.h>
#include <otawa/branch/CondNumber.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/PostDominance.h>
#include <otawa/dfa/hai/FirstUnrollingFixPoint.h>
#include <otawa/dfa/hai/HalfAbsInt.h>
#include <otawa/dfa/hai/UnrollingListener.h>
#include <otawa/hard/BHT.h>

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
Identifier<Block*> HEADER("otawa::branch::HEADER", NULL);


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
		"not-classified",	// NOT_CLASSIFIED = 4
		"static taken",		// STATIC_TAKEN = 5
		"static not-taken",	// STATIC_NOT_TAKEN = 6
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
void BranchBuilder::categorize(BasicBlock *bb, BranchProblem::Domain *dom, Block* &cat_header, category_t &cat) {
	Block *current_header;
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
		dfa::hai::UnrollingListener<BranchProblem> list( ws, prob);
		dfa::hai::FirstUnrollingFixPoint<dfa::hai::UnrollingListener<BranchProblem> > fixp(list);
		dfa::hai::HalfAbsInt<dfa::hai::FirstUnrollingFixPoint<dfa::hai::UnrollingListener<BranchProblem> > > hai(fixp, *ws);
		hai.solve();

		for (CFGCollection::Iter cfg(*INVOLVED_CFGS(ws)); cfg(); cfg++) {
			for(CFG::BlockIter b = cfg->blocks(); b(); b++)
				if(b->isBasic()) {
					BasicBlock *bb = b->toBasic();
					if ((COND_NUMBER(bb) != -1) && int(hard::BHT_CONFIG(ws)->line(bb->control()->address())) == row) {

						if(logFor(LOG_BB))
							log << "\tcategorize jump on " << *bb << " on row " << row << "\n";
						Block *header = NULL;
						category_t cat = NOT_CLASSIFIED;
						BranchProblem::Domain *dom = list.results[cfg->index()][bb->index()];

						categorize(bb, dom, header, cat);

						CATEGORY(bb) = cat;
						if(header)
							HEADER(bb) = header;

					}
			}
		}
	}
}

} }		// otawa::branch

