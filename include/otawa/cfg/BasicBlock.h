/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/BasicBlock.h -- interface of BasicBlock class.
 */
#ifndef OTAWA_CFG_BASIC_BLOCK_H
#define OTAWA_CFG_BASIC_BLOCK_H

#include <elm/utility.h>
#include <elm/Iterator.h>
#include <otawa/program.h>
#include <otawa/instruction.h>

namespace otawa {

// BaseBlock class
class BasicBlock: public ProgObject, public Lock {
public:
	class Mark;
private:
	friend class CFGInfo;
	static const unsigned long FLAG_Call = 0x01;
	static const unsigned long FLAG_Unknown = 0x02;	
	static const unsigned long FLAG_Return = 0x04;
	Mark *_head;
	unsigned long flags;
	elm::AutoPtr<BasicBlock> tkn, ntkn;
	
	// Private methods
	inline void setTaken(elm::AutoPtr<BasicBlock> bb) { tkn = bb; };
	inline void setNotTaken(elm::AutoPtr<BasicBlock> bb) { ntkn = bb; };
	inline unsigned long getFlags(void) const { return flags; };
	inline void setFlags(unsigned long _flags) { flags = _flags; };
	void release(void);

	// Iterator
	class Iterator: public IteratorInst<Inst *> {
		otawa::Inst *inst;
	public:
		inline Iterator(BasicBlock *bb): inst(bb->head()->next()) { };
		virtual bool ended(void) const;
		virtual Inst *item(void) const;
		virtual void next(void);
	};

public:
	static id_t ID;
	BasicBlock(Inst *head);

	// Mark class
	class Mark: public PseudoInst {
		AutoPtr<BasicBlock> _bb;
	public:
		inline Mark(AutoPtr<BasicBlock> bb): PseudoInst(ID), _bb(bb) {
		};
		inline ~Mark(void) { _bb->_head = 0; };
		inline AutoPtr<BasicBlock> bb(void) const  { return _bb; };
	};	

	// Methods
	inline IteratorInst<Inst *> *visit(void) { return new Iterator(this); };
	inline operator IteratorInst<Inst *> *(void) { return visit(); };
	inline bool isCall(void) const { return (flags & FLAG_Call) != 0; };
	inline bool isReturn(void) const { return (flags & FLAG_Return) != 0; };
	inline bool isTargetUnknown(void) const
		{ return (flags & FLAG_Unknown) != 0; };
	inline elm::AutoPtr<BasicBlock> getTaken(void) const { return tkn; };
	inline elm::AutoPtr<BasicBlock> getNotTaken(void) const { return ntkn; };
	inline Mark *head(void) const { return _head; };
	inline address_t address(void) const { return _head->address(); };
	size_t getBlockSize(void) const;
};

} // otawa

#endif // OTAWA_CFG_BASIC_BLOCK_H
