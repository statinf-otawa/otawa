/*
 *	$Id$
 *	Copyright (c) 2006, IRIT-UPS <rochange@irit.fr>.
 *
 *	ARMSimulator.cpp -- ARMSimulator class implementation.
 */

#include <ARMProcessor.h>
#include <ARMSimulator.h>
#include <otawa/program.h>
#include <otawa/otawa.h>

namespace otawa { namespace sim {




/**
 * Instruction execution time. Default to 5.
 */
GenericIdentifier<int> INSTRUCTION_TIME("sim.instruction_time");


/**
 * @class ARMSimulator
 * The ARM simulator simulates an ARM 7 processor. 
 */


/**
 * Build an ARM7 simulator.
 */
ARMSimulator::ARMSimulator(void)
: Simulator("ARM_simulator", Version(1, 0, 0),
	"An ARM7 simulator.") {
}


/**
 */	
ARMState *ARMSimulator::instantiate(FrameWork *fw, const PropList& props) {
	return new ARMState(fw, 5);
}

void ARMState::init() {
	processor = new ARMProcessor("ARM Processor");
	processor->build(this);
}

Driver::~Driver() {
}

Inst* DriverUntilAddr::nextInstruction() {
	if (next->address() == end_addr)
		return NULL;
	next = next->next();
	return next;
}

Inst* DriverUntilAddr::nextInstruction(bool branch_is_taken) {
	if (next->address() == end_addr)
		return NULL;
	assert(next->isConditional());
	if (branch_is_taken)
		next = next->target();
	else
		next = next->next();
	return next;
}

void DriverUntilAddr::terminateInstruction(Inst *inst) {
	assert(inst);
	if (inst->address() == end_addr)
		sim_mode = sim::HALTED;
}

sim::mode_t DriverUntilAddr::simMode() {
	return sim_mode;
}

Inst* DriverUntilEnd::nextInstruction() {
	next = next->next();
	return next;
}

Inst* DriverUntilEnd::nextInstruction(bool branch_is_taken) {
	assert(next->isConditional());
	if (branch_is_taken)
		next = next->target();
	else
		next = next->next();
	return next;
}

void DriverUntilEnd::terminateInstruction(Inst *inst) {
	assert(inst);
	if (inst->next() == NULL)
		sim_mode = sim::HALTED;
}

sim::mode_t DriverUntilEnd::simMode() {
	return sim_mode;
}


sim::mode_t ARMState::step(void) {
	elm::cout << "Cycle " << _cycle << "\n";
	processor->step();
	_cycle ++;
	return sim_mode;
}

void ARMState::setPC(address_t pc) {
	processor->setPC(pc);
}



} } // otawa::sim
