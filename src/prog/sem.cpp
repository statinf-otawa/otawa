/*
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
#include <otawa/hard/Platform.h>
#include <otawa/prog/Process.h>
#include <otawa/sem/BBIter.h>
#include <otawa/sem/inst.h>
#include <otawa/sem/StateIter.h>

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
 * Basically, a machine instruction I is translated into a block of n semantic instructions [i0, i1, i2, ..., in-1].
 * To interpret a semantic block, one has to consider a semantic instruction counter, p. At beginning
 * of the interpretation, p = 0. According to the interpreted instructions, p is simply incremented (+1)
 * to perform sequential execution, or changed by adding a positive quantity n to avoid executing some instructions.
 * Notice that n >= 0 induces that no loop can appear in the interpretation of semantic block and hence no fix point
 * is required. The execution stops as soon as p >= n.
 *
 * A interpretation of a semantic i, |[i]|, can be viewed as a function taking as input (p, s) with s the
 * state of the program and producing a new (p', s'):
 *
 * |[i]|: (p, s) -> (p', s')
 *
 * The state s is a an instance of possible states, that is, of functions mapping registers R, temporaries T
 * and memory addresses A to a value in domain V:
 *
 * S: R U T U A -> V
 *
 * Set of registers depends on the underlying architecture but is mapped to IN. Temporaries are a subset of negative
 * integers but A currently represents address on 32-bits. For the remaining of the document, s[i] represents
 * the value of V in the state s that matches i, that may be a register, a temporary or an address. s[x / y]
 * represents a new state where value of y is x, that is, s' = s[x / y] = lambda i. if i = y then x else s[i].
 *
 * The algorithm to interpret a semantic block follows (s0 is the initial state):
 * @code
 *	p <- 0
 *	s <- s0
 *	WHILE i < n DO
 *		(p, s) <- |[ip]| (p, s)
 * @endcode
 *
 * The following sections describes the semantics of each instruction.
 *
 *
 * @section sem-set-comp Computation Instructions
 *
 * Basically, our semantic instructions have three operands and works with registers.
 * The first operand d is the target registers to store the result and next ones represents source operands (a and b).
 * The following arithmetics operations exists:
 * @li ADD -- addition,
 * @li SUB -- subtraction,
 * @li SHL -- logical shift left,
 * @li SHR -- logical shift right,
 * @li ASR -- arithmetic shift right,
 * @li AND -- binary and,
 * @li OR -- binary inclusive or,
 * @li XOR -- binary inclusive xor,
 * @li MUL -- signed multiplication,
 * @li MULU -- unsigned multiplication,
 * @li MULH -- higher order bits of multiplication,
 * @li DIV -- signed division,
 * @li DIVU -- unsigned division,
 * @li MOD -- signed integer division remainder,
 * @li MODU -- unsigned integer division remainder.
 *
 * The following instructions represents unary operations applied on the a register and stores
 * the result on the d register.
 * @li NEG -- sign negation,
 * @li NOT -- binary complement.
 *
 * Below is given the semantics of each instruction where:
 * @li a, b, c represents registers,
 * @li i represents 32-bits immediate value,
 * @li operators applies on unsigned values except if they are followed by "+".
 *
 * @code
 * |[add (d, a, b)]| (p, s) = (p + 1, s[s[a] +   s[b] / d])
 * |[sub (d, a, b)]| (p, s) = (p + 1, s[s[a] -   s[b] / d])
 * |[shl (d, a, b)]| (p, s) = (p + 1, s[s[a] <<  s[b] / d])
 * |[shr (d, a, b)]| (p, s) = (p + 1, s[s[a] >>  s[b] / d])
 * |[asr (d, a, b)]| (p, s) = (p + 1, s[s[a] >>+ s[b] / d])
 * |[and (d, a, b)]| (p, s) = (p + 1, s[s[a] &   s[b] / d])
 * |[or  (d, a, b)]| (p, s) = (p + 1, s[s[a] |   s[b] / d])
 * |[xor (d, a, b)]| (p, s) = (p + 1, s[s[a] ^   s[b] / d])
 * |[mul (d, a, b)]| (p, s) = (p + 1, s[s[a] *   s[b] / d])
 * |[mulu(d, a, b)]| (p, s) = (p + 1, s[s[a] *+  s[b] / d])
 * |[mulh(d, a, b)]| (p, s) = (p + 1, s[(s[a] *  s[b]) >> bitlength(d) / d])
 * |[div (d, a, b)]| (p, s) = (p + 1, s[s[a] /   s[b] / d])
 * |[divu(d, a, b)]| (p, s) = (p + 1, s[s[a] /+  s[b] / d])
 * |[mod (d, a, b)]| (p, s) = (p + 1, s[s[a] %   s[b] / d])
 * |[modu(d, a, b)]| (p, s) = (p + 1, s[s[a] %+  s[b] / d])
 * |[neg (d, a)   ]| (p, s) = (p + 1, s[-s[a]         / d])
 * |[not (d, a)   ]| (p, s) = (p + 1, s[~s[a]         / d])
 * @endcode
 *
 *
 * @section sem-set-inst Set Instructions
 *
 * There are four set instructions:
 * @li to transfer content of a register to another one, @p set,
 * @li to get a constant 32-bits value in a register, @p seti,
 * @li to assign undefined T value to a register, @p scratch,
 * @li to set the page of an address for multi-address space architectures, @p setp (very rarely used).
 *
 * Their semantics is described below (i is a 32-bits value):
 *
 * @code
 * |[set    (d, a)]| (p, s) = (p + 1, s[s[a] / d])
 * |[seti   (d, i)]| (p, s) = (p + 1, s[i / d])
 * |[scratch(d)   ]| (p, s) = (p + 1, s[T / d])
 * |[setp   (d, i)]| (p, s) = (p + 1, s[p :: s[d] / d])
 * @endcode
 *
 *
 * @section sem-flow Flow Instructions
 *
 * In semantic instructions, there are two types of control flow: machine flow corresponds to PC assignment
 * in the underlying instruction set and is supported by CFG; semantic flow that controls the execution
 * flow of semantic instruction. The only instruction concerning the machine control flow is @p branch
 * that gives the new address stored in the PC but it doesn't change the semantic instruction counter p.
 *
 * The semantic control flow is handled by three instructions, @p cont that stops the execution, @p fork
 * that creates two executions paths and @p assume that assumes a condition true. The condition used in
 * @p assume is generated from a comparison result, a ~ b (a compared to b), generated by either a
 * @p cmp instruction, or @cmpu instruction. The semantics of the condition are:
 * @li EQ -- a == b
 * @li LT -- a < b
 * @li LE -- a <= b
 * @li GE -- a >= b
 * @li GT -- a > b
 * @li NE -- a != b
 * @li ANY_COND -- do not know anything (recall we are performing static analysis).
 * @li ULT -- a < b
 * @li ULE -- a <= b
 * @li UGE -- a >= b
 * @li UGT -- a > b
 *
 * Below is the semantics of the flow instructions (c is a condition and c[a] returns true
 * if condition c is satisfied in register a):
 *
 * @code
 * |[branch(a)       ]| (p, s) = (p + 1, s)		// PC <- s[a]
 * |[cmp   (d, a, b) ]| (p, s) = (p + 1, s[s[a] ~+ s[b] / d])
 * |[cmpu  (d, a, b) ]| (p, s) = (p + 1, s[s[a] ~  s[b] / d])
 * |[cont            ]| (p, s) = (n, s)
 * |[assume(c, a)    ]| (p, s) = (p + 1, s and c[a] = true)
 * |[fork(k)         ]| (p, s) = (p + 1, s) or (p + k, s)
 * @endcode
 *
 * In OTAWA 2, @p if is considered deprecated as it the composition of @p fork and @p assume.
 * However, some instruction sets may keep to use it so its semantics is given below:
 * @code
 * |[if    (c, a, k) ]| (p, s) = if c[a] then (p + 1, s) else (p + k, s)
 * @endcode
 *
 * @section sem-mem Memory Access Instruction
 *
 * There are only two instructions used to perform memory access:
 * @li @p load allows to get a value from memory using its address and its type,
 * @li @p store allows to store a register value to a memory address using the given type.
 *
 * Notice that the types are required to realize the bytes modified in memory
 * and to generate correct value in register (including sign extension). The types may be one of:
 * @li int8 -- signed integer on 8-bits,
 * @li int16 -- signed integer on 16-bits,
 * @li int32 -- signed integer on 32-bits,
 * @li int64 -- signed integer on 64-bits,
 * @li uint8 -- unsigned integer on 8-bits,
 * @li uint16 -- unsigned integer on 16-bits,
 * @li uint32 -- unsigned integer on 32-bits,
 * @li uint64 -- unsigned integer on 64-bits.
 *
 * The semantics of these instructions follows (t is the type, s(t) represents access to state with type t):
 * @code
 * |[load (d, a, t)]| (p, s) = (p + 1, s[s(t)[s[a] / d])
 * |[store(d, a, t)]| (p, s) = (p + 1, s(t)[d / s[b]])
 * @endcode
 *
 * @section sem-spec Special Instruction
 *
 * There is also a @spec semantics instruction kind that must be used by
 * instruction effects not tractable with the current semantics instruction set.
 * One using the SPEC instructions must be aware that standard usual analyses
 * will not cope with such instructions: they will need to be customized.
 * Therefore, the semantics of @p spec is undefined.
 *
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
 * To help the developer supports the multiple execution
 * path of a semantic bloc, one can use the class @ref otawa::sem::PathIter that
 * works like a usual iterator but provides also indications of the executed
 * paths and instructions:
 * @li op, d, a, b, cst, reg, addr -- access to instruction description,
 * @li pathEnd() -- true if execution is at end,
 * @li isFork() -- true if the current instruction is a @p if, meaning that two paths will start from this point.
 *
 * A common way to use @ref otawa::sem::PathIter is to maintain a stack of states where the current state
 * is pushed when an @p if is found and a state is popped when an execution path ends. The different states
 * obtained at each end of an execution path can be joined:
 * @code
 * 	Vector<State> stack;
 * 	State s = initial_state, result = bottom_state;
 * 	stack.push(s);
 * 	for(sem::PathIter i(machine_instruction); i; i++) {
 * 		if(i.pathEnd()) {
 * 			result = join(result, s);
 * 			s = stack.pop();
 * 		}
 * 		else {
 * 			if(i.isFork())
 *				stack.push(s);
 *			s = apply(i, s);
 * 		}
 * 	}
 * 	return result;
 * @endcode
 *
 * A specific processing is devoted to SPEC instructions. A convenient analysis
 * must let its user to specialize it in order to support these instructions.
 * To achieve this goal, it must provide in the analysis a virtual function
 * that is called each time the SPEC instruction is interpreted. It would be
 * useful if this function takes as parameter the real instruction, the
 * semantics instruction and the current abstract state. In the initial analysis,
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
	"spec",		// SPEC
	"mulh",		// MULH
	"assume",	// ASSUME
	"fork"		// FORK
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
 *
 * @ingroup sem
 */

