/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/BasicBlock.h -- interface of BasicBlock class.
 */
#ifndef OTAWA_CFG_BASIC_BLOCK_H
#define OTAWA_CFG_BASIC_BLOCK_H

#include <assert.h>
#include <otawa/prop/Identifier.h>
#include <elm/genstruct/SLList.h>
#include <elm/inhstruct/DLList.h>
#include <elm/utility.h>
#include <elm/Iterator.h>
#include <otawa/program.h>
#include <otawa/instruction.h>

namespace otawa {

// Extern class
class FrameWork;
class Edge;
class CFG;
extern Identifier<int> INDEX;

// BasicBlock class
class BasicBlock: public elm::inhstruct::DLNode, public ProgObject {
	friend class CFGBuilder;
	friend class CFGInfo;
	friend class CFG;
	friend class VirtualCFG;
public:
	class Mark;
protected:
	static const unsigned long FLAG_Call = 0x01;
	static const unsigned long FLAG_Unknown = 0x02;	
	static const unsigned long FLAG_Return = 0x04;
	static const unsigned long FLAG_Entry = 0x08;
	static const unsigned long FLAG_Exit = 0x10;
	static const unsigned long FLAG_Virtual = 0x20;
	unsigned long flags;
	elm::genstruct::SLList<Edge *> ins, outs;
	Mark *_head;
	CFG *_cfg;

	// EdgeIterator class
	class EdgeIterator: public elm::genstruct::SLList<Edge *>::Iterator  {
	public:
		inline EdgeIterator(elm::genstruct::SLList<Edge *>& edges);
	};

public:
	// InstIterator class
	class InstIterator: public PreIterator<InstIterator, Inst *> {
		otawa::Inst *inst;
	public:
		inline InstIterator(BasicBlock *bb);
		inline bool ended(void) const;
		inline Inst *item(void) const;
		inline void next(void);
	};

protected:
	static BasicBlock& null_bb;
	virtual ~BasicBlock(void);
	void setTaken(BasicBlock *bb);
	void setNotTaken(BasicBlock *bb);

public:
	static Identifier<BasicBlock *> ID;

	// Mark class
	class Mark: public PseudoInst {
		friend class BasicBlock;
		friend class NullBasicBlock;
		friend class CodeBasicBlock;
		BasicBlock *_bb;
		inline ~Mark(void) { remove(); _bb->_head = 0; };
	public:
		inline Mark(BasicBlock *bb): PseudoInst(&ID), _bb(bb) { };
		inline BasicBlock *bb(void) const  { return _bb; };
	};	

	// Constructors
	inline BasicBlock(void): _head(0), flags(0), _cfg(0) { };
	static BasicBlock *findBBAt(FrameWork *fw, address_t addr);
	
	// Generic accessors
	virtual inline IteratorInst<Inst *> *visit(void);
	inline operator IteratorInst<Inst *> *(void) { return visit(); };
	inline bool isCall(void) const { return (flags & FLAG_Call) != 0; };
	inline bool isReturn(void) const { return (flags & FLAG_Return) != 0; };
	inline bool isTargetUnknown(void) const
		{ return (flags & FLAG_Unknown) != 0; };
	inline bool isEntry(void) const { return flags & FLAG_Entry; };
	inline bool isExit(void) const { return flags & FLAG_Exit; };
	inline bool isEnd(void) const { return flags & (FLAG_Entry | FLAG_Exit); };
	inline Mark *head(void) const { return _head; };
	inline address_t address(void) const { return _head->address(); };
	virtual int countInstructions(void) const;
	size_t size(void) const;
	inline bool isVirtual(void) const { return flags & FLAG_Virtual; };
	inline unsigned long getFlags(void) const { return flags; };
	inline int number(void) { return INDEX(this); };
	inline CFG *cfg(void) { return _cfg; }
	
	// Edge management
	inline void addInEdge(Edge *edge) { ins.addFirst(edge); };
	void addOutEdge(Edge *edge) { outs.addFirst(edge); };
	void removeInEdge(Edge *edge) { ins.remove(edge); };
	void removeOutEdge(Edge *edge) { outs.remove(edge); };
	
	// Edge iterators
	class InIterator: public EdgeIterator {
	public:
		inline InIterator(BasicBlock *bb);
	};
	class OutIterator: public EdgeIterator {
	public:
		inline OutIterator(BasicBlock *bb);
	};
	
	// Deprecated
	BasicBlock *getTaken(void);
	BasicBlock *getNotTaken(void);
	inline size_t getBlockSize(void) const { return size(); };
	IteratorInst<Edge *> *inEdges(void);
	IteratorInst<Edge *> *outEdges(void);	
};


// BasicBlock class
class CodeBasicBlock: public BasicBlock {
	friend class CFGInfo;
	~CodeBasicBlock(void);
public:
	CodeBasicBlock(Inst *head);
};


// EndBasicBlock class
class EndBasicBlock: public BasicBlock {
public:
	inline EndBasicBlock(unsigned long _flags = 0) {
		flags = _flags;
		_head = null_bb.head();
	};
};


// BasicBlock::InstIterator inlines
inline BasicBlock::InstIterator::InstIterator(BasicBlock *bb)
: inst(bb->head()->next()) {
	assert(bb);
}

inline bool BasicBlock::InstIterator::ended(void) const {
	PseudoInst *pseudo;
	return inst->atEnd()
		|| ((pseudo = inst->toPseudo()) && pseudo->id() == &ID);
}

inline Inst *CodeBasicBlock::InstIterator::item(void) const {
	return inst;
}

inline void CodeBasicBlock::InstIterator::next(void) {
	inst = inst->next();
}


// BasicBlock::EdgeIterator inlines
inline BasicBlock::EdgeIterator::EdgeIterator(elm::genstruct::SLList<Edge *>& edges)
: elm::genstruct::SLList<Edge *>::Iterator(edges) {
};


// BasicBlock::InIterator inlines
inline BasicBlock::InIterator::InIterator(BasicBlock *bb)
: EdgeIterator(bb->ins) {
	assert(bb);
};


// BasicBlock::OutIterator inlines
inline BasicBlock::OutIterator::OutIterator(BasicBlock *bb)
: EdgeIterator(bb->outs) {
	assert(bb);
};

// BasicBlock inlines
inline IteratorInst<Inst *> *BasicBlock::visit(void) {
	InstIterator iter(this);
	return new IteratorObject<InstIterator, Inst *>(iter);
}

} // otawa

#endif // OTAWA_CFG_BASIC_BLOCK_H
