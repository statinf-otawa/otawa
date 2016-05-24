/*
 *     $Id$
 *     TO BE DETERMINED
 *
 *
 *     This file is part of OTAWA
 *     Copyright (c) 2007, IRIT UPS.
 *
 *     OTAWA is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     OTAWA is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with OTAWA; if not, write to the Free Software
 *     Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *     02110-1301  USA
 */

#include <otawa/oslice/InstCollector.h>
namespace otawa { namespace oslice {
#define DEBUGGING_MESSAGE(x) // x
/**
 */
InstCollector::InstCollector(AbstractRegistration& _reg)
: 	otawa::Processor(_reg) { }

/**
 */
void InstCollector::configure(const PropList &props) {
	Processor::configure(props);
}

void InstCollector::processWorkSpace(WorkSpace *fw) {
	// obtain the collected CFG from the program
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(fw);
	// first lets make the control instructions the interested instructions that we want to slice for
	interested_instructions_t *interestedInstructions = INTERESTED_INSTRUCTIONS(fw);
	interestedInstructionsLocal = new interested_instructions_t();
	if(!interestedInstructions) {
		interestedInstructions = new interested_instructions_t();
		INTERESTED_INSTRUCTIONS(fw) = interestedInstructions;
	}
	collectInterestedInstructions(coll, interestedInstructions);

	interested_instructions_t* p = INTERESTED_INSTRUCTIONS(fw);
	assert(p);

} // end of function InstCollector::work

void InstCollector::collectInterestedInstructions(const CFGCollection& coll, interested_instructions_t* interestedInstructions) {
	// for each CFG
	for (int i = 0; i < coll.count(); i++) {
		CFG *cfg = coll[i]; // current CFG
		// for each BB in the CFG
		for (CFG::BlockIter v = cfg->blocks(); v; v++) {
			if (!v->isBasic())
				continue;
			BasicBlock *bb = v->toBasic();
			for(BasicBlock::InstIter inst(bb); inst; inst++) {
				if(interested(inst)) {
					DEBUGGING_MESSAGE(elm::cerr << __SOURCE_INFO__ << "adding the interested instruction " << *inst << " @ " << inst->address() << io::endl;)
					elm::cerr << "adding the interested instruction " << *inst << " @ " << inst->address() << io::endl;
					// put the interested instructions in the bucket
					InterestedInstruction* interestedInstruction = new InterestedInstruction(inst, bb);
					interestedInstructions->add(interestedInstruction);
					interestedInstructionsLocal->add(interestedInstruction);
				} // end of the checking interested instruction
			} // end of each instruction in BB
		} // end for (CFG::BlockIter v = cfg->blocks(); v; v++) {
	} // end for (int i = 0; i < coll.count(); i++) {
}

} }
