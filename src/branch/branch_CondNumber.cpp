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
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/features.h>
 
#include <otawa/branch/CondNumber.h>
#include <otawa/hard/BHT.h>

namespace otawa { namespace branch {

/**
 * This feature assign numbers to branch present in a workspace for next processing.
 *
 * @par Properties
 * @li @ref COND_NUMBER
 * @li @ref COND_MAX
 *
 * @ingroup branch
 */
p::feature NUMBERED_CONDITIONS_FEATURE("otawa::branch::NUMBERED_CONDITIONS_FEATURE", new Maker<CondNumber>());

/**
 * Property giving the number of the control in its BHT set.
 *
 * @par Feature
 * @li @ref NUMBERED_CONDITIONS_MAKER
 *
 * @par Hook
 * @li @ref BasicBlock
 *
 * @ingroup branch
 */
Identifier<int> COND_NUMBER("otawa::branch::COND_NUMBER", -1);

/**
 * This features returns an array integer giving, for each BHT set,
 * the maximum number of branches in the workspace.
 *
 * @par Feature
 * @li @ref NUMBERED_CONDITIONS_MAKER
 *
 * @par Hook
 * @li @ref WorkSpace
 *
 * @ingroup branch
 */
Identifier<int *> COND_MAX("otawa::branch::COND_MAX", NULL);


/**
 * @class CondNumber
 * Associate numbers to each branch relatively to their BHT set.
 *
 * @par Configuration
 *
 * @par Provided Features
 * @li @ref NUMBERED_CONDITIONS_FEATURE
 *
 * @par Required Features
 * @li @ref COLLECTED_CFG_FEATUR
 *
 *
 * @ingroup branch
 */
p::declare CondNumber::reg =
		p::init("otawa::branch::CondNumber", Version(1,0,0), BBProcessor::reg)
		.require(COLLECTED_CFG_FEATURE)
		.require(hard::BHT_FEATURE)
		.provide(NUMBERED_CONDITIONS_FEATURE)
		.maker<CondNumber>();


CondNumber::CondNumber(void): BBProcessor(reg) {
}

void CondNumber::processBB(WorkSpace* ws, CFG *cfg, BasicBlock *bb) {
	Inst *last = bb->lastInst();
	cleanup(ws);
	if (last != NULL) {
		if (last->isConditional() || last->isIndirect()) {

			int row = hard::BHT_CONFIG(ws)->line(last->address());
			if(logFor(Processor::LOG_BB))
				log << "\t\t\tconditional: " << bb->number() << " (row " << row << ")\n";
			COND_NUMBER(bb) = current_index[row];
			current_index[row]++;
		}
	}
  
}

void CondNumber::setup(WorkSpace *ws) {
	int num_rows = hard::BHT_CONFIG(ws)->rowCount();
	current_index = new int[num_rows];
	for (int i = 0; i < num_rows; i++) {
		current_index[i] = 0;
	}
	COND_MAX(ws) = current_index;
} 

void CondNumber::cleanup(WorkSpace *ws) {
	// not cleaned
	COND_MAX(ws) = current_index;	
} 

} }		// otawa::branch

