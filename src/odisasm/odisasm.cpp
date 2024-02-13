/*
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

#include <elm/avl/Set.h>
#include <elm/data/SortedList.h>
#include <elm/io/ansi.h>
#include <elm/option/SwitchOption.h>

#include <otawa/app/CFGApplication.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/Process.h>
#include <otawa/hard/Processor.h>
#include <otawa/hard/Register.h>
#include <otawa/prog/sem.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/prog/WorkSpace.h>


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
 * @li -b, --bytes: display bytes of instructions,
 * @li -c, --condition: display condition for conditional instructions,
 * @li -k, --kind: display kind of instructions,
 * @li -r, --regs: display register information,
 * @li -p, --pipeline PROCESSOR_NAME: display execution pipeline of instructions for the given processor
 * @li -s, --semantics:	display translation of instruction into semantics language,
 * @li -t, --target: display target of control instructions
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


class ODisasm: public CFGApplication {
public:

	ODisasm(void)
	: CFGApplication(Make("odisasm", Version(1, 1, 0))
		.description("Disassemble instruction to OTAWA instruction description")
		.author("H. Cassé <hugues.casse@irit.fr>")),
	regs	(option::SwitchOption::Make(this).cmd("-r").cmd("--regs")		.description("display register information")),
	kind	(option::SwitchOption::Make(this).cmd("-k").cmd("--kind")		.description("display kind of instructions")),
	sem		(option::SwitchOption::Make(this).cmd("-s").cmd("--semantics")	.description("display translation of instruction in semantics language")),
	target	(option::SwitchOption::Make(this).cmd("-t").cmd("--target")		.description("display target of control instructions")),
	bytes	(option::SwitchOption::Make(this).cmd("-b").cmd("--bytes")		.description("display bytes composing the instruction")),
	ksem	(option::SwitchOption::Make(this).cmd("-S").cmd("--kernel-sem")	.description("display the kernel semantic instruction (without condition for conditional instructions")),
	cond    (option::SwitchOption::Make(this).cmd("-c").cmd("--condition")	.description("for architecture supporting conditional instructions, display the condition")),
	exec    (option::SwitchOption::Make(this).cmd("-e").cmd("--exec")		.description("display the steps used to execute the instruction in the microarchitecture")),
	pipeline(option::Value<string>::Make(this).cmd("-p").cmd("--pipeline")	.description("display execution pipeline of instructions for the given processor").argDescription("PROCESSOR_NAME").def("")),
	littleendian	(option::SwitchOption::Make(this).cmd("-l").cmd("--littleendian")		.description("if elf is in little endian to correctly display bytes")),
	max_size(0), proc(NULL)
	{ }

protected:

	void processTask(const CFGCollection& coll, PropList &props) override {
		require(otawa::COLLECTED_CFG_FEATURE);

		// support pipeline
		if(pipeline) {
			hard::PROCESSOR_ID(props) = pipeline;
			workspace()->require(hard::PROCESSOR_FEATURE, props);
			proc = hard::PROCESSOR_FEATURE.get(workspace());
			if(!proc)
				throw otawa::Exception("No processor found in platform");
		}

		// perform the display
		for(auto g: coll)
			processCFG(g);
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
		cout << io::BOLD << io::BRIGHT_BLUE << "# FUNCTION " << cfg->label() << io::PLAIN << io::endl;

		// put BB in the right order
		typedef elm::avl::Set<BasicBlock *, BasicBlockComparator> avl_t;
		avl_t avl;
		for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++)
			if(bb->isBasic())
				avl.add(bb->toBasic());

		// disassemble the BB
		for(avl_t::Iter bb(avl); bb(); bb++)
			for(BasicBlock::BundleIter bu(*bb); bu(); bu++) {

				// display specific to an instruction
				for(BasicBlock::Bundle::Iter i(*bu); i(); i++)
					processInst(*i);

				// display common to bundle
				processBundle(*bu);
			}
	}

	/**
	 * Disassemble properties of a bundle.
	 * @param bu	Bundle to display.
	 */
	void processBundle(const BasicBlock::Bundle& bu) {

		// display semantics
		if(sem || ksem) {
			otawa::sem::Block block;
			if(ksem)
				bu.semKernel(block);
			else
				bu.semInsts(block);
			cout << io::YELLOW << "\t\tsemantics\n" << io::PLAIN;
			otawa::sem::Printer printer(workspace()->platform());
			for(int i = 0; i < block.count(); i++) {
				cout << "\t\t\t";
				printer.print(cout, block[i]);
				cout << io::endl;
			}
		}

		// display pipeline
		if(pipeline) {
			hard::Processor::steps_t steps;
			proc->execute(bu.first(), steps);
			dumpExeGraph(steps, cout, "\t\t");
		}

	}

	/**
	 * Disassemble an instruction.
	 * @param	inst	Instruction to disassemble.
	 */
	void processInst(Inst *inst) {

		// disassemble labels
		for(Identifier<String>::Getter label(inst, FUNCTION_LABEL); label(); label++)
			cout << '\t' << io::CYAN << *label << ":\n" << io::PLAIN;
		for(Identifier<String>::Getter label(inst, LABEL); label(); label++)
			cout << '\t' << io::CYAN << *label << ":\n" << io::PLAIN;

		// display the address
		cout << inst->address();

		// display bytes of instruction (if required)
		if(bytes) {
			cout << "  ";
			if(littleendian) {
				for(t::int32 i = inst->size()-1 ; i >= 0; i--) {
					t::uint8 b;
					workspace()->process()->get(inst->address() + i, b);
					cout << io::hex(b).pad('0').width(2);
				}
			}
			else {
				for(t::uint32 i = 0; i < inst->size(); i++) {
					t::uint8 b;
					workspace()->process()->get(inst->address() + i, b);
					cout << io::hex(b).pad('0').width(2);
				}
			}
			if(inst->size() > max_size)
				max_size = inst->size();
			else
				for(t::uint32 i = inst->size(); i < max_size; i++)
					cout << "  ";
		}

		// disassemble instruction
		cout << "  " << inst << io::endl;

		// display kind if required
		if(kind) {
			cout << io::YELLOW << "\t\tkind = " << io::PLAIN;
			Inst::kind_t kind = inst->kind();
			for(int i = 0; kinds[i].kind; i++) {
				if(kinds[i].kind & kind)
					cout << kinds[i].name << ' ';
				kind &= ~kinds[i].kind;
			}
			if(kind)
				cout << " (" << io::hex(kind).pad('0') << ")";
			cout << io::endl;
		}

		// display condition
		if(cond) {
			Condition c = inst->condition();
			if(!c.isEmpty())
				cout << io::YELLOW << "\t\tcondition: " << io::PLAIN << c << io::endl;
		}

		// display target if any
		if(target) {
			if(inst->isControl() && !inst->isReturn() && !(kind & Inst::IS_TRAP)) {
				cout << io::YELLOW << "\t\ttarget = " << io::PLAIN;
				Inst *target = inst->target();
				if(!target)
					cout << "unknown";
				else
					cout << target->address();
				cout << io::endl;
			}
		}

		// display registers
		if(regs) {

			// display read registers
			SortedList<string> srr;
			RegSet rr;
			inst->readRegSet(rr);
			for(int i = 0; i < rr.count(); i++) {
				hard::Register *reg = workspace()->platform()->findReg(rr[i]);
				ASSERTP(reg, "No register found in platform for unique identifier " << rr[i]);
				srr.add(_ << reg->name() << " (" << reg->platformNumber() << ")");
			}
			cout << io::YELLOW << "\t\tread regs = " << io::PLAIN;
			for(SortedList<string>::Iter r(srr); r(); r++)
				cout << *r << " ";
			cout << io::endl;

			// display read registers
			SortedList<string> swr;
			RegSet wr;
			inst->writeRegSet(wr);
			for(int i = 0; i < wr.count(); i++) {
				hard::Register *reg = workspace()->platform()->findReg(wr[i]);
				ASSERTP(reg, "No register found in platform for unique identifier " << rr[i]);
				swr.add(_ << reg->name() << " (" << reg->platformNumber() << ")");
			}
			cout << io::YELLOW << "\t\twritten regs = " << io::PLAIN;
			for(SortedList<string>::Iter r(swr); r(); r++)
				cout << *r << " ";
			cout << io::endl;
		}

	}

	void dumpExeGraph(hard::Processor::steps_t& steps, Output out = otawa::cout, String start = "") {
		out << start << "pipeline\n";
		out << start << steps[0];
		for (int i = 1; i < steps.length(); i++) {
			if(steps[i].kind() == hard::Step::STAGE)
				out << '\n' << start;
			out << steps[i];
		}
		out << endl;
	}

	option::SwitchOption regs, kind, sem, target, bytes, ksem, cond, exec, littleendian;
	option::Value<string> pipeline;
	t::uint32 max_size;
	const hard::Processor *proc;
};

OTAWA_RUN(ODisasm);
