/*
 *	$Id$
 *	sem module implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <elm/string.h>
#include <otawa/prog/sem.h>
#include <otawa/hard/Platform.h>

using namespace elm;

/**
 * @defgroup sem Semantics Instructions
 *
 * OTAWA provides an abstraction layer between the actual binary program and static analyses.
 * This abstraction is composed of generic information about the processed instruction:
 * type of instruction, target for branch instruction, read and writtent registers, and so on.
 *
 * To perform value or interval analysis, we need to understand the processing of instructions.
 * To maintain this abstraction layer, OTAWA provides with some architecture plugins a way
 * to translate instruction into a list of architecture-independent instructions to let
 * value analyses to be independent of the actual architecture.
 *
 * @section sem-set Semantics Instruction Set
 *
 * Designing a language to describe any instruction set is may an impossible task.
 * In the opposite, our semantics language has for goal to allow to perform analyses
 * while retaining only important things for such a task: basic arithmetics and comparison
 * for integer and address and integer computation, minimal flow of control to allow
 * fast analysis.
 *
 * @section sem-set-comp Computation Instructions
 *
 * Basically, our semantics instructions have three operands and works with registers.
 * The first operand d is the target registers to store the result and next ones represents source operands (a and b).
 * The following arithmetics operations exists:
 * @li CMP -- comparison
 * @li CMPU -- unsigned comparison
 * @li ADD -- addition
 * @li SUB -- subtraction
 * @li SHL -- logical shift left
 * @li SHR -- logical shift right
 * @li ASR -- arithmetic shift right
 *
 * The comparison puts its result in the target register that may be one of the following constants:
 * @li EQ -- a == b
 * @li LT -- a < b
 * @li LE -- a <= b
 * @li GE -- a >= b
 * @li GT -- a > b
 * @li NE -- a != b
 * @li ANY_COND -- do not know anything (recall we are performing static analysis).
 * In addition, some architecture requires an unsigned comparison that may produces other conditions:
 * @li ULT -- a < b
 * @li ULE -- a <= b
 * @li UGE -- a >= b
 * @li UGT -- a > b
 *
 *
 *
 * @section sem-reg Register and Temporaries
 *
 * @section sem-anal Building Analyses
 */


namespace otawa { namespace sem {

static cstring inst_names[] = {
	"nop",		//	NOP
	"branch",	// BRANCH
	"trap",		// TRAP
	"cont",		// CONT
	"if",		// IF
	"load",		// LOAD
	"store",	// STORE
	"scratch",	// SCRATCH
	"set",		// SET
	"seti",		// SETI
	"setp",		// SETP
	"cmp",		// CMP
	"cmpu",		// CMPU
	"add",		// ADD
	"sub",		// SUB
	"shl",		// SHL
	"shr",		// SHR
	"asr"		// ASR

};

static void printArg(const hard::Platform *pf, io::Output& out, signed short arg) {
	if(arg < 0) {
		out << 't' << (-arg);
		return;
	}
	if(pf) {
		hard::Register *reg = pf->findReg(arg);
		if(reg)
			out << reg->name();
		return;
	}
	out << '?' << arg;
}

static cstring cond_names[] = {
	"none",		// NO_COND = 0,
	"eq",		// EQ,
	"lt",		// LT,
	"le",		// LE,
	"ge",		// GE,
	"gt",		// GT,
	"",
	"",
	"any",		// ANY_COND = 8
	"ne",		// NE,
	"ult",		// ULT,
	"ule",		// ULE,
	"uge",		// UGE,
	"ugt"		// UGT
};


/**
 * @class inst
 * This structure class represents an instruction in the semantics representation of machine instruction.
 * It contains an opcode, giving the performed operation, and arguments depending on this opcode.
 *
 * The variable is ever used to store the result of an instruction. A variable may match a register
 * (index is positive and matches the register unique number in @ref otawa::hard::Platform description) or
 * a temporary (index is strictly negative).
 *
 * @ref LOAD, @ref STORE access memory data and uses variable a to get the address and b is an immediate
 * value given the size of the accessed data item.
 *
 * @ref SCRATCH means that the register is replaced with meaningless value.
 *
 * @ref SET and @ref SETI assigns to d the variable in b or the immediate value in cst.
 *
 * @ref CMP, @ref ADD, @ref SUB, @ref SHL, @ref SHR and @ref ASR uses both variable a and b to perform, respectively,
 * comparison, addition, subtraction, logical shift left, logical shift right, arithmetics shift right.
 */

/**
 * Output the current instruction to the given output.
 * @param out	Output to print to.
 */
void inst::print(io::Output& out) const {
	Printer printer;
	printer.print(out, *this);
}


/**
 * @class Block
 * A block represents a sequence of semantics instructions @ref inst.
 */

/**
 * Print the current block.
 * @param out	Output to print to.
 */
void Block::print(elm::io::Output& out) const {
	Printer printer;
	printer.print(out, *this);
}


/**
 * @class Printer
 * Printer class for semantics instructions (resolve the generic register value
 * to the their real platform name).
 */


/**
 * @fn Printer::Printer(hard::Platform *platform);
 * Build a semantics instruction printer using the given platform.
 * @param platform	Current platform.
 */

/**
 * Print the given block.
 * @param out	Output stream to use.
 * @param block	Block to output.
 */
void Printer::print(elm::io::Output& out, const Block& block) const {
	for(Block::InstIter inst(block); inst; inst++)
		print(out, inst);
}


/**
 * Print the given instruction.
 * @param out	Output stream to use.
 * @param inst	Semantics instruction to output.
 */
void Printer::print(elm::io::Output& out, const inst& inst) const {
	out << inst_names[inst.op];
	switch(inst.op) {
	case BRANCH:
		out << ' '; printArg(pf, out, inst.d());
		break;
	case TRAP:
		break;
	case CONT:
		break;
	case LOAD:
	case STORE:
		out << ' '; printArg(pf, out, inst.d());
		out << ", "; printArg(pf, out, inst.a());
		out << ", " << inst.b();
		break;
	case SCRATCH:
		out << ' '; printArg(pf, out, inst.d());
		break;
	case SET:
		out << ' '; printArg(pf, out, inst.d());
		out << ", "; printArg(pf, out, inst.a());
		break;
	case SETI:
	case SETP:
		out << ' '; printArg(pf, out, inst.d());
		out << ", 0x" << io::hex(inst.cst()) << " (" << inst.cst() << ")";
		break;
	case IF:
		out << ' ' << cond_names[inst.d()];
		out << ", "; printArg(pf, out, inst.a());
		out << ", " << inst.b();
		break;
	case CMP:
	case CMPU:
	case ADD:
	case SUB:
	case SHL:
	case SHR:
	case ASR:
		out << ' '; printArg(pf, out, inst.d());
		out << ", "; printArg(pf, out, inst.a()); out << ", ";
		printArg(pf, out, inst.b());
		break;
	}
}

} }	// otawa::sem
