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

namespace otawa {

Identifier<branch_category_t> BRANCH_CATEGORY("otawa::BRANCH_CATEGORY", BRANCH_NOT_CLASSIFIED);
Identifier<BasicBlock*> BRANCH_HEADER("otawa::BRANCH_HEADER", NULL);

static SilentFeature::Maker<BranchBuilder> BRANCH_CAT_MAKER;
SilentFeature BRANCH_CAT_FEATURE("otawa::BRANCH_CAT_FEATURE", BRANCH_CAT_MAKER);

BranchBuilder::BranchBuilder(void) : Processor("otawa::BranchBuilder", Version(1,0,0)) {
	require(COLLECTED_CFG_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(DOMINANCE_FEATURE);
	require(NUMBERED_CONDITIONS_FEATURE);
	provide(BRANCH_CAT_FEATURE);
}

void BranchBuilder::categorize(BasicBlock *bb, BranchProblem::Domain *dom, BasicBlock* &cat_header, branch_category_t &cat) {
	BasicBlock *current_header;
	int id = COND_NUMBER(bb);
	
	if (dom->getMust().contains(id)) {
		cat = BRANCH_ALWAYS_H;
		cout << "always history: " << cat << "\n";
	} else if (!dom->getMay().contains(id)) {
		cat = BRANCH_ALWAYS_D; 
		cout << "always default: " << cat << "\n";		
	} else {
		cat = BRANCH_NOT_CLASSIFIED;
		if (Dominance::isLoopHeader(bb))
			current_header = bb;
		else current_header = ENCLOSING_LOOP_HEADER(bb);
		  	
		int bound = 0;
		cat_header = NULL;
				
		  for (int k = dom->getPers().length() - 1 ; (k >= bound) && (current_header != NULL); k--) {
				if (dom->getPers().isPersistent(id, k)) {
					cat = BRANCH_FIRST_UNKNOWN;
					cout << "first unknown\n";
					cat_header = current_header;
				}
				current_header = ENCLOSING_LOOP_HEADER(current_header);
		  }
	}
													
				
	cout << "cat result: " << cat << "\n";			
	
}

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
		    	cout << "Categorize jump on bb " << bb->number() << " on row " << row << "\n";
		      BasicBlock *header = NULL;
		      branch_category_t cat = BRANCH_NOT_CLASSIFIED;
		      BranchProblem::Domain *dom = list.results[cfg->number()][bb->number()];
		      
		      categorize(bb, dom, header, cat);
		      
		      BRANCH_CATEGORY(bb) = cat;
		      BRANCH_HEADER(bb) = header; 
		      
		    }
		  }
        }
     }
}

void BranchBuilder::configure(const PropList &props) {
	
}
                        
}
