/*
 *	$Id$
 *	BasicBlock class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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

#ifndef OTAWA_CFG_BASIC_BLOCK_H
#define OTAWA_CFG_BASIC_BLOCK_H

#include <otawa/prop/Identifier.h>
#include <elm/genstruct/SLList.h>
#include <elm/inhstruct/DLList.h>
#include <elm/utility.h>
#include <elm/PreIterator.h>
#include <otawa/program.h>
#include <otawa/instruction.h>

namespace otawa {

// Extern class
class WorkSpace;
class Edge;
class CFG;
extern Identifier<int> INDEX;

// BasicBlock class
class BasicBlock: public PropList {
	friend class CFGBuilder;
	friend class CFGInfo;
	friend class CFG;
	friend class VirtualCFG;
	friend class Edge;
protected:
	Inst *first;
	size_t _size;
	static const unsigned long FLAG_Call = 0x01;
	static const unsigned long FLAG_Unknown = 0x02;
	static const unsigned long FLAG_Return = 0x04;
	static const unsigned long FLAG_Entry = 0x08;
	static const unsigned long FLAG_Exit = 0x10;
	static const unsigned long FLAG_Virtual = 0x20;
	static const unsigned long FLAG_Cond = 0x40;
	unsigned long flags;
	elm::genstruct::SLList<Edge *> ins, outs;
	CFG *_cfg;

	// EdgeIterator class
	class EdgeIterator: public elm::genstruct::SLList<Edge *>::Iterator  {
	public:
		inline EdgeIterator(elm::genstruct::SLList<Edge *>& edges)
			: elm::genstruct::SLList<Edge *>::Iterator(edges) { };
	};

public:
	// InstIterator class
	class InstIter: public PreIterator<InstIter, Inst *> {
	public:
		inline InstIter(const BasicBlock *bb): inst(bb->first), top(bb->topAddress()) { ASSERT(bb); }
		inline InstIter(const InstIter& iter): inst(iter.inst) { }
		inline bool ended(void) const { return !inst || inst->address() >= top; }
		inline Inst *item(void) const { return inst; }
		inline void next(void)  { inst = inst->nextInst(); }
	private:
		otawa::Inst *inst;
		Address top;
	};
	typedef InstIter InstIterator;

protected:
	static BasicBlock& null_bb;
	virtual ~BasicBlock(void);
	void setTaken(BasicBlock *bb);
	void setNotTaken(BasicBlock *bb);

public:
	static Identifier<BasicBlock *> ID;

	// Constructors
	BasicBlock(void);

	// Generic accessors
	inline bool isCall(void) const { return (flags & FLAG_Call) != 0; };
	inline bool isReturn(void) const { return (flags & FLAG_Return) != 0; };
	inline bool isTargetUnknown(void) const
		{ return (flags & FLAG_Unknown) != 0; };
	inline bool isEntry(void) const { return flags & FLAG_Entry; };
	inline bool isExit(void) const { return flags & FLAG_Exit; };
	inline bool isEnd(void) const { return flags & (FLAG_Entry | FLAG_Exit); };
	inline bool isConditional(void) const { return flags & FLAG_Cond; }
	inline address_t address(void) const { return first->address(); };
	inline Address topAddress(void) const { return address() + size(); }
	virtual int countInsts(void) const;
	inline size_t size(void) const { return _size; }
	inline bool isVirtual(void) const { return flags & FLAG_Virtual; };
	inline unsigned long getFlags(void) const { return flags; };
	inline int number(void) const { return INDEX(this); };
	inline CFG *cfg(void) { return _cfg; }
	inline Inst *firstInst(void) const { return first; }
	Inst *lastInst(void) const;
	void print(io::Output& out) const;

	// Edge management
	inline void addInEdge(Edge *edge) { ins.addFirst(edge); };
	void addOutEdge(Edge *edge) { outs.addFirst(edge); };
	void removeInEdge(Edge *edge) { ins.remove(edge); };
	void removeOutEdge(Edge *edge) { outs.remove(edge); };

	// Edge iterators
	class InIterator: public EdgeIterator {
	public:
		inline InIterator(BasicBlock *bb)
			: EdgeIterator(bb->ins) { ASSERT(bb); };
	};
	class OutIterator: public EdgeIterator {
	public:
		inline OutIterator(BasicBlock *bb)
			: EdgeIterator(bb->outs) { ASSERT(bb); };
	};

	// Deprecated
	BasicBlock *getTaken(void);
	BasicBlock *getNotTaken(void);
	inline size_t getBlockSize(void) const { return size(); };
	inline int countInstructions(void) const { return countInsts(); }
};
inline Output& operator<<(Output& out, BasicBlock *bb) { bb->print(out); return out; }


// BasicBlock class
class CodeBasicBlock: public BasicBlock {
	friend class CFGInfo;
public:
	CodeBasicBlock(Inst *head);
	inline void setSize(size_t size) { _size = size; }
};


// EndBasicBlock class
class EndBasicBlock: public BasicBlock {
public:
	inline EndBasicBlock(unsigned long _flags = 0) {
		flags = _flags;
		first = null_bb.firstInst();
	};
};

} // otawa

#endif // OTAWA_CFG_BASIC_BLOCK_H
