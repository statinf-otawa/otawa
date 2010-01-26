/*
 *	$Id$
 *	OTAWA Instruction Disassembler
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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
#include <otawa/prog/TextDecoder.h>
#include <otawa/hard/Register.h>
#include <elm/genstruct/SortedSLList.h>

using namespace elm;
using namespace otawa;

static struct {
	Inst::kind_t kind;
	cstring name;
} kinds[] = {
	{ Inst::IS_COND, "COND" },
	{ Inst::IS_CONTROL, "CONTROL" },
	{ Inst::IS_CALL, "CALL" },
	{ Inst::IS_RETURN, "RETURN" },
	{ Inst::IS_MEM, "MEM" },
	{ Inst::IS_LOAD, "LOAD" },
	{ Inst::IS_STORE, "STORE" },
	{ Inst::IS_INT, "INT" },
	{ Inst::IS_FLOAT, "FLOAT" },
	{ Inst::IS_ALU, "ALU" },
	{ Inst::IS_MUL, "MUL" },
	{ Inst::IS_DIV, "DIV" },
	{ Inst::IS_SHIFT, "SHIFT" },
	{ Inst::IS_TRAP, "TRAP" },
	{ Inst::IS_INTERN, "INTERN" },
	{ Inst::IS_MULTI, "MULTI" },
	{ Inst::IS_SPECIAL, "SPECIAL" },
	{ 0, "" }
};

class ODisasm: public Application {
public:

	ODisasm(void)
	: Application(
		"odisasm",
		Version(1, 0, 0),
		"Disassemble instruction to OTAWA instruction description",
		"H. Cassé <casse@irit.fr>") { }

	virtual void work (const string &entry, PropList &props) throw (elm::Exception) {
		require(DECODED_TEXT);
		for(Process::FileIter file(workspace()->process()); file; file++) {
			cout << "FILE: " << file->name() << io::endl;
			for(File::SegIter seg(file); seg; seg++) {
				cout << "SEGMENT: " << seg->name() << io::endl;
				for(Segment::ItemIter item(seg); item; item++) {
					cout << item->address() << " ";
					Inst *inst = item->toInst();
					if(inst) {

						// display kind
						Inst::kind_t kind = inst->kind();
						for(int i = 0; kinds[i].kind; i++)
							if(kinds[i].kind & kind)
								cout << kinds[i].name << ' ';

						// display target if any
						if(inst->isControl() && !inst->isReturn() && !(kind & Inst::IS_TRAP)) {
							cout << "\n\ttarget = ";
							Inst *target = inst->target();
							if(!target)
								cout << "unknown";
							else
								cout << target->address();
						}

						// display read registers
						genstruct::SortedSLList<string> srr;
						const elm::genstruct::Table<hard::Register * >& rr = inst->readRegs();
						for(int i = 0; i < rr.count(); i++)
							srr.add(rr[i]->name());
						cout << "\n\tread regs = ";
						for(genstruct::SortedSLList<string>::Iterator r(srr); r; r++)
							cout << *r << " ";

						// display read registers
						genstruct::SortedSLList<string> swr;
						const elm::genstruct::Table<hard::Register * >& wr = inst->writtenRegs();
						for(int i = 0; i < wr.count(); i++)
							swr.add(wr[i]->name());
						cout << "\n\twritten regs = ";
						for(genstruct::SortedSLList<string>::Iterator r(swr); r; r++)
							cout << *r << " ";
					}
					cout << io::endl;
				}
			}
		}
	}
};

int main(int argc, char **argv) {
	ODisasm app;
	return app.run(argc, argv);
}
