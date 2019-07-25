/*
 *	Inst class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-17, IRIT UPS.
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
#ifndef OTAWA_INST_H
#define OTAWA_INST_H

#include <elm/string.h>
#include <elm/io.h>
#include <elm/data/Vector.h>
#include <otawa/prog/ProgItem.h>
#include <otawa/prog/features.h>
#include <otawa/hard/Platform.h>
#include <otawa/sem/inst.h>
#include "../prop.h"

namespace otawa {

using namespace elm;

// Declaration
class Inst;
namespace hard {
	class Platform;
	class Register;
}

// Register usage
typedef Vector<t::uint16> RegSet;
class RegIter: public PreIterator<RegIter, const hard::Register *> {
public:
	inline RegIter(const RegSet& s, hard::Platform *p): set(s), i(0), pf(p) { }
	inline bool ended(void) const { return i >= set.length(); }
	inline const hard::Register *item(void) const { return pf->findReg(set[i]); }
	inline void next(void) { i++; }
private:
	const RegSet& set;
	int i;
	hard::Platform *pf;
};


// Inst class
class Inst: public ProgItem {
	friend class CodeItem;
public:
	static rtti::Type& __type;

	// Kind management
	typedef elm::t::uint32 kind_t;
	static const kind_t IS_COND		= 0x00001;
	static const kind_t IS_CONTROL	= 0x00002;
	static const kind_t IS_CALL		= 0x00004;
	static const kind_t IS_RETURN	= 0x00008;
	static const kind_t IS_MEM		= 0x00010;
	static const kind_t IS_LOAD		= 0x00020;
	static const kind_t IS_STORE	= 0x00040;
	static const kind_t IS_INT		= 0x00080;
	static const kind_t IS_FLOAT	= 0x00100;
	static const kind_t IS_ALU		= 0x00200;
	static const kind_t IS_MUL		= 0x00400;
	static const kind_t IS_DIV		= 0x00800;
	static const kind_t IS_SHIFT	= 0x01000;
	static const kind_t IS_TRAP		= 0x02000;
	static const kind_t IS_INTERN	= 0x04000;
	static const kind_t IS_MULTI 	= 0x08000;
	static const kind_t IS_SPECIAL 	= 0x10000;
	static const kind_t IS_INDIRECT	= 0x10000;
	static const kind_t IS_UNKNOWN	= 0x20000;
	static const kind_t IS_ATOMIC	= 0x40000;
	static const kind_t IS_BUNDLE	= 0x80000;

	// Kind class
	class Kind {
	public:
		inline Kind(kind_t _kind = 0): kind(_kind) { }
		inline operator kind_t(void) const { return kind; }

		inline bool meets(kind_t mask) { return (kind & mask) == mask; }
		inline bool oneOf(kind_t mask) { return kind & mask; }
		inline bool noneOf(kind_t mask) { return !oneOf(mask); }

		inline bool isAtomic()		{ return oneOf(IS_ATOMIC); }
		inline bool isBranch()		{ return oneOf(IS_CONTROL) && noneOf(IS_RETURN | IS_CALL | IS_TRAP); }
		inline bool isBundle()		{ return oneOf(IS_BUNDLE); }
		inline bool isBundleEnd()	{ return !oneOf(IS_BUNDLE); }
		inline bool isCall()		{ return oneOf(IS_CALL); }
		inline bool isCond()		{ return oneOf(IS_COND); }
		inline bool isControl()		{ return oneOf(IS_CONTROL); }
		inline bool isDiv()			{ return oneOf(IS_DIV); }
		inline bool isFloat()		{ return oneOf(IS_FLOAT); }
		inline bool isIndirect()	{ return oneOf(IS_INDIRECT); }
		inline bool isInt()			{ return oneOf(IS_INT); }
		inline bool isIntern()		{ return oneOf(IS_INTERN); }
		inline bool isLoad()		{ return oneOf(IS_LOAD); }
		inline bool isMem()			{ return oneOf(IS_MEM); }
		inline bool isMul()			{ return oneOf(IS_MUL); }
		inline bool isMulti()		{ return meets(IS_MEM | IS_MULTI); }
		inline bool isReturn()		{ return oneOf(IS_RETURN); }
		inline bool isSpecial()		{ return oneOf(IS_SPECIAL); }
		inline bool isStore()		{ return oneOf(IS_STORE); }
		inline bool isUnknown()		{ return oneOf(IS_UNKNOWN); }

		// deprecated
		inline bool isConditional()	{ return oneOf(IS_COND); }

		kind_t kind;
	};

	// null instruction
	static Inst& null;

	// Accessors
	inline Inst *nextInst(void) const
		{ ProgItem *item = next(); if(!item) return 0; else return item->toInst(); }
	inline Inst *prevInst(void) const
		{ ProgItem *item = previous(); if(!item) return 0; else return item->toInst(); }
	virtual void dump(io::Output& out);

	// Kind access
	virtual kind_t kind(void) = 0;
	inline Kind getKind(void) { return kind(); }

	inline bool isAtomic()		{ return getKind().isAtomic(); }
	inline bool isBranch()		{ return getKind().isBranch(); }
	inline bool isBundle()		{ return getKind().isBundle(); }
	inline bool isBundleEnd()	{ return getKind().isBundleEnd(); }
	inline bool isCall()		{ return getKind().isCall(); }
	inline bool isCond()		{ return getKind().isConditional(); }
	inline bool isControl() 	{ return getKind().isControl(); }
	inline bool isDiv()			{ return getKind().isDiv(); }
	inline bool isFloat()		{ return getKind().isFloat(); }
	inline bool isIndirect()	{ return getKind().isIndirect(); }
	inline bool isInt()			{ return getKind().isInt(); }
	inline bool isIntern()		{ return getKind().isIntern(); }
	inline bool isLoad()		{ return getKind().isLoad(); }
	inline bool isMem() 		{ return getKind().isMem(); }
	inline bool isMul()			{ return getKind().isMul(); }
	inline bool isMulti() 		{ return getKind().isMulti(); }
	inline bool isReturn()		{ return getKind().isReturn(); }
	inline bool isSpecial()		{ return getKind().isSpecial(); }
	inline bool isStore()		{ return getKind().isStore(); }
	inline bool isUnknown()		{ return getKind().isUnknown(); }

	// other accessors
	virtual Inst *target(void);
	virtual Type *type(void);
	virtual void semInsts(sem::Block& block);
	virtual int semInsts(sem::Block& block, int temp);
	virtual void semKernel(sem::Block& block);
	virtual int semKernel(sem::Block& block, int temp);
	virtual int semWriteBack(sem::Block& block, int temp);
	virtual delayed_t delayType(void);
	virtual int delaySlots(void);
	virtual void readRegSet(RegSet& set);
	virtual void writeRegSet(RegSet& set);

	// conditional accessors
	virtual Condition condition(void);

	// ProgItem overload
	virtual Inst *toInst(void);

	// deprecated
	virtual const Array<hard::Register *>& readRegs(void);
	virtual const Array<hard::Register *>& writtenRegs(void);
	virtual int multiCount(void);
	inline bool isConditional(void)	{ return getKind().isConditional(); }

protected:
	static const Array<hard::Register *> no_regs;
	virtual ~Inst(void) { };
};


// output
io::Output& operator<<(elm::io::Output& out, Inst *inst);
io::Output& operator<<(io::Output& out, Inst::Kind kind);

} // namespace otawa

#endif // OTAWA_INST_H
