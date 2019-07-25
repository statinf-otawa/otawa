/*
 *	InstCollector class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef __OTAWA_OSLICE_INST_COLLECTOR_H__
#define __OTAWA_OSLICE_INST_COLLECTOR_H__

#include <otawa/cfg/features.h>
#include "oslice.h"
#include <otawa/prog/WorkSpace.h> // necessary to have Identifier working on fw

namespace otawa { namespace oslice {

class InstCollector: public otawa::Processor {
public:
	static p::declare reg;
	InstCollector(AbstractRegistration& _reg = reg);

protected:
	// called by the children
	virtual void configure(const PropList &props);
	// implemented by each children
	void collectInterestedInstructions(const CFGCollection& coll, interested_instructions_t* interestedInstructions);
	virtual bool interested(Inst* i);
	interested_instructions_t *interestedInstructionsLocal;


private:
	void processWorkSpace(WorkSpace *fw);
	int showDebugingMessage;
};

/*
 * The Cleaner class for InstCollector
 */
class InstCollectorCleaner: public Cleaner {
public:
	InstCollectorCleaner(WorkSpace *_ws, interested_instructions_t* _iiti): ws(_ws), interestedInstructionsLocal(_iiti) { }

protected:
	virtual void clean(void) {
//		interested_instructions_t* targetToRemove = INTERESTED_INSTRUCTIONS(ws);
//		for(interested_instructions_t::Iterator iiti(*interestedInstructionsLocal); iiti; iiti++) {
//			targetToRemove->remove(iiti);
//		}
		INTERESTED_INSTRUCTIONS(ws) = 0;
	}
private:
	WorkSpace* ws;
	interested_instructions_t* interestedInstructionsLocal;
};

} } // otawa::oslice

#endif
