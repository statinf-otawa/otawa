/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/cfg/CFGInfo.h -- interface of CFGInfo class.
 */
#ifndef OTAWA_CFG_CFG_INFO_H
#define OTAWA_CFG_CFG_INFO_H

#include <elm/utility.h>
#include <elm/datastruct/Vector.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {

// Classes
class BasicBlock;
class CFG;
class Code;
class FrameWork;
class Inst;
	
// CFGInfo class
class CFGInfo: public elm::Lock {
	static id_t ID_Entry;
	FrameWork *fw;
	datastruct::Vector<Code *> _codes;
	datastruct::Vector<CFG *> _cfgs;
	elm::inhstruct::DLList bbs;
	BasicBlock *nextBB(Inst *inst);
	BasicBlock *thisBB(Inst *inst);
	void build(void);
	void buildCFG(Code *code);
	bool built;
public:
	static const id_t ID;
	CFGInfo(FrameWork *fw);
	virtual ~CFGInfo(void);
	void clear(void);
	void addCode(Code *code, File *file = 0);
	void addFile(File *file);
	void addSubProgram(Inst *inst);
	BasicBlock *findBB(Inst *inst);
	CFG *findCFG(Inst *inst);
	CFG *findCFG(BasicBlock *bb);
	CFG *findCFG(String label);
	const elm::Collection<CFG *>& cfgs(void);
	inline const elm::Collection<Code *>& codes(void) const
		{ return _codes; };
};

} // otawa

#endif	// OTAWA_CFG_CFG_INFO_H
