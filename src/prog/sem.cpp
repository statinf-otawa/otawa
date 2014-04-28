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
 * @defgroup sem Semantic Instructions
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
 * Basically, our semantic instructions have three operands and works with registers.
 * The first operand d is the target registers to store the result and next ones represents source operands (a and b).
 * The following arithmetics operations exists:
 * @li CMP -- comparison
 * @li CMPU -- unsigned comparison
 * @li ADD -- addition
 * @li SUB -- subtraction
 * @li SHL -- logical shift left
 * @li SHR -- logical shift right
 * @li ASR -- arithmetic shift right
 * @li AND -- binary and
 * @li OR -- binary inclusive or
 * @li XOR -- binary inclusive xor
 * @li MUL -- signed multiplication
 * @li MULU -- unsigned multiplication
 * @li DIV -- signed division
 * @li DIVU -- unsigned division
 * @li MOD -- signed integer division remainder
 * @li MODU -- unsigned integer division remainder
 *
 * The following instructions represents unary operations applied on the a register and stores
 * the result on the d register.
 * @li NEG -- sign negation,
 * @li NOT -- binary complement.
 *
 * There is also a SPEC semantics instruction kind that must be used by
 * instruction effects not tractable with the current semantics instruction set.
 * One using the SPEC instructions must be aware that standard usual analyses
 * will not cope with such instructions: they will need to be customized.
 *
 * The comparison puts its result in the target register that may be one of the following constants:
 * @li EQ -- a == b
 * @li LT -- a < b
 * @li LE -- a <= b
 * @li GE -- a >= b
 * @li GT -- a > b
 * @li NE -- a != b
 * @li ANY_COND -- do not know anything (recall we are performing static analysis).
 *
 * In addition, unsigned comparisons are also needed:
 * @li ULT -- a < b
 * @li ULE -- a <= b
 * @li UGE -- a >= b
 * @li UGT -- a > b
 *
 * @section sem-reg Register and Temporaries
 *
 * Operators used in the instruction (a, b and d) represents unique representation
 * of registers (as returned by Register::platformNumber()) for positive number or
 * temporaries when negative numbers are used.
 *
 * Temporary values are useful when
 * the semantics expression of an instruction is complex and requires several
 * temporary results. To alleviate the management of temporaries, they are easy
 * identified as they are represented as negative numbers and their maximum number
 * is provided by the @ref Process::tempMax(). Please, notice that the liveness
 * of a temporary must not expand out of the semantics block of an instruction !
 *
 * @section sem-anal Building Analyses
 *
 * Static analyses using instruction semantics are usually called data-flow
 * analysis. Using Abstact Interpretation, the interpretation domain is
 * usually an abstraction of the program variables state. The state includes
 * usually the value of the registers (simply identified by the register platform
 * number) and the addresses of used locations in memory. Register numbers
 * and memory location addresses grouped together forms the address set.
 *
 * So, the state becomes usually maps from addresses to value. OTAWA provides
 * already several representations for these maps.The next step is to define
 * the abstraction of the values: values are stored in registers and in memory
 * and must be specialized according to the performed analysis. For example,
 * the CLP (Cycle Linear Progression) analysis, the values are represented
 * as triplets (b, d, n) representing a set of integers (and adresses) from
 * the set {b + i d / 0 <= i < n}. But, it may be any value you want, adapted
 * to your analysis. Whatever, one must remark that addresses are usually
 * storable in registers and in memory, the value set must provides a way
 * to represent them.
 *
 * The usual map representations provide already functions to perform
 * abstract interpretation (bottom value, initial value, update, join, etc).
 * In the case of the value, you have also to provide function to perform
 * abstract interpretation but also functions to interpret the different
 * semantic instructions.
 *
 * A specific processing is devoted to SPEC instructions. A convenient analysis
 * must let its user to specialize it in order to support these instructions.
 * To achieve this goal, it must provide in the analysis a virtual function
 * that is called each time the SPEC instruction is interpreted. It would be
 * useful if this function takes as parameter the real instruction, the
 * semantics instruction and the current ab stract state. In the initial analysis,
 * this function simply do nothing but it lets customizer to overload it
 * in order to customize the interpretation.
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
	"asr",		// ASR
	"neg",		// NEG
	"not",		// NOT
	"and",		// AND
	"or",		// OR
	"xor",		// XOR
	"mul",		// MUL
	"mulu",		// MULU
	"div",		// DIV
	"divu",		// DIVU
	"mod",		// MOD
	"modu",		// MODU
	"spec"		// SPEC
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
 * A block represents a sequence of semantic instructions @ref inst.
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
 * Printer class for semantic instructions (resolve the generic register value
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
		out << ", " << inst.type();
		break;
	case SCRATCH:
		out << ' '; printArg(pf, out, inst.d());
		break;
	case SET:
	case NEG:
	case NOT:
		out << ' '; printArg(pf, out, inst.d());
		out << ", "; printArg(pf, out, inst.a());
		break;
	case SETI:
	case SETP:
		out << ' '; printArg(pf, out, inst.d());
		out << ", 0x" << io::hex(inst.cst()) << " (" << inst.cst() << ")";
		break;
	case IF:
		out << ' ' << inst.cond();
		out << ", "; printArg(pf, out, inst.sr());
		out << ", " << inst.jump();
		break;
	case CMP:
	case CMPU:
	case ADD:
	case SUB:
	case SHL:
	case SHR:
	case ASR:
	case AND:
	case OR:
	case MUL:
	case MULU:
	case DIV:
	case DIVU:
	case MOD:
	case MODU:
	case XOR:
		out << ' '; printArg(pf, out, inst.d());
		out << ", "; printArg(pf, out, inst.a()); out << ", ";
		printArg(pf, out, inst.b());
		break;
	case SPEC:
		out << ' ' << inst.d() << ", " << inst.cst();
		break;
	}
}


