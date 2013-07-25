/*
 *	odec -- OTAWA decoder for instructions
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#include <otawa/app/Application.h>
#include <otawa/util/FlowFactLoader.h>

using namespace elm;
using namespace otawa;

/**
 * @addtogroup commands
 * @section odec	odec Command
 *
 * This command is an helper to implement a new loader plugin. Program decoding
 * is a complex and time-consuming task implemented by @ref otawa::TextDecoder analyzer.
 * It uses the decoding ability of the loader to follow execution paths of the program.
 * Because of the complexity of this work, a lot of inconsistencies are silently
 * ignored by the decoder but these issues could be good indications about
 * something wrongly done by the implemented loader.
 *
 * As it should to costly to support verbosity switch in the decoder, this command
 * proceed as the decoder but provides as much information on the process impossible.
 * This makes easier the understanding and the retrieval of an error from a loader.
 */

// Simple marker
static Identifier<bool> MARKER("", false);

// Marker for multiple paths
static Identifier <Inst *> FROM("", 0);

// ODec application class
class ODec: public Application {
public:
	ODec(void):
		Application("ODec", Version(1, 0, 0),
			"Test the decoding of instruction of a program", "H. Cassé <casse@irit.fr>",
			"Copyright (c) 2012 - IRIT UPS")
	{
	}

protected:
	virtual void work(PropList &props) throw (elm::Exception) {
		WorkSpace *ws = workspace();

		// put the symbols
		for(Process::FileIter file(workspace()->process()); file; file++)
			for(File::SymIter sym(file); sym; sym++) {
				if(sym->kind() == Symbol::FUNCTION || sym->kind() == Symbol::LABEL) {
					Inst *inst = workspace()->findInstAt(sym->address());
					if(inst) {
						Symbol::ID(inst).add(*sym);
						switch(sym->kind()) {
						case Symbol::FUNCTION:
							FUNCTION_LABEL(inst).add(sym->name());
							break;
						case Symbol::LABEL:
							LABEL(inst).add(sym->name());
							break;
						default:
							break;
						}
					}
				}
			}

		// Look the _start
		Inst *start = ws->start();
		if(start) {
			if(isVerbose())
				cerr << "ENTRY: processing entry at " << start->address() << io::endl;
			processEntry(ws, start->address());
		}
		else if(isVerbose())
			cerr << "no entry to process\n";

		// Look the function symbols
		for(Process::FileIter file(ws->process()); file; file++)
			for(File::SymIter sym(file); sym; sym++)
				if(sym->kind() == Symbol::FUNCTION) {
					if(IGNORE_ENTRY(sym))
						cerr << "\n\nINFO: ignoring function symbol \"" << sym->name() << "\"\n";
					else {
						cerr << "\n\nENTRY: processing function \"" << sym->name() << " at " << sym->address() << io::endl;
						Inst *inst = ws->findInstAt(sym->address());
						if(inst)
							processEntry(ws, sym->address());
						else
							cerr << "bad function symbol \"" << sym->name()
								   << "\" no code segment at " << sym->address() << io::endl;
					}
				}

		// dump the instructions
		for(Process::FileIter file(workspace()->process()); file; file++) {
			cout << "FILE: " << file->name() << io::endl;
			for(File::SegIter seg(file); seg; seg++) {
				cout << "SEGMENT: " << seg->name() << io::endl;
				for(Segment::ItemIter item(seg); item; item++) {
					Inst *inst = item->toInst();
					if(inst) {
						if(MARKER(inst))
							cout << io::endl;
						for(Identifier<Symbol *>::Getter sym(inst, Symbol::ID); sym; sym++)
							cout << "\t" << sym->name() << ":\n";
						cout << "\t\t" << inst->address() << "  ";
						if(inst->isUnknown()) {
							cout << "<unknown>:";
							writeBytes(cout, inst->address(), inst->size());
						}
						else
							cout << inst;
						if(MARKER(inst)) {
							bool fst = true;
							for(Identifier<Inst *>::Getter from(inst, FROM); from; from++) {
								cout << (fst ? "\tfrom " : ", ");
								fst = false;
								cout << from->address();
							}
						}
						cout << io::endl;
					}
				}
			}
			cout << "\n";
		}
	}

private:
	void writeBytes(Output& out, Address addr, int size) {
		t::uint8 byte;
		for(int i = 0; i < size; i++) {
			workspace()->process()->get(addr + i, byte);
			out << io::right(io::hex(io::width(2, io::pad('0', byte))));
		}
	}

