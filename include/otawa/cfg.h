/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	cfg.h -- control flow graph classes interface.
 */
#ifndef OTAWA_CFG_H
#define OTAWA_CFG_H

#include <elm/inhstruct/DLList.h>
#include <elm/datastruct/Vector.h>
#include <otawa/program.h>
#include <otawa/instruction.h>

namespace otawa {

	
// Defined classes
class BasicBlock;
class CFG;
class CFGInfo;

	
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
	Locked<BasicBlock> tkn, ntkn;
	inline void setTaken(BasicBlock *bb) { tkn = bb; };
	inline void setNotTaken(BasicBlock *bb) { ntkn = bb; };
	inline unsigned long getFlags(void) const { return flags; };
	inline void setFlags(unsigned long _flags) { flags = _flags; };

public:
	static id_t ID;
	BasicBlock(Inst *head);

	// Mark class
	class Mark: public PseudoInst {
		Locked<BasicBlock> _bb;
	public:
		inline Mark(BasicBlock *bb): PseudoInst(ID), _bb(bb) {
		};
		inline ~Mark(void) { _bb->_head = 0; };
		inline BasicBlock *bb(void) const  { return *_bb; };
	};	

	// Methods
	inline bool isCall(void) const { return (flags & FLAG_Call) != 0; };
	inline bool isReturn(void) const { return (flags & FLAG_Return) != 0; };
	inline bool isTargetUnknown(void) const { return (flags & FLAG_Unknown) != 0; };
	inline BasicBlock *getTaken(void) const { return *tkn; };
	inline BasicBlock *getNotTaken(void) const { return *ntkn; };
	inline Mark *head(void) const { return _head; };
	inline address_t address(void) const { return _head->address(); };
};


// CFG class
class CFG: public ProgObject {
	Code *_code;
	Locked<BasicBlock> ent;
public:
	static id_t ID;
	CFG(Code *code, BasicBlock *entry);
	inline BasicBlock *entry(void) const { return *ent; };
	inline Code *code(void) const { return _code; };
};


// CFGInfo class
class CFGInfo {
	static id_t ID_Entry;
	datastruct::Vector<Code *> _codes;
	datastruct::Vector<CFG *> _cfgs;
	BasicBlock *nextBB(Inst *inst);
	BasicBlock *thisBB(Inst *inst);
public:
	static id_t ID;
	void clear(void);
	void addCode(Code *code);
	BasicBlock *findBB(Code *code, Inst *inst);
	CFG *findCFG(Code *code, Inst *inst);
	inline const datastruct::Collection<CFG *>& cfgs(void) const { return _cfgs; };
	inline const datastruct::Collection<Code *>& codes(void) const { return _codes; };
};

}	// namespace otawa

#endif	// OTAWA_CFG_H
