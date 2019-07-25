/*
 *	CCGDFA class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
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
#include <elm/assert.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <elm/data/HashMap.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ccg/Builder.h>
#include <otawa/ccg/ConstraintBuilder.h>
#include <otawa/ccg/DFA.h>
#include "../../include/otawa/cfg/ContextTree.h"

using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa { namespace ccg {


/**
 */
int Problem::vars = 0;


/**
 */
Domain *Problem::gen(CFG *cfg, Block *bb) {
	/*int length =*/ ccggraph->count();
	Domain *bitset = empty();
	if(bb->isEntry() && (cfg == ENTRY_CFG(fw))) {
		bitset->add(0);
		return bitset;
	}
	else if(bb->isBasic()) {
		address_t adlbloc;
	    int identif = 0;
		for(BasicBlock::InstIter inst = bb->toBasic()->insts(); inst; inst++) {
			adlbloc = inst->address();
			for (LBlockSet::Iterator lbloc(*ccggraph); lbloc; lbloc++) {
				address_t address = lbloc->address();
				if(adlbloc == address && lbloc->bb()== bb)
					identif = lbloc->id();
			}
		}

		if(identif != 0)
		 	bitset->add(identif);
		return bitset;
	}
	else
		return bitset;
}

/**
 */
Domain *Problem::preserve(CFG *cfg, Block *bb) {
	if(!bb->isBasic())
		return empty();
	bool testnotconflit = false;
	bool visit = false;
	address_t adlbloc;
    int identif1 = 0;

	for(BasicBlock::InstIter inst = bb->toBasic()->insts(); inst; inst++) {
		visit = false;
		int dec = cach->blockBits();
		adlbloc = inst->address();
		if (!testnotconflit) {

			// lblocks iteration
			for (LBlockSet::Iterator lbloc(*ccggraph); lbloc; lbloc++) {
				if (adlbloc == lbloc->address() && bb == lbloc->bb()) {
					testnotconflit = true;
					identif1 = lbloc->id();
					unsigned long tag = ((unsigned long)adlbloc.offset()) >> dec;
					for(LBlockSet::Iterator lbloc1(*ccggraph); lbloc1; lbloc1++) {
						unsigned long taglblock = ((unsigned long)lbloc1->address().offset()) >> dec;
						if(adlbloc != lbloc1->address() && tag == taglblock
						&& bb != lbloc1->bb()) {
							ccggraph->lblock(identif1);
							break;
						}
					}
					break;
				}

			}
			visit = true;

			if (!visit) {
				for (LBlockSet::Iterator lbloc(*ccggraph); lbloc; lbloc++) {
					if(adlbloc == lbloc->address() && bb != lbloc->bb())
						break ;
				}
			}

		}
	}

	// the bit vector of kill
	/*int length =*/ ccggraph->count();
	Domain *kill;
	kill = empty();
	if(identif1 == 0)  {
		kill->fill();
	}
	return kill;
}



} }	// otawa::ccg

