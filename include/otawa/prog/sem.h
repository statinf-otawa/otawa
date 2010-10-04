/*
 *	$Id$
 *	sem module interface
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

#ifndef OTAWA_SEM_H_
#define OTAWA_SEM_H_

#include <elm/io.h>
#include <elm/genstruct/Vector.h>

namespace otawa {

using namespace elm;

namespace hard { class Platform; }

namespace sem {

// type of instruction
typedef enum opcode {
	NOP = 0,
	BRANCH,		// perform a branch on content of register a
	TRAP,		// perform a trap
	CONT,		// continue in sequence with next instruction
	IF,			// continue if condition a is meet in register b, else jump c instructions
	LOAD,		// d <- MEMb(a)
	STORE,		// MEMb(a) <- d
	SCRATCH,	// d <- T
	SET,		// d <- a
	SETI,		// d <- cst
	SETP,		// page(d) <- cst
	CMP,		// d <- a ~ b
	CMPU,		// d <- a ~u b
	ADD,		// d <- a + b
	SUB,		// d <- a - b
	SHL,		// d <- a << b
	SHR,		// d <- a >> b
	ASR,		// d <- a +>> b
} opcode;


// type of conditions
typedef enum cond_t {
	NO_COND = 0,
	EQ,
	LT,
	LE,
	GE,
	GT,
	ANY_COND = 8,
	NE,
	ULT,
	ULE,
	UGE,
	UGT
} cond_t;


// inst type
typedef struct inst {
	t::uint16 op;
	t::int16 _d;
	union {
		t::uint32 cst;							// set, seti, setp
		struct { t::int16 a, b;  } regs;		// others
	} args;

	inst(void) { }
	inst(opcode _op): op(_op) { }
	inst(opcode _op, int d): op(_op)
		{ _d = d; }
	inst(opcode _op, int d, int a): op(_op)
		{ _d = d; args.regs.a = a; }
	inst(opcode _op, int d, int a, int b): op(_op)
		{ _d = d; args.regs.a = a; args.regs.b = b; }

	inline t::int16 d(void) const { return _d; }
	inline t::int16 a(void) const { return args.regs.a; }
	inline t::int16 b(void) const { return args.regs.b; }
	inline t::uint32 cst(void) const { return args.cst; }

	void print(elm::io::Output& out) const;
} inst;
inline elm::io::Output& operator<<(elm::io::Output& out, inst i) { i.print(out); return out; }

inline inst nop(void) { return inst(NOP); }
inline inst branch(int to) { return inst(BRANCH, to); }
inline inst trap(cond_t cond) { return inst(TRAP, cond); }
inline inst cont(void) { return inst(CONT); }
inline inst _if(int cond, int sr, int jump) { return inst(IF, cond, sr, jump); }
inline inst load(int d, int a, int b) { return inst(LOAD, d, a, b); }
inline inst store(int d, int a, int b) { return inst(STORE, d, a, b); }
inline inst scratch(int d) { return inst(SCRATCH, d); }
inline inst set(int d, int a) { return inst(SET, d, a); }
inline inst seti(int d, unsigned long cst) { inst i(SETI, d); i.args.cst = cst; return i; }
inline inst setp(int d, unsigned long cst) { inst i(SETP, d); i.args.cst = cst; return i; }
inline inst cmp(int d, int a, int b) { return inst(CMP, d, a, b); }
inline inst cmpu(int d, int a, int b) { return inst(CMPU, d, a, b); }
inline inst add(int d, int a, int b) { return inst(ADD, d, a, b); }
inline inst sub(int d, int a, int b) { return inst(SUB, d, a, b); }
inline inst shl(int d, int a, int b) { return inst(SHL, d, a, b); }
inline inst shr(int d, int a, int b) { return inst(SHR, d, a, b); }
inline inst asr(int d, int a, int b) { return inst(ASR, d, a, b); }


// Block class
class Block: public elm::genstruct::Vector<inst> {
	typedef elm::genstruct::Vector<inst> S;
public:
	class InstIter: public S::Iterator {
	public:
		inline InstIter(const Block& block): S::Iterator(block) { }
		inline InstIter(const InstIter& iter): S::Iterator(iter) { }
		inline opcode op(void) const { return opcode(item().op); }
		inline t::int16 d(void) const { return item().d(); }
		inline t::int16 a(void) const { return item().a(); }
		inline t::int16 b(void) const { return item().b(); }
		inline t::uint32 cst(void) const { return item().cst(); }
	};
	void print(elm::io::Output& out) const;
};
inline elm::io::Output& operator<<(elm::io::Output& out, const Block& b) { b.print(out); return out; }

// Printer class
class Printer {
public:
	inline Printer(const hard::Platform *platform = 0): pf(platform) { }

	void print(elm::io::Output& out, const Block& block) const;
	void print(elm::io::Output& out, const inst& inst) const;
private:
	const hard::Platform *pf;
};

} }	// otawa::sem

#endif /* OTAWA_SEM_H_ */