/**
 * Output the current instruction to the given output.
 * @param out	Output to print to.
 */
void inst::print(io::Output& out, const hard::Platform *pf) const {
	Printer printer(pf);
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
void Block::print(elm::io::Output& out, const hard::Platform *pf) const {
	Printer printer(pf);
	printer.print(out, *this);
}

/**
 * Restructure the instruction such that it doesn't contain any IF. The code is
 * a list of sequential semantic instruction (a) chained by FORK instructions
 * (except the last sequence) and where all IF are also replaced by ASSUME.
 * Notice that instruction not executed are replaced by NOP. In each sequence,
 * there is as many instruction as in the initial sequence (not counting the
 * fork).
 * 
 * For example, the code below:
 * ```
 *	ADD(t1, R1, R2)
 *	CMP (SR, R0, t1)
 *  IF(EQ, SR, 2)
 *  SUB(R1, R1, R2)
 *	ADD(R0, R0, t1)
 * 	CONT
 * ```
 * Will be replaced by:
 * ```
 *	FORK(5)
 *	CMP(SR, R0, t1)
 *	ASSUME(EQ, SR)
 *  SUB(R1, R1, R2)
 *	ADD(R0, R0, t1)
 * 	CONT
 *	
 *	NOP
 *	CMP(SR, R0, t1)
 *	ASSUME(NE, SR)
 *	NOP
 *	NOP
 *	CONT
 * ```
 */
void Block::fork() {
	
	// look for an IF
	if(!elm::exists(*this, [](const inst& i) { return i.op == IF; }))
		return;

	// prepare array
	int l = length();
	setLength(l + 1);
	for(int i = l-1; i >= 0; i--)
		(*this)[i+1] = (*this)[i];
	(*this)[0] = sem::nop();
	
	// process the current sequence
	fork(0, l, 1);
}

/**
 * Work function for Block::fork().
 * @param b		Based index of the current path.
 * @param l		Code length (without FORK).
 * @param i		Index of the forked branch (relative to base).
 */
void Block::fork(int b, int l, int _i) {
	auto i = _i;
	for(; i <= l; i++)
		if((*this)[b + i].op == IF) {
			int nb = length();
			int js = (*this)[b + i].jump();
			
			// fix fork and condition
			(*this)[b + i] = sem::assume((*this)[b + i].cond(), (*this)[b + i].sr());
			(*this)[length() -l - 1] = sem::fork(l);
			
			// add fork and copy the head
			add(sem::nop());
			for(int j = 1; j < i; j++) {
				auto x = (*this)[b + j];
				add(x);
			}
				
			// make the assume and the nop
			add(sem::assume(invert((*this)[b + i].cond()), (*this)[b + i].sr()));
			for(int j = 0; j < js; j++)
				add(sem::nop());
			
			// copy the tail
			for(int j = i + js + 1; j <= l; j++) {
				auto x = (*this)[b + j];
				add(x);
			}
			
			// run processing of new sequence
			fork(nb, l, i + 1);
		}
}


/**
 * @class Printer
 * Printer class for semantic instructions (resolve the generic register value
 * to the their real platform name).
 * @ingroup sem
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
void Printer::print(elm::io::Output& out, const Block& block, const hard::Platform *plf) const {
	for(Block::InstIter inst(block); inst(); inst++) {
		print(out, *inst, plf);
		out << io::endl;
	}
}


/**
 * Print the given instruction.
 * @param out	Output stream to use.
 * @param inst	Semantics instruction to output.
 */
void Printer::print(elm::io::Output& out, const inst& inst, const hard::Platform *plf) const {
	const hard::Platform *pf2;
	if(!pf)
		pf2 = plf;
	else
		pf2 = pf;
	out << inst_names[inst.op];
	switch(inst.op) {
	case BRANCH:
		out << ' '; printArg(pf2, out, inst.d());
		break;
	case TRAP:
		break;
	case CONT:
		break;
	case LOAD:
	case STORE:
		out << ' '; printArg(pf2, out, inst.d());
		out << ", "; printArg(pf2, out, inst.a());
		out << ", " << inst.type();
		break;
	case SCRATCH:
		out << ' '; printArg(pf2, out, inst.d());
		break;
	case SET:
	case NEG:
	case NOT:
		out << ' '; printArg(pf2, out, inst.d());
		out << ", "; printArg(pf2, out, inst.a());
		break;
	case SETI:
	case SETP:
		out << ' '; printArg(pf2, out, inst.d());
		out << ", 0x" << io::hex(inst.cst()) << " (" << inst.cst() << ")";
		if (inst.cst() > type_info<uint_t>::max)
			out << " (" << (t::int32)inst.cst() << ")";
		break;
	case IF:
		out << ' ' << inst.cond();
		out << ", "; printArg(pf2, out, inst.sr());
		out << ", " << inst.jump();
		break;
	case ASSUME:
		out << ' ' << inst.cond();
		out << ", "; printArg(pf2, out, inst.sr());
		break;
	case FORK:
		out << ' ' << inst.jump();
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
	case MULH:
	case DIV:
	case DIVU:
	case MOD:
	case MODU:
	case XOR:
		out << ' '; printArg(pf2, out, inst.d());
		out << ", "; printArg(pf2, out, inst.a()); out << ", ";
		printArg(pf2, out, inst.b());
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
 * @ingroup sem
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
 * Reflect the condition as the operator arguments was exchanged.
 * No effect on EQ and NE operators.
 * @param cond	Condition to reflect.
 * @return		Reflected condition.
 */
cond_t reflect(cond_t cond) {
	static cond_t map[] = {
		NO_COND,	// NO_COND = 0,
		EQ,			// EQ,
		GT,			// LT,
		GE,			// LE,
		LE,			// GE,
		LT,			// GT,
		NO_COND,
		NO_COND,
		ANY_COND,	// ANY_COND = 8,
		NE,			// NE,
		UGT,		// ULT,
		UGE,		// ULE,
		ULE,		// UGE,
		ULT			// UGT,
	};
	ASSERT(cond < MAX_COND);
	return map[cond];
	
}


/**
 * If not already, make a condition unsigned.
 * @param cond	Condition to transform.
 * @return		Unsigned version of the condition.
 */
cond_t unsignedCond(cond_t cond) {
	static cond_t map[] = {
		NO_COND,	// NO_COND = 0,
		EQ,			// EQ,
		UGT,		// LT,
		UGE,		// LE,
		ULE,		// GE,
		ULT,		// GT,
		NO_COND,
		NO_COND,
		ANY_COND,	// ANY_COND = 8,
		NE,			// NE,
		UGT,		// ULT,
		UGE,		// ULE,
		ULE,		// UGE,
		ULT			// UGT,
	};
	ASSERT(cond < MAX_COND);
	return map[cond];	
}


/**
 * If not already, make a condition signed.
 * @param cond	Condition to transform.
 * @return		Unsigned version of the condition.
 */
cond_t signedCond(cond_t cond) {
	static cond_t map[] = {
		NO_COND,	// NO_COND = 0,
		EQ,			// EQ,
		GT,			// LT,
		GE,			// LE,
		LE,			// GE,
		LT,			// GT,
		NO_COND,
		NO_COND,
		ANY_COND,	// ANY_COND = 8,
		NE,			// NE,
		GT,			// ULT,
		GE,			// ULE,
		LE,			// UGE,
		LT			// UGT,
	};
	ASSERT(cond < MAX_COND);
	return map[cond];	
}


/**
 * Test if a condition concerns signed numbers or not.
 * EQ and NE are considered to apply equally on signed and unsigned
 * numbers.
 * @param cond	Type to test.
 * @return		True if cond concerns signed numbers, false else.
 * @ingroup sem
 */
bool isSigned(cond_t cond) {
	return EQ <= cond  && cond <= NE;
}


/**
 * Test if a condition concerns unsigned numbers or not.
 * EQ and NE are considered to apply equally on signed and unsigned
 * numbers.
 * @param cond	Type to test.
 * @return		True if cond concerns signed numbers, false else.
 * @ingroup sem
 */
bool isUnsigned(cond_t cond) {
	return cond == EQ || cond >= ANY_COND;
}


/**
 * Get the size of the given type.
 * @param type	Type to get size for.
 * @return		Size in bytes.
 * @ingroup sem
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
 * @ingroup sem
 */


/**
 * Start interpretation of an instruction.
 * @param inst	Instruction to interpret.
 */
void PathIter::start(Inst *inst) {
	bb.clear();
	inst->semInsts(bb);
	todo.clear();
	pc = 0;
	bb.add(sem::cont());
}


/**
 * Start interpretation of the initialization semantic instructions
 * of a process.
 */
void PathIter::start(Process *proc) {
	bb.clear();
	proc->semInit(bb);
	todo.clear();
	pc = 0;
	bb.add(sem::cont());
}


/**
 * Start interpretation of a semantic block.
 * @param block		Block to interpret.
 */
void PathIter::start(const sem::Block& block) {
	bb = block;
	todo.clear();
	pc = 0;
	bb.add(sem::cont());
}


/**
 * Go to next semantic instruction.
 */
void PathIter::next(void) {
	if(pathEnd())
		pc = todo.pop();
	else {
		if(isFork())
			todo.push(pc + bb[pc].jump() + 1); // because the target of the jump is how many semantic instructions to skip, so we need to add 1 to get the one we are interested
		pc++;
	}
}


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

/**
 * @class BBIter
 * This class is an iterator on the semantic instruction paths of a basic block.
 * Basically, a machine instruction is made of semantic instructions that may exhibit
 * several executions paths (but no loop). This iterator enumerates all instructions
 * of a basic block and for each instruction, all semantic paths of each instruction.
 * As this iterator is relatively expansive, it should be allocated once and restarted
 * for each basic block using start() method.
 *
 * Several accessors allow to get the current execution point:
 * @li item() gets the current semantic instruction,
 * @li instruction() gets the current machine instruction.
 *
 * This class provides also several end methods for:
 * @li ended() for the end of the basic block,
 * @li instEnd() for the end of a particular instruction (all semantic paths),
 * @li pathEnd() for the end of a particular semantic execution path.
 *
 */

/**
 * Start the traversal of a different basic block.
 * @param bb	Basic block to start with.
 */
void BBIter::start(BasicBlock *bb) {
	i = BasicBlock::InstIter(bb);
	if(*i)
		si.start(*i);
}

/**
 * @fn bool BBIter::pathEnd(void) const;
 * Test if the iteration point is at the end of an semantic execution path.
 * @return	True if it is the end of a semantic instruction path, false else.
 */

/**
 * @fn bool BBIter::isCond(void) const;
 * Test if the current instruction is a condition, that is a fork between two
 * semantic execution paths.
 * @return	True if the current instruction is a condition, false else.
 */

/**
 * @fn bool BBIter::instEnd(void) const;
 * Test if the iterator is at the end of a machine instruction.
 * @return	True if at end of machine instruction, false else.
 */

/**
 * @fn bool BBIter::ended(void) const;
 * Test if the iterator has ended the block.
 * @return	True if at end of the block, false else.
 */

/**
 * @fn inst BBIter::item(void) const;
 * Get the current semantic instruction.
 * @return	Current semantic instruction.
 */

/**
 * @fn Inst *BBIter::instruction(void) const;
 * Get the current machine instruction.
 * @return	Current machine instruction.
 */

/**
 * Go to next semantic instruction (including changing
 * instruction or changing execution path).
 */
void BBIter::next(void) {
	if(si.ended()) {
		i++;
		if(i())
			si.start(*i);
		else
			return;
	}
	else
		si.next();
}

/**
 * Go to the start of the next instruction, performing
 * the remaining execution paths of the current instruction.
 */
void BBIter::nextInst(void) {
	next();
	while(!instEnd())
		next();
}

/**
 * Go to the end of the basic block.
 */
void BBIter::toEnd(void) {
	while(!ended())
		next();
}


/**
 * @class StateIter
 * This class, based on @ref BBIter, allows to traverse all semantic execution paths
 * of a basic block while maintaining the consistency of the analysis state. The parameter
 * class must be implement the @ref StateManager concept described below:
 *
 * @code
 * class StateManager {
 * public:
 * 		typedef ... t;				// type of states
 * 		t bot(void);				// bottom least value
 * 		void free(t s);				// free a state
 * 		bool equals(t s1, t s2);	// test for equality
 * 		t copy(t s);				// provide a fresh copy of state (fried by free())
 * 		t join(t s1, t s2);			// join both states
 * };
 * @endcode
 * And type StateManager::t must support the assignment operation.
 *
 * @param S		State manager (must implement StateManager concept above).
 */

/**
 * @fn StateIter::StateIter(S& manager);
 * Build a state iterator using the given state manager.
 * @param manager	State manager.
 */

/**
 * @fn S::t& StateIter::state(void) const;
 * Get the state matching the current point.
 */


} }	// otawa::sem