	Inst *getInst(WorkSpace *ws, otawa::address_t address, Inst *source = 0) {
		try {
			Inst *inst = ws->findInstAt(address);
			if(!inst) {
				if(!source)
					cerr << "ERROR: unconsistant binary: no code segment at " << address << " from symbol\n";
				if(source->topAddress() == address)
					cerr << "ERROR: unconsistant binary: no code segment at " << address << " after instruction at " << source->address() << io::endl;
				else
					cerr << "ERROR: unconsistant binary: no code segment at " << address << "  target of branch at " << source->address() << io::endl;
				return 0;
			}
			else if(inst->isUnknown()) {
				cerr << "ERROR: ERROR: unknown instruction at " << address << ": ";
				int cnt = ws->process()->instSize();
				if(!cnt)
					cnt = 4;
				writeBytes(cerr, address, cnt);
				cerr << io::endl;
				return 0;
			}
			else
				return inst;
		}
		catch(otawa::DecodingException& e) {
			cerr << "ERROR: " << e.message()<< io::endl;
			return 0;
		}
	}


	void processEntry(WorkSpace *ws, address_t address) {
		ASSERT(ws);
		ASSERT(address);

		// Initialize the queue
		VectorQueue<Inst *> todo(1024);
		Inst *inst = getInst(workspace(), address);
		if(!inst) {
			cerr << "ERROR: bad function entry at " << address << io::endl;
			return;
		}
		todo.put(inst);

		// Repeat until there is no more address to explore
		while(!todo.isEmpty()) {

			// Get the next instruction
			Inst *first_inst = todo.get();
			if(!first_inst)
				continue;
			if(isVerbose())
				cerr << "starting from " << first_inst->address() << io::endl;
			inst = first_inst;

			// Follow the instruction until a branch
			address_t next;
			while(inst && !MARKER(inst)) {
				if(isVerbose()) {
					cerr << "process " << inst->address() << " : ";
					writeBytes(cerr, inst->address(), inst->size());
					cerr << ": " << inst << io::endl;
				}
				if(inst->isControl())
					break;
				next = inst->topAddress();
				inst = getInst(ws, next, inst);
			}

			// mark the block
			if(isVerbose())
				cerr << "end found\n";
			if(!inst) {
				cerr << "WARNING: unknown instruction at " << next << io::endl;
				continue;
			}
			bool marker_found = MARKER(inst);
			MARKER(first_inst) = true;
			if(marker_found)
				continue;

			// Record target and next
			if(inst->isConditional()) {
				if(isVerbose())
					cerr << "put(" << inst->topAddress() << ")" << io::endl;
				Inst *ti = getInst(ws, inst->topAddress(), inst);
				if(!ti)
					cerr << "ERROR: broken sequence from " << inst->address() << " to " <<  inst->topAddress() << io::endl;
				else {
					FROM(ti).add(first_inst);
					todo.put(ti);
				}
			}
			if(!inst->isReturn() && !IS_RETURN(inst)) {
				Inst *target = 0;
				try {
					target = inst->target();
					if(!target)
						continue;
				}
				catch(ProcessException& e) {
					cerr << "WARNING: " << e.message() << ": the branched code will not be decoded\n";
				}
				if(target && !NO_CALL(target)) {
					if(isVerbose())
						cerr << "put(" << target->address() << ")\n";
					FROM(target).add(first_inst);
					todo.put(target);
				}
				else if(!target) {
					bool one = false;
					for(Identifier<Address>::Getter target(inst, BRANCH_TARGET); target; target++) {
						one = true;
						Inst *ti = getInst(ws, target, inst);
						if(!ti) {
							cerr << "ERROR: broken target from " << inst->address() << " to " << *target << io::endl;
							continue;
						}
						FROM(ti).add(first_inst);
						todo.put(ti);
						if(isVerbose())
							cerr << "put(" << target << ")\n";
					}
					if(!one)
						cerr << "WARNING: no target for branch at " << inst->address() << io::endl;
				}
				if(inst->isCall() && (!target || !NO_RETURN(target))) {
					if(isVerbose())
						cerr << "put(" << inst->topAddress() << ")\n";
					Inst *ti = getInst(ws, inst->topAddress(), inst);
					if(!ti) {
						cerr << "ERROR: broken target from " << inst->address() << " to " << *target << io::endl;
						continue;
					}
					FROM(ti).add(first_inst);
					todo.put(ti);
				}
			}
		}
	}

};

 OTAWA_RUN(ODec)
