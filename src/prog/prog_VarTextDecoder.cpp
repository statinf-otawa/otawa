/*
 *	$Id$
 *	Copyright (c) 2007, IRIT UPS.
 *
 *	VarTextDecoder class implementation
 */

#include <elm/assert.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/VarTextDecoder.h>
#include <otawa/proc/Registry.h>
#include <elm/genstruct/VectorQueue.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/util/FlowFactLoader.h>

using namespace elm;

#define QUEUE_SIZE	512

#define TRACE(m)	//cerr << m << io::endl;

namespace otawa {

// Simple marker
static Identifier<bool> MARKER("", false);

/**
 * @class VarTextDecoder
 * This text decoder may be used for most processor with fixed or variable
 * length instructions. It proceeds by following all the paths of the
 * programs starting from entry points like declared program start,
 * function labels and so on.
 */


/**
 * Static processors.
 */
VarTextDecoder VarTextDecoder::_;


/**
 * Constructor.
 */
VarTextDecoder::VarTextDecoder(void)
: Processor("otawa::VarTextDecoder", Version(1, 0, 0)) {
	provide(DECODED_TEXT);
}


/**
 */
void VarTextDecoder::processWorkSpace(WorkSpace *ws) {

	// Look the _start
	Inst *start = ws->start();
	if(start) {
		if(isVerbose())
			log << "\tprocessing entry at " << start->address() << io::endl;
		processEntry(ws, start->address());
	}
	else if(isVerbose())
		log << "\tno entry to process\n";

	// Look the function symbols
	for(Process::FileIter file(ws->process()); file; file++)
		for(File::SymIter sym(file); sym; sym++)
			if(sym->kind() == Symbol::FUNCTION) {
				if(isVerbose())
					log << "\tprocessing function \"" << sym->name() << " at " << sym->address() << io::endl;
				Inst *inst = ws->findInstAt(sym->address());
				if(inst)
					processEntry(ws, sym->address());
				else
					warn(elm::_ << "bad function symbol \"" << sym->name()
						   << "\" no code segment at " << sym->address());
			}
}


/**
 * Find the instruction at the given address or raise an exception.
 * @param ws					Current workspace.
 * @param address				Address of the instruction to find.
 * @return						Instruction matching the given address.
 */
Inst *VarTextDecoder::getInst(WorkSpace *ws, otawa::address_t address) {
	Inst *inst = ws->findInstAt(address);
	if(!inst)
		warn( elm::_ << "unconsistant binary: no code segment at " << address);
	return inst;
}


/**
 * This functions follows all path from the given address.
 * @param ws		Current workspace.
 * @param address	Address of the first instruction (it must the address of
 * a actual instruction).
 */
void VarTextDecoder::processEntry(WorkSpace *ws, address_t address) {
	ASSERT(ws);
	ASSERT(address);
	TRACE("otawa::VarTextDecoder::processEntry("  << address << ")");

	// Initialize the queue
	VectorQueue<address_t> todo(QUEUE_SIZE);
	todo.put(address);

	// Repeat until there is no more address to explore
	while(!todo.isEmpty()) {

		// Get the next instruction
		address_t addr = todo.get();
		TRACE("otawa::VarTextDecoder::processEntry: starting from " << addr);
		Inst *first_inst = getInst(ws,  addr);
		Inst *inst = first_inst;

		// Follow the instruction until a branch
		address_t next;
		while(inst && !MARKER(inst) && !inst->isControl()) {
			TRACE("otawa::VarTextDecoder::processEntry: process "
				<< inst->address() << " : " << io::hex(inst->kind()));
			next = inst->topAddress();
			inst = getInst(ws, next);
		}

		// mark the block
		TRACE("otawa::VarTextDecoder::processEntry: end found");
		if(!inst) {
			warn(elm::_ << "unknown instruction at " << next);
			continue;
		}
		bool marker_found = MARKER(inst);
		MARKER(first_inst) = true;
		if(marker_found)
			continue;

		// Record target and next
		if(inst->isConditional()) {
			TRACE("otawa::VarTextDecoder::processEntry: put(" << inst->topAddress() << ")");
			todo.put(inst->topAddress());
		}
		if(!inst->isReturn() && !IS_RETURN(inst)) {
			Inst *target = 0;
			try {
				target = inst->target();
			}
			catch(ProcessException& e) {
				warn(elm::_ << e.message() << ": the branched code will not be decoded");
			}
			if(target && !NO_CALL(target)) {
				TRACE("otawa::VarTextDecoder::processEntry: put(" << target->address() << ")");
				todo.put(target->address());
			}
			else if(isVerbose() && !target)
				log << "WARNING: no target for branch at " << inst->address()
					<< io::endl;
			if(inst->isCall() && (!target || !NO_RETURN(target))) {
				TRACE("otawa::VarTextDecoder::processEntry: put(" << inst->topAddress() << ")");
				todo.put(inst->topAddress());
			}
		}
	}
}

} // otawa
