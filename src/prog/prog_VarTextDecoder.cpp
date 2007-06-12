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
	if(start)
		processEntry(ws, start->address());
	
	// Look the function symbols
	for(Process::FileIter file(ws->process()); file; file++)
		for(File::SymIter sym(file); sym; sym++)
			if(sym->kind() == Symbol::FUNCTION)
				processEntry(ws, sym->address());
}


/**
 * Find the instruction at the given address or raise an exception.
 * @param ws					Current workspace.
 * @param address				Address of the instruction to find.
 * @return						Instruction matching the given address.
 * @throws ProcessorException	If the instruction cannot be found.
 */
Inst *VarTextDecoder::getInst(WorkSpace *ws, otawa::address_t address) {
	Inst *inst = ws->findInstAt(address);
	if(!inst)
		throw ProcessorException(*this, elm::_
			<< "unconsistant binary: no code segment at " << address);
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
	TRACE("processEntry("  << address << ")");
	
	// Initialize the queue
	VectorQueue<address_t> todo(QUEUE_SIZE);
	todo.put(address);
	
	// Repeat until there is no more address to explore
	while(!todo.isEmpty()) {
		
		// Get the next instruction
		address_t addr = todo.get();
		TRACE("Starting from " << addr);
		Inst *inst = getInst(ws,  addr);
		if(MARKER(inst))
			continue;
		MARKER(inst) = true;
			
		// Follow the instruction until a branch
		while(!inst->isControl()) {
			TRACE("process(" << inst->address() << ") : "
				 << io::hex(inst->kind()));
			address_t next = inst->address() + inst->size();
			inst = getInst(ws, next);
			if(MARKER(inst))
				goto cont;	
		}
		TRACE("end found");
		
		// Record target and next
		if(inst->isConditional() || inst->isCall())
			todo.put(inst->topAddress());
		if(!inst->isReturn()) {
			Inst *target = inst->target();
			if(target) {
				TRACE("todo.put(" << target->address() << ")");
				todo.put(target->address());
			}
			else if(isVerbose())
				out << "WARNING: no target for branch at " << inst->address()
					<< io::endl;
		}
		
		cont: ;
	}
}

} // otawa