/**
 * Invert the given condition.
 * @param cond	Condition to invert.
 * @return		Inverted condition.
 */
cond_t invert(cond_t cond) {
	static cond_t invs[] = {
		NO_COND,	// NO_COND = 0,
		NE,			// EQ,
		GE,			// LT,
		GT,			// LE,
		LT,			// GE,
		LE,			// GT,
		NO_COND,
		NO_COND,
		ANY_COND,	// ANY_COND = 8,
		EQ,			// NE,
		UGE,		// ULT,
		UGT,		// ULE,
		ULT,		// UGE,
		ULE			// UGT,
	};
	ASSERT(cond < MAX_COND);
	return invs[cond];
}


/**
 * Get the size of the given type.
 * @param type	Type to get size for.
 * @return		Size in bytes.
 */
int size(type_t type) {
	static int sizes[] = {
			0,		// NO_TYPE = 0,
			1,		// INT8 = 1,
			2,		// INT16 = 2,
			4,		// INT32 = 3,
			8,		// INT64 = 4,
			1,		// UINT8 = 5,
			2,		// UINT16 = 6,
			4,		// UINT32 = 7,
			8,		// UINT64 = 8,
			4,		// FLOAT32 = 9,
			8		// FLOAT64 = 10
	};
	return sizes[type];
}

io::Output& operator<<(io::Output& out, type_t type) {
	static cstring labels[] = {
			"no-type",	// NO_TYPE = 0,
			"int8",		// INT8 = 1,
			"int16",	// INT16 = 2,
			"int32",	// INT32 = 3,
			"int64",	// INT64 = 4,
			"uint8",	// UINT8 = 5,
			"uint16",	// UINT16 = 6,
			"uint32",	// UINT32 = 7,
			"uint64",	// UINT64 = 8,
			"float32",	// FLOAT32 = 9,
			"float64"	// FLOAT64 = 10
	};
	ASSERT(type < MAX_TYPE);
	out << labels[type];
	return out;
}

io::Output& operator<<(io::Output& out, cond_t cond) {
	static cstring labels[] = {
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
	ASSERT(cond < MAX_COND);
	out << labels[cond];
	return out;
}


/**
 * @class PathIter
 * This iterator allows easily to traverse all execution paths of a block
 * of semantic instructions. As it may consume resources, it is delivered
 * to support iteration on multiple blocks sequentially.
 */


/**
 * @fn bool PathIter::pathEnd(void) const;
 * Test if the current instruction is a path end.
 * @return	True if it is a path end, false else.
 */


/**
 * @fn bool PathIter::isCond(void) const;
 * Test if the current instruction is a conditional, that means that
 * two different paths will be created from this point.
 * @return	True if it is a condition, false else.
 */


} }	// otawa::sem
