/*
 *	$Id$
 *	CFG class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-07, IRIT UPS.
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
#ifndef OTAWA_CFG_CFG_H
#define OTAWA_CFG_CFG_H

#include <elm/assert.h>
#include <otawa/prog/Inst.h>
#include <otawa/prop/PropList.h>
#include <otawa/sgraph/DiGraph.h>

using namespace elm;

namespace otawa {

class BasicBlock;
class Block;
class CFG;
class CFGMaker;
class SynthBlock;

class Edge: public PropList, public sgraph::GenEdge<Block, Edge> {
	friend class CFGMaker;
public:
	inline Edge(t::uint32 flags): _flags(flags) { }
	inline Block *target(void) const { return sink(); }
	inline bool isNotTaken(void) const { return _flags & NOT_TAKEN & !isBoth(); }
	inline bool isTaken(void) const { return !isNotTaken() & !isBoth(); }
	inline bool isBoth(void) const { return (_flags & TAKEN) && (_flags & NOT_TAKEN); }
	inline t::uint32 flags(void) const { return _flags; }
	static const t::uint32 NOT_TAKEN = 0x00000001;
	static const t::uint32 TAKEN = 0x00000002;	// if the PC is altered
	inline bool isForward(void) const;
	inline bool isBackward(void) const { return !isForward(); }
private:
	t::uint32 _flags;
};
io::Output& operator<<(io::Output& out, Edge *edge);


class Block: public PropList, public sgraph::GenVertex<Block, Edge> {
	friend class CFGMaker;
public:
	static const t::uint16
		IS_END   = 0 << 0,
		IS_BASIC = 1 << 0,
		IS_SYNTH = 2 << 0,
		IS_ENTRY = 0 << 2,
		IS_EXIT  = 1 << 2,
		IS_UNKN	 = 2 << 2,
		IS_VIRT  = 3 << 2,
		IS_CALL  = 0x2 << 0,
		MASK1 	 = 0x3,
		MASK12	 = MASK1 | (0x3 << 2);

	inline bool isEnd(void)   const   { return (_type & MASK1)  == IS_END; }
	inline bool isEntry(void) const   { return (_type & MASK12) == (IS_END | IS_ENTRY); }
	inline bool isExit(void)  const   { return (_type & MASK12) == (IS_END | IS_EXIT); }
	inline bool isUnknown(void) const { return (_type & MASK12) == (IS_END | IS_UNKN); }
	inline bool isVirtual(void) const { return (_type & MASK12) == (IS_END | IS_VIRT); }
	inline bool isSynth(void) const   { return (_type & MASK1)  == IS_SYNTH; }
	inline bool isCall(void)  const   { return (_type & MASK12) == (IS_SYNTH | IS_CALL); }
	inline bool isBasic(void) const   { return (_type & MASK1)  == IS_BASIC; }

	inline int id(void) const;
	inline BasicBlock *toBasic(void);
	inline SynthBlock *toSynth(void);
	inline operator BasicBlock *(void) { return toBasic(); }
	inline operator SynthBlock  *(void) { return toSynth(); }
	inline CFG *cfg(void) const { return _cfg; }
	inline Address address(void);

protected:
	Block(t::uint16 type = IS_BASIC);

private:
	t::uint16 _type;
	EdgeIter ntak;
	CFG *_cfg;
};
io::Output& operator<<(io::Output& out, Block *block);
inline io::Output& operator<<(io::Output& out, BasicBlock *block)
	{ return out << (Block *)block; }
inline io::Output& operator<<(io::Output& out, SynthBlock *block)
	{ return out << (Block *)block; }

class SynthBlock: public Block {
	friend class CFGMaker;
public:
	SynthBlock(t::uint32 type = IS_CALL);
	inline CFG *callee(void) const { return _callee; }
	inline CFG *caller(void) const { return cfg(); }
	Inst *callInst(void);
	inline Address address(void) const;
private:
	CFG *_callee;
};


class BasicBlock: public Block {
public:
	BasicBlock(const genstruct::Table<Inst *>& insts);
	~BasicBlock(void);

	inline Address address(void) const { return first()->address(); }
	int size(void);
	inline Address topAddress(void) { return address() + size(); }

	inline Inst *first(void) const { return _insts[0]; }
	Inst *control(void);
	Inst *last(void);
	int count(void) const;

	class InstIter: public AllocatedTable<Inst *>::Iterator {
	public:
		inline InstIter(void) { }
		inline InstIter(const BasicBlock *bb): AllocatedTable<Inst *>::Iterator(bb->_insts) { }
	};
	inline InstIter insts(void) const { return InstIter(this); }

	class BundleIter: public PreIterator<BundleIter, Inst *> {
	public:
		inline BundleIter(void) { }
		inline BundleIter(const BasicBlock *bb): _iter(bb) { }
		inline Inst * const item(void) const {return _iter.item(); }
		inline bool ended(void) const { return _iter.ended(); }
		inline void next(void) { while(!ended() && item()->isBundle()) _iter.next(); _iter.next(); }
	private:
		InstIter _iter;
	};
	inline BundleIter bundles(void) const { return BundleIter(this); }

	class BasicEdge {
	public:
		inline BasicEdge(void): v(0), w(0), e(0) { }
		inline BasicEdge(BasicBlock *source, Edge *edge, BasicBlock *sink)
			: v(source), w(sink), e(edge) { }
		inline BasicBlock *source(void) const { return v; }
		inline BasicBlock *sink(void) const { return w; }
		inline Edge *edge(void) const { return e; }
	private:
		BasicBlock *v, *w;
		Edge *e;
	};

	class BasicIns: public PreIterator<BasicIns, BasicEdge> {
	public:
		BasicIns(BasicBlock *bb);
		inline bool ended(void) const { return !e.sink(); }
		inline const BasicEdge& item(void) const { return e; }
		void next(void);
	private:
		BasicEdge e;
		genstruct::Vector<Pair<Edge *, Edge *> > wl;
	};
	inline BasicIns basicIns(void) { return BasicIns(this); }

private:
	DeletableTable<Inst *> _insts;
};

class CFG: public PropList, public sgraph::GenDiGraph<Block, Edge> {
	friend class CFGMaker;
	friend class CFGCollection;
public:
	typedef t::uint8 type_t;
	static const type_t
		NONE = 0,
		SUBPROG = 1,
		SYNTH = 2,
		USER = 128;

	~CFG(void);

	typedef VertexIter BlockIter;
	typedef genstruct::SLList<SynthBlock *>::Iterator CallerIter;

	string label(void);
	string name(void);
	string format(const Address& addr);
	inline int index(void) const { return idx; }
	inline int offset(void) const { return _offset; }
	inline Inst *first(void) const { return fst; }
	inline Address address(void) const { return first()->address(); }
	inline Block *exit(void) const { return _exit; }
	inline Block *unknown(void) const { return _unknown; }
	inline BlockIter blocks(void) const { return vertices(); }
	inline type_t type(void) const { return _type; }
	inline CallerIter callers(void) const { return CallerIter(_callers); }

private:
	CFG(Inst *first, type_t type = SUBPROG);
	int idx, _offset;
	type_t _type;
	Inst *fst;
	Block *_exit, *_unknown;
	genstruct::SLList<SynthBlock *> _callers;
};
io::Output& operator<<(io::Output& out, CFG *cfg);
inline io::Output& operator<<(io::Output& out, const CFG::BlockIter& i) { return out << *i; }


// delayed inlines
//inline Block *Edge::target(void) const	{ return sink(); }
inline bool Edge::isForward(void) const { ASSERT(isTaken()); return source()->address() < sink()->address(); }
inline int Block::id(void) const { return index() + _cfg->offset(); }
inline BasicBlock *Block::toBasic(void) { ASSERT(isBasic()); return static_cast<BasicBlock *>(this); }
inline SynthBlock *Block::toSynth(void) { ASSERT(isCall());  return static_cast<SynthBlock  *>(this); }
Output& operator<<(Output& out, Block *b);
inline Address Block::address(void)
	{ if(isBasic()) return toBasic()->address(); if(isSynth()) return toSynth()->address(); else return Address::null; }
inline Address SynthBlock::address(void) const
	{ if(_callee) return _callee->first()->address(); else return Address::null; }


class CFGMaker: public PropList, public sgraph::GenDiGraphBuilder<Block, Edge>  {
public:
	CFGMaker(Inst *first, bool fix = false);
	inline Block *entry(void) const { return cfg->entry(); }
	Block *exit(void);
	Block *unknown(void);
	CFG *build(void);
	void add(Block *v);
	void call(SynthBlock *v, CFG *cfg);
	void call(SynthBlock *v, const CFGMaker& cfg);
	inline void add(Block *v, Block *w, Edge *e) { sgraph::GenDiGraphBuilder<Block, Edge>::add(v, w, e); }
	inline CFG::BlockIter blocks(void) const { return cfg->vertices(); }
	int count(void);
	inline Block *at(int index) const { return cfg->at(index); }
private:
	CFG *cfg;
	bool _fix;
};

} // otawa

#endif // OTAWA_CFG_CFG_H
