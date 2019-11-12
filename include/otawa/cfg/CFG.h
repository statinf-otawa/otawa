/*
 *	CFG class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-18, IRIT UPS.
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
#include <elm/data/List.h>
#include <elm/data/Range.h>
#include <otawa/prog/Inst.h>
#include <otawa/prop/PropList.h>
#include <otawa/graph/DiGraph.h>
#include <otawa/prog/Bundle.h>

using namespace elm;

namespace otawa {

class BasicBlock;
class Block;
class CFG;
class CFGMaker;
class SynthBlock;

class Edge: public PropList, public graph::GenEdge<Block, Edge> {
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


class Block: public PropList, public graph::GenVertex<Block, Edge> {
	friend class CFG;
	friend class CFGMaker;
public:
	static const t::uint16
		IS_VIRTUAL	= 0 << 0,
		IS_BASIC	= 1 << 0,
		IS_SYNTH 	= 2 << 0,
		IS_ENTRY 	= 0 << 2,
		IS_EXIT  	= 1 << 2,
		IS_UNKN		= 2 << 2,
		IS_PHONY  	= 3 << 2,
		IS_CALL  	= 0x2 << 0,
		MASK1 	 	= 0x3,
		MASK12	 	= MASK1 | (0x3 << 2);

	inline bool isEnd(void)   	const { return (_type & MASK1)  == IS_VIRTUAL; }
	inline bool isVirtual(void)	const { return (_type & MASK1)  == IS_VIRTUAL; }
	inline bool isEntry(void) 	const { return (_type & MASK12) == (IS_VIRTUAL | IS_ENTRY); }
	inline bool isExit(void)  	const { return (_type & MASK12) == (IS_VIRTUAL | IS_EXIT); }
	inline bool isUnknown(void) const { return (_type & MASK12) == (IS_VIRTUAL | IS_UNKN); }
	inline bool isPhony(void) 	const { return (_type & MASK12) == (IS_VIRTUAL | IS_PHONY); }
	inline bool isSynth(void) 	const { return (_type & MASK1)  == IS_SYNTH; }
	inline bool isCall(void)  	const { return (_type & MASK12) == (IS_SYNTH | IS_CALL); }
	inline bool isBasic(void) 	const { return (_type & MASK1)  == IS_BASIC; }

	inline int id(void) const;
	inline const BasicBlock *toBasic(void) const;
	inline const SynthBlock *toSynth(void) const;
	inline BasicBlock *toBasic(void);
	inline SynthBlock *toSynth(void);
	inline operator BasicBlock *(void) { return toBasic(); }
	inline operator SynthBlock  *(void) { return toSynth(); }
	inline CFG *cfg(void) const { return _cfg; }
	inline Address address(void) const;

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


class PhonyBlock: public Block {
public:
	inline PhonyBlock(): Block(IS_VIRTUAL | IS_PHONY) { }
};


class BasicBlock: public Block {
public:
	BasicBlock(const Array<Inst *>& insts);
	~BasicBlock(void);

	inline Address address(void) const { return first()->address(); }
	int size(void) const;
	inline Address topAddress(void) const { return address() + size(); }
	inline MemArea area(void) const { return MemArea(address(), size()); }
	bool contains(Inst *i);
	inline bool contains(Address addr) { return area().contains(addr); }
	inline bool overlap(const MemArea& area) const { return area.meet(this->area()); }
	inline bool overlap(BasicBlock *bb) const { return area().meet(bb->area()); }

	inline Inst *first(void) const { return _insts[0]; }
	Inst *control(void);
	Inst *last(void);
	int count(void) const;
	inline bool contains(Inst *i) const
		{ return address() <= i->address() && i->address() < topAddress(); }

	class BundleIter;
	class InstIter: public AllocArray<Inst *>::Iter {
		friend class BasicBlock;
	public:
		inline InstIter(void) { }
		inline InstIter(const BasicBlock *bb): AllocArray<Inst *>::Iter(bb->_insts) { }
	private:
		inline InstIter(const AllocArray<Inst *>::Iter& i): AllocArray<Inst *>::Iter(i) { }
	};
	inline InstIter insts(void) const { return InstIter(this); }
	inline InstIter begin(void) const { return _insts.begin(); }
	inline InstIter end(void) const { return _insts.end(); }

	typedef BaseBundle<InstIter> Bundle;

	class BundleIter: public PreIterator<BundleIter, Bundle> {
		friend class BasicBlock;
	public:
		inline BundleIter(void) { }
		inline BundleIter(const BasicBlock *bb): _iter(bb) { }
		inline Bundle item(void) const { return Bundle(_iter); }
		inline bool ended(void) const { return _iter.ended(); }
		inline void next(void)
			{ while(!ended() && _iter->isBundle()) _iter.next(); if(!ended()) _iter.next(); }
		inline bool equals(const BundleIter& i) const { return _iter.equals(i._iter); }
	private:
		inline BundleIter(const InstIter& i): _iter(i) { }
		InstIter _iter;
	};
	inline Range<BundleIter> bundles() const { return Range<BundleIter>(BundleIter(begin()), BundleIter(end())); }

	typedef Vector<Pair<BasicBlock *, Edge *>> basic_preds_t;
	void basicPreds(basic_preds_t& preds);

private:
	AllocArray<Inst *> _insts;
};

class CFG: public PropList, public graph::GenDiGraph<Block, Edge> {
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
	typedef List<SynthBlock *>::Iter CallerIter;

	string label(void);
	string name(void);
	string format(const Address& addr);
	inline int index(void) const { return idx; }
	inline int offset(void) const { return _offset; }
	inline Inst *first(void) const { return fst; }
	inline Address address(void) const { return first()->address(); }
	inline Block *entry(void) const  { return _entry; }
	inline Block *exit(void) const { return _exit; }
	inline Block *unknown(void) const { return _unknown; }
	inline BlockIter blocks(void) const { return vertices(); }
	inline type_t type(void) const { return _type; }
	inline const List<SynthBlock *>& callers(void) const { return _callers; }
	int callCount(void) const;
	void clean(const AbstractIdentifier& id);

private:
	CFG(Inst *first, type_t type = SUBPROG);
	int idx, _offset;
	type_t _type;
	Inst *fst;
	Block *_entry, *_exit, *_unknown;
	List<SynthBlock *> _callers;
};
io::Output& operator<<(io::Output& out, CFG *cfg);
inline io::Output& operator<<(io::Output& out, const CFG::BlockIter& i) { return out << *i; }


// delayed inlines
//inline Block *Edge::target(void) const	{ return sink(); }
inline bool Edge::isForward(void) const { ASSERT(isTaken()); return source()->address() < sink()->address(); }
inline int Block::id(void) const { return index() + _cfg->offset(); }
inline BasicBlock *Block::toBasic(void) { ASSERT(isBasic()); return static_cast<BasicBlock *>(this); }
inline SynthBlock *Block::toSynth(void) { ASSERT(isCall());  return static_cast<SynthBlock  *>(this); }
inline const BasicBlock *Block::toBasic(void) const { ASSERT(isBasic()); return static_cast<const BasicBlock *>(this); }
inline const SynthBlock *Block::toSynth(void) const { ASSERT(isCall());  return static_cast<const SynthBlock  *>(this); }
Output& operator<<(Output& out, Block *b);
inline Address Block::address(void) const
	{ if(isBasic()) return toBasic()->address(); if(isSynth()) return toSynth()->address(); else return Address::null; }
inline Address SynthBlock::address(void) const
	{ if(_callee) return _callee->first()->address(); else return Address::null; }


class CFGMaker: public PropList, public graph::GenDiGraphBuilder<Block, Edge>  {
public:
	CFGMaker(Inst *first, bool fix = false);
	inline Block *entry(void) const { return cfg->entry(); }
	Block *exit(void);
	Block *unknown(void);
	CFG *build(void);
	void add(Block *v);
	void call(SynthBlock *v, CFG *cfg);
	void call(SynthBlock *v, const CFGMaker& cfg);
	inline void add(Block *v, Block *w, Edge *e) { graph::GenDiGraphBuilder<Block, Edge>::add(v, w, e); }
	inline CFG::BlockIter blocks(void) const { return cfg->vertices(); }
	int count(void);
	inline Block *at(int index) const { return cfg->at(index); }
private:
	CFG *cfg;
	bool _fix;
};


// useful accessors
inline Block::EdgeCollection OUT_EDGES(const Block *b) { return b->outEdges(); }
inline Block::EdgeCollection IN_EDGES(const Block *b) { return b->outEdges(); }
inline Block::SuccCollection SUCCS(const Block *b) { return b->succs(); }
inline Block::PredCollection PREDS(const Block *b) { return b->preds(); }

// back access
extern Identifier<bool> BACK_EDGE;
class BackIter: public Block::EdgeIter {
public:
	inline BackIter(void) { }
	inline BackIter(const Block *b): Block::EdgeIter(b->ins()) { step(); }
	inline void next(void) { Block::EdgeIter::next(); step(); }
private:
	void step(void)
		{ while(!ended() && !BACK_EDGE(item())) Block::EdgeIter::next(); }
};

class BACK_EDGES {
public:
	inline BACK_EDGES(const Block *b): _b(b) { }
	inline BackIter begin(void) const { return BackIter(_b); }
	inline BackIter end(void) const { return BackIter(); }
private:
	const Block *_b;
};


// entry access
class EntryIter: public Block::EdgeIter {
public:
	inline EntryIter(void) { }
	inline EntryIter(const Block *b): Block::EdgeIter(b->ins()) { step(); }
	inline void next(void) { Block::EdgeIter::next(); step(); }
private:
	void step(void)
		{ while(!ended() && BACK_EDGE(item())) Block::EdgeIter::next(); }
};

class ENTRY_EDGES {
public:
	inline ENTRY_EDGES(const Block *b): _b(b) { }
	inline EntryIter begin(void) const { return EntryIter(_b); }
	inline EntryIter end(void) const { return EntryIter(); }
private:
	const Block *_b;
};


// exit access
extern Identifier<elm::Vector<Edge*> *> EXIT_LIST;
class EXIT_EDGES {
public:
	inline EXIT_EDGES(const Block *b): _b(b) { ASSERT(b->hasProp(EXIT_LIST)); }
	inline Vector<Edge *>::Iter begin(void) const { return EXIT_LIST(_b)->begin(); }
	inline Vector<Edge *>::Iter end(void) const { return EXIT_LIST(_b)->end(); }
private:
	const Block *_b;
};

} // otawa

#endif // OTAWA_CFG_CFG_H
