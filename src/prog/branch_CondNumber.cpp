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

namespace otawa {

static SilentFeature::Maker<CondNumber> NUMBERED_CONDITIONS_MAKER;
SilentFeature NUMBERED_CONDITIONS_FEATURE("otawa::NUMBERED_CONDITIONS_FEATURE", NUMBERED_CONDITIONS_MAKER);

Identifier<int> COND_NUMBER("otawa::COND_NUMBER", -1);
Identifier<int*> COND_MAX("otawa::COND_MAX", NULL);

CondNumber::CondNumber(void) : BBProcessor("otawa::CondNumber", Version(1,0,0)) {
  require(COLLECTED_CFG_FEATURE);
  provide(NUMBERED_CONDITIONS_FEATURE);
}

void CondNumber::processBB(WorkSpace* ws, CFG *cfg, BasicBlock *bb) {
	Inst *last = bb->lastInst();
	cleanup(ws);
	if (last != NULL) {
		if (last->isConditional() || last->isIndirect()) {

			int row = hard::BHT_CONFIG(ws)->line(last->address());
			cout << "conditionnal: " << bb->number() << " (row " << row << ")\n";			
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
	COND_MAX(ws) = current_index;	
} 


void CondNumber::configure(const PropList &props) {

}
                        
}
