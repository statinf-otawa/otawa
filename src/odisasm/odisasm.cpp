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
#include <elm/option/SwitchOption.h>
#include <otawa/cfg/features.h>
#include <elm/genstruct/AVLTree.h>
#include <otawa/prog/sem.h>

/**
 * @addtogroup commands
 * @section odisasm odisasm Command
 *
 * ODisasm disassemble a binary program and display information provided
 * by the instruction loader, that is,
 * @li instruction kinds,
 * @li branch target (for control instructions),
 * @li read and written registers,
 * @li semantic instructions (when available).
 *
 * This command is particularly useful when a loader is written.
 * It allows to examine the information provided by the loader from
 * the OTAWA run-time point of view and to find back possible errors.
 *
 * @par SYNTAX
 * @code
 * $ odisasm binary_file function1 function2 ...
 * @endcode
 *
 * The following are provided:
 * @li -r, --regs: display register information,
 * @li -k, --kind: display kind of instructions,
 * @li -s, --semantics:	display translation of instruction into semantics language,
 * @li -t, --target: display target of control instructions
 * @li -b, --byres: display bytes of instructions
 *
 * @par Example
 * @code
 * $ odisasm -ktr loop1
 * 01800248	lwz r0,12(r31)
 *	kind = MEM LOAD INT
 *	read regs = r31
 *	written regs = r0
 *0180024c	cmpi 7,0,r0,99
 *	kind = INT ALU
 *	read regs = r0 xer
 *	written regs = cr0
 *01800250	bc 4,29,-9
 *	kind = COND CONTROL
 *	target =
 *	read regs = cr0 ctr
 *	written regs =
 *01800254	lwz r0,8(r31)
 *	kind = MEM LOAD INT
 *	read regs = r31
 *	written regs = r0
 *01800258	or r3,r0,r0
 *	kind = INT ALU
 *	read regs = r0 r0
 *	written regs = r3
 *0180025c	addi r11,r31,32
 *	kind = INT ALU
 *	read regs = r31
 *	written regs = r11
 *01800260	lwz r0,4(r11)
 *	kind = MEM LOAD INT
 *	read regs = r11
 *	written regs = r0
 *01800264	mtspr lr,r0
 *	kind = INTERN
 *	read regs = r0
 *	written regs = lr
 *01800268	lwz r31,-4(r11)
 *	kind = MEM LOAD INT
 *	read regs = r11
 *	written regs = r31
 * @endcode
 */

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
		"H. Cass�� <casse@irit.fr>"),
	regs(*this, option::cmd, "-r", option::cmd, "--regs", option::help, "display register information", option::end),
	kind(*this, option::cmd, "-k", option::cmd, "--kind", option::help, "display kind of instructions", option::end),
	sem(*this, option::cmd, "-s", option::cmd, "--semantics", option::help, "display translation of instruction in semantics language", option::end),
	target(*this, option::cmd, "-t", option::cmd, "--target", option::help, "display target of control instructions", option::end),
	bytes(option::SwitchOption::Make(this).cmd("-b").cmd("--byes").description("display bytes composing the instrucion")),
	max_size(0)
	{ }

	virtual void work (const string &entry, PropList &props) throw (elm::Exception) {
		require(otawa::COLLECTED_CFG_FEATURE);
		const CFGCollection *coll = otawa::INVOLVED_CFGS(workspace());
		for(int i = 0; i < coll->count(); i++)
			processCFG(coll->get(i));

	}


private:

	class BasicBlockComparator {
	public:
		static int compare(const BasicBlock *v1, const BasicBlock *v2) { return v1->address() - v2->address(); }
	};

	/**
	 * Disassemble a CFG.
	 */
	void processCFG(CFG *cfg) {
		cout << "# FUNCTION " << cfg->label() << io::endl;

		// put BB in the right order
		typedef elm::genstruct::AVLTree<BasicBlock *, BasicBlockComparator> avl_t;
		avl_t avl;
		for(CFG::BBIterator bb(cfg); bb; bb++)
			if(!bb->isEnd())
				avl.add(bb);

		// disassemble the BB
		for(avl_t::Iterator bb(avl); bb; bb++)
			for(BasicBlock::InstIter inst(bb); inst; inst++)
				processInst(inst);
	}

	/**
	 * Disassemble an instruction.
	 * @param	inst	Instruction to disassemble.
	 */
	void processInst(Inst *inst) {

		// disassemble labels
		for(Identifier<String>::Getter label(inst, FUNCTION_LABEL); label; label++)
			cout << '\t' << *label << ":\n";
		for(Identifier<String>::Getter label(inst, LABEL); label; label++)
			cout << '\t' << *label << ":\n";

		// display the address
		cout << inst->address();

		// display bytes of instruciton (if required)
		if(bytes) {
			cout << '\t';
			for(int i = 0; i < inst->size(); i++) {
				t::uint8 b;
				workspace()->process()->get(inst->address() + i, b);
				cout << io::hex(b).pad('0').width(2);
			}
			if(inst->size() > max_size)
				max_size = inst->size();
			else
				for(int i = inst->size(); i < max_size; i++)
					cout << "  ";
		}

		// disassemble instruction
		cout << "\t" << inst << io::endl;

		// display kind if required
		if(kind) {
			cout << "\tkind = ";
			Inst::kind_t kind = inst->kind();
			for(int i = 0; kinds[i].kind; i++)
				if(kinds[i].kind & kind)
					cout << kinds[i].name << ' ';
			cout << io::endl;
		}

		// display target if any
		if(target) {
			if(inst->isControl() && !inst->isReturn() && !(kind & Inst::IS_TRAP)) {
				cout << "\ttarget = ";
				Inst *target = inst->target();
				if(!target)
					cout << "unknown";
				cout << io::endl;
			}
		}

		// display registers
		if(regs) {

			// display read registers
			genstruct::SortedSLList<string> srr;
			const elm::genstruct::Table<hard::Register * >& rr = inst->readRegs();
			for(int i = 0; i < rr.count(); i++)
				srr.add(_ << rr[i]->name() << " (" << rr[i]->platformNumber() << ")");
			cout << "\tread regs = ";
			for(genstruct::SortedSLList<string>::Iterator r(srr); r; r++)
				cout << *r << " ";
			cout << io::endl;

			// display read registers
			genstruct::SortedSLList<string> swr;
			const elm::genstruct::Table<hard::Register * >& wr = inst->writtenRegs();
			for(int i = 0; i < wr.count(); i++)
				srr.add(_ << wr[i]->name() << " (" << wr[i]->platformNumber() << ")");
			cout << "\twritten regs = ";
			for(genstruct::SortedSLList<string>::Iterator r(swr); r; r++)
				cout << *r << " ";
			cout << io::endl;
		}

		// display semantics
		if(sem) {
			otawa::sem::Block block;
			inst->semInsts(block);
			cout << "\t\tsemantics\n";
			for(int i = 0; i < block.count(); i++)
				cout << "\t\t\t" << block[i] << io::endl;
		}
	}

	option::SwitchOption regs, kind, sem, target, bytes;
	int max_size;
};

int main(int argc, char **argv) {
	ODisasm app;
	return app.run(argc, argv);
}
