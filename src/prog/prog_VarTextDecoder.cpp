/*
 *	$Id$
 *	VarTextDecoder Analyzer interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-10, IRIT UPS.
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
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/VarTextDecoder.h>
#include <otawa/proc/Registry.h>
#include <elm/genstruct/VectorQueue.h>
#include <otawa/program.h>
#include <otawa/util/FlowFactLoader.h>

using namespace elm;

#define QUEUE_SIZE	512

#define TRACE(m)	// cerr << m << io::endl;

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
		if(logFor(LOG_CFG))
			log << "\tprocessing entry at " << start->address() << io::endl;
		processEntry(ws, start->address());
	}
	else if(logFor(LOG_CFG))
		log << "\tno entry to process\n";

	// Look the function symbols
	for(Process::FileIter file(ws->process()); file; file++)
		for(File::SymIter sym(file); sym; sym++)
			if(sym->kind() == Symbol::FUNCTION) {
				if(IGNORE_ENTRY(sym)) {
					if(logFor(LOG_CFG))
						log << "\tignoring function symbol \"" << sym->name() << "\"\n";
				}
				else {
					if(logFor(LOG_CFG))
						log << "\tprocessing function \"" << sym->name() << " at " << sym->address() << io::endl;
					Inst *inst = ws->findInstAt(sym->address());
					if(inst)
						processEntry(ws, sym->address());
					else
						warn(elm::_ << "bad function symbol \"" << sym->name()
							   << "\" no code segment at " << sym->address());
				}
			}

	// cleanup MARKER
	for(Process::FileIter file(ws->process()); file; file++)
		for(File::SegIter seg(file); seg; seg++)
			for(Segment::ItemIter item(seg); item; item++) {
				TRACE("DEBUG: cleaning " << item->address());
				item->removeAllProp(&MARKER);
			}
}


/**
 * Get a string representing the bytes in hexadecimal of the given
 * address on the given length.
 * @param addr	Address of the area to output.
 * @param size	Size in bytes of the area to output.
 * @return		Output of the area as hexadecimal bytes.
 */
string VarTextDecoder::getBytes(Address addr, int size) {
	StringBuffer buf;
	t::uint8 byte;
	for(int i = 0; i < size; i++) {
		workspace()->process()->get(addr + i, byte);
		buf << io::f(byte).right().width(2).pad('0');
	}
	return buf.toString();
}


/**
 * Find the instruction at the given address or raise an exception.
 * @param ws					Current workspace.
 * @param address				Address of the instruction to find.
 * @param source				Source instruction (before or branching from).
 * @return						Instruction matching the given address.
 */
Inst *VarTextDecoder::getInst(WorkSpace *ws, otawa::address_t address, Inst *source) {
	try {
		Inst *inst = ws->findInstAt(address);
		if(!inst) {
			if(!source)
				warn( elm::_ << "unconsistant binary: no code segment at " << address << " from symbol");
			if(source->topAddress() == address)
				warn( elm::_ << "unconsistant binary: no code segment at " << address << " after instruction at " << source->address());
			else
				warn( elm::_ << "unconsistant binary: no code segment at " << address << "  target of branch at " << source->address());
			return 0;
		}
		else if(inst->isUnknown()) {
			int cnt = ws->process()->instSize();
			if(!cnt)
				cnt = 4;
			log << "WARNING: unknown instruction at " << address << ": " << getBytes(address, cnt) << io::endl;
			return 0;
		}
		else
			return inst;
	}
	catch(otawa::DecodingException& e) {
		log << "WARNING: " << e.message() << io::endl;
		return 0;
	}
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
	VectorQueue<Inst *> todo(QUEUE_SIZE);
	Inst *inst = getInst(workspace(), address);
	if(!inst) {
		log << "WARNING: bad function entry at " << address << io::endl;
		return;
	}
	todo.put(inst);

	// Repeat until there is no more address to explore
	while(!todo.isEmpty()) {

		// Get the next instruction
		Inst *first_inst = todo.get();
		if(!first_inst)
			continue;
		TRACE("otawa::VarTextDecoder::processEntry: starting from " << first_inst->address());
		Inst *inst = first_inst;

		// Follow the instruction until a branch
		address_t next;
		while(inst && !MARKER(inst)) {
			TRACE("otawa::VarTextDecoder::processEntry: process " << inst->address() << " : " << io::hex(inst->kind()) << ": " << inst);
			if(inst->isControl())
				break;
			next = inst->topAddress();
			inst = getInst(ws, next, inst);
		}

		// mark the block
		TRACE("otawa::VarTextDecoder::processEntry: end found");
		if(!inst) {
			warn(elm::_ << "unknown instruction at " << next);
			continue;
		}
		bool marker_found = MARKER(inst);
		MARKER(first_inst) = true;
		TRACE("otawa::VarTextDecoder::processEntry: setting marker at " << first_inst->address());
		if(marker_found) {
			TRACE("otawa::VarTextDecoder::processEntry: marker found");
			continue;
		}

		// Record target and next
		if(inst->isConditional()) {
			TRACE("otawa::VarTextDecoder::processEntry: put(" << inst->topAddress() << ")");
			todo.put(getInst(ws, inst->topAddress(), inst));
			Inst *ti = getInst(ws, inst->topAddress(), inst);
			if(!ti)
				log << "WARNING: broken sequence from " << inst->address() << " to " <<  inst->topAddress() << io::endl;
			else
				todo.put(ti);
		}
		if(!inst->isReturn() && !IS_RETURN(inst)) {
			Inst *target = 0;
			try {
				target = inst->target();
				/*if(!target)
					continue;*/
			}
			catch(ProcessException& e) {
				warn(elm::_ << e.message() << ": the branched code will not be decoded");
			}
			if(target && !NO_CALL(target)) {
				TRACE("otawa::VarTextDecoder::processEntry: put(" << target->address() << ")");
				todo.put(target);
			}
			else if(!target) {
				TRACE("otawa::VarTextDecoder::processEntry: indirect branch at " << inst->address());
				bool one = false;
				Identifier<Address> *id;
				if(inst->isCall())
					id = &CALL_TARGET;
				else
					id = &BRANCH_TARGET;
				for(Identifier<Address>::Getter target(inst, *id); target; target++) {
					TRACE("otawa::VarTextDecoder::processEntry: indirect branch to " << *target);
					one = true;
					Inst *ti = getInst(ws, target, inst);
					if(!ti) {
						log << "WARNING: broken target from " << inst->address() << " to " << *target << io::endl;
						continue;
					}
					todo.put(ti);
					TRACE("otawa::VarTextDecoder::processEntry: put(" << target << ")");
				}
				if(!one && logFor(LOG_INST))
					log << "WARNING: no target for branch at " << inst->address() << io::endl;
			}
			if(inst->isCall() && (!target || !NO_RETURN(target))) {
				TRACE("otawa::VarTextDecoder::processEntry: put(" << inst->topAddress() << ")");
				Inst *ti = getInst(ws, inst->topAddress(), inst);
				if(!ti) {
					log << "WARNING: broken target from " << inst->address() << " to " << *target << io::endl;
					continue;
				}
				todo.put(ti);
			}
		}
	}
}

} // otawa
