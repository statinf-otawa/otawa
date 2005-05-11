/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/BasicBlock.h -- interface of BasicBlock class.
 */
#ifndef OTAWA_CFG_BASIC_BLOCK_H
#define OTAWA_CFG_BASIC_BLOCK_H

#include <elm/genstruct/SLList.h>
#include <elm/inhstruct/DLList.h>
#include <elm/utility.h>
#include <elm/Iterator.h>
#include <otawa/program.h>
#include <otawa/instruction.h>
//#include <otawa/cfg/Edge.h>

namespace otawa {

// Extern class
class FrameWork;
class Edge;

// BasicBlock class
class BasicBlock: public elm::inhstruct::DLNode, public ProgObject {
	friend class CFGInfo;
	friend class CFG;
public:
	class Mark;
protected:
	static const unsigned long FLAG_Call = 0x01;
	static const unsigned long FLAG_Unknown = 0x02;	
	static const unsigned long FLAG_Return = 0x04;
	static const unsigned long FLAG_Entry = 0x08;
	static const unsigned long FLAG_Exit = 0x10;
	unsigned long flags;
	elm::genstruct::SLList<Edge *> ins, outs;
	Mark *_head;

	// EdgeIterator
	class EdgeIterator: public IteratorInst<Edge *> {
		elm::genstruct::SLList<Edge *>::Iterator iter;
	public:
		inline EdgeIterator(elm::genstruct::SLList<Edge *> &list)
		: iter(list) { };
		virtual bool ended(void) const;
		virtual Edge *item(void) const;
		virtual void next(void);
	};

	// Iterator
	class Iterator: public IteratorInst<Inst *> {
		otawa::Inst *inst;
	public:
		inline Iterator(BasicBlock *bb): inst(bb->head()->next()) { };
		virtual bool ended(void) const;
		virtual Inst *item(void) const;
		virtual void next(void);
	};
	
	static BasicBlock& null_bb;
	virtual ~BasicBlock(void);
	void setTaken(BasicBlock *bb);
	void setNotTaken(BasicBlock *bb);
	inline unsigned long getFlags(void) const { return flags; };
	inline void setFlags(unsigned long _flags) { flags = _flags; };
public:
	static id_t ID;

	// Mark class
	class Mark: public PseudoInst {
		friend class BasicBlock;
		friend class NullBasicBlock;
		friend class CodeBasicBlock;
		BasicBlock *_bb;
		inline ~Mark(void) { remove(); _bb->_head = 0; };
	public:
		inline Mark(BasicBlock *bb): PseudoInst(ID), _bb(bb) { };
		inline BasicBlock *bb(void) const  { return _bb; };
	};	

	// Constructors
	inline BasicBlock(void): _head(0), flags(0) { };
	static BasicBlock *findBBAt(FrameWork *fw, address_t addr);
	
	// Generic accessors
	inline IteratorInst<Inst *> *visit(void) { return new Iterator(this); };
	inline operator IteratorInst<Inst *> *(void) { return visit(); };
	inline bool isCall(void) const { return (flags & FLAG_Call) != 0; };
	inline bool isReturn(void) const { return (flags & FLAG_Return) != 0; };
	inline bool isTargetUnknown(void) const
		{ return (flags & FLAG_Unknown) != 0; };
	inline bool isEntry(void) const { return flags & FLAG_Entry; };
	inline bool isExit(void) const { return flags & FLAG_Exit; };
	inline Mark *head(void) const { return _head; };
	inline address_t address(void) const { return _head->address(); };
	int countInstructions(void) const;
	size_t size(void) const;
	
	// Edge management
	inline void addInEdge(Edge *edge) { ins.addFirst(edge); };
	void addOutEdge(Edge *edge) { outs.addFirst(edge); };
	void removeInEdge(Edge *edge) { ins.remove(edge); };
	void removeOutEdge(Edge *edge) { outs.remove(edge); };
	inline IteratorInst<Edge *> *inEdges(void) { return new EdgeIterator(ins); };
	inline IteratorInst<Edge *> *outEdges(void) { return new EdgeIterator(outs); };
	
	// Deprecated
	BasicBlock *getTaken(void);
	BasicBlock *getNotTaken(void);
	inline size_t getBlockSize(void) const { return size(); };
};


// BasicBlock class
class CodeBasicBlock: public BasicBlock {
	friend class CFGInfo;
	~CodeBasicBlock(void);
public:
	CodeBasicBlock(Inst *head);
};


// VirtualBasicBlock class
class VirtualBasicBlock: public BasicBlock {
public:
	inline VirtualBasicBlock(unsigned long _flags = 0) {
		flags = _flags;
		_head = null_bb.head();
	};
};

} // otawa

#endif // OTAWA_CFG_BASIC_BLOCK_H
