/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	cfg.h -- control flow graph classes interface.
 */
#ifndef OTAWA_CFG_H
#define OTAWA_CFG_H

#include <elm/sequence.h>
#include <elm/data/dllist.h>
#include <elm/obj/vector.h>
#include <otawa/program.h>
#include <otawa/instruction.h>

namespace otawa {

// BaseBlock class
class CFG;
class BasicBlock: public ProgObject, public Lock {
public:
	class Mark;
private:
	Mark *_head;
	Locked<BasicBlock> tkn, ntkn;

public:
	static id_t ID;
	BasicBlock(Inst *head);
	inline BasicBlock *getTaken(void) const { return *tkn; };
	inline BasicBlock *getNotTaken(void) const { return *ntkn; };
	inline void setTaken(BasicBlock *bb) { tkn = bb; };
	inline void setNotTaken(BasicBlock *bb) { ntkn = bb; };
	inline Mark *head(void) const { return _head; };

	// Mark class
	class Mark: public PseudoInst {
		Locked<BasicBlock> _bb;
	public:
		inline Mark(BasicBlock *bb): PseudoInst(ID), _bb(bb) { };
		inline ~Mark(void) { _bb->_head = 0; };
		inline BasicBlock *bb(void) const  { return *_bb; };
	};	
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
	obj::Vector<Code *> _codes;
	obj::Vector<CFG *> _cfgs;
	BasicBlock *nextBB(Inst *inst);
	BasicBlock *thisBB(Inst *inst);
public:
	static id_t ID_CFG;
	void clear(void);
	void addCode(Code *code);
	BasicBlock *findBB(Code *code, Inst *inst);
	CFG *findCFG(Code *code, Inst *inst);
	inline const Collection<CFG *>& cfgs(void) const { return _cfgs; };
	inline const Collection<Code *>& codes(void) const { return _codes; };
};

}	// namespace otawa

#endif	// OTAWA_CFG_H

