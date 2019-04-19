/*
 * 	InstructionCollector class implemetation
 *
 *     This file is part of OTAWA
 *     Copyright (c) 2015, IRIT UPS.
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


bool interestedInstruction(otawa::Inst* inst) { ASSERT(0); return true; }

/*
 *
 *	The FPTR_FOR_COLLECTING can be used as the following:
 *
 *	bool f(Inst* inst) { if(inst->isLoad()) return true; else return false; }
 *	otawa::oslice::FPTR_FOR_COLLECTING(workspace()) = f;
 *	workspace()->require(otawa::oslice::INSTRUCTION_COLLECTOR_FEATURE, props);
 *
 *	The f is a function which takes a pointer of an instruction, and return bool, assigned to the FPTR_FOR_COLLECTING
 *	This will be used by the default instruction collector.
 *	Once the collector is used, please remember to remove this Identifier, so the behaviors won't be over-ridden.
 */
Identifier<bool (*)(otawa::Inst*)> FPTR_FOR_COLLECTING("otawa::oslice::FPTR_FOR_COLLECTING", &interestedInstruction);


p::feature INSTRUCTION_COLLECTOR_FEATURE("otawa::oslice::INSTRUCTION_COLLECTOR_FEATURE", new Maker<InstCollector>());


p::declare InstCollector::reg = p::init("otawa::oslice::InstCollector", Version(1, 0, 0))
	.make<InstCollector>()
	.require(COLLECTED_CFG_FEATURE)
	.provide(INSTRUCTION_COLLECTOR_FEATURE);

/**
 */
InstCollector::InstCollector(AbstractRegistration& _reg)
: 	otawa::Processor(_reg) { }

/**
 */
void InstCollector::configure(const PropList &props) {
	Processor::configure(props);
}

bool InstCollector::interested(Inst* i) { // default behavior for overriden
	return true;
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

	IN_ASSERT(interested_instructions_t* p = INTERESTED_INSTRUCTIONS(fw));
	ASSERT(p);

} // end of function InstCollector::work

void InstCollector::collectInterestedInstructions(const CFGCollection& coll, interested_instructions_t* interestedInstructions) {
	// for each CFG
	for (int i = 0; i < coll.count(); i++) {
		CFG *cfg = coll[i]; // current CFG
		// for each BB in the CFG
		for (CFG::BlockIter v = cfg->blocks(); v(); v++) {
			if (!v->isBasic())
				continue;
			BasicBlock *bb = v->toBasic();
			for(BasicBlock::InstIter inst(bb); inst(); inst++) {
				bool collecting = true;
				if(FPTR_FOR_COLLECTING(workspace()) != &interestedInstruction)
					collecting = (*FPTR_FOR_COLLECTING(workspace()))(*inst);
				else
					collecting = interested(*inst);

				if(collecting) {
					DEBUGGING_MESSAGE(elm::cerr << __SOURCE_INFO__ << "adding the interested instruction " << *inst << " @ " << inst->address() << io::endl;)
					elm::cerr << "adding the interested instruction " << *inst << " @ " << inst->address() << io::endl;
					// put the interested instructions in the bucket
					InterestedInstruction* interestedInstruction = new InterestedInstruction(*inst, bb);
					interestedInstructions->add(interestedInstruction);
					interestedInstructionsLocal->add(interestedInstruction);
				} // end of the checking interested instruction
			} // end of each instruction in BB
		} // end for (CFG::BlockIter v = cfg->blocks(); v; v++) {
	} // end for (int i = 0; i < coll.count(); i++) {
}

} }
