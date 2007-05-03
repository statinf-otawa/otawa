/*
 *	$Id$
 *	Copyright (c) 2003-07, IRIT UPS.
 *
 *	otawa/cfg/CFGInfo.h -- interface of CFGInfo class.
 */
#ifndef OTAWA_CFG_CFG_INFO_H
#define OTAWA_CFG_CFG_INFO_H

#include <elm/utility.h>
#include <elm/util/AutoPtr.h>
#include <elm/datastruct/Vector.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {

// Classes
class BasicBlock;
class CFG;
class CodeItem;
class WorkSpace;
class Inst;
	
// CFGInfo class
class CFGInfo: public elm::Lock {
	WorkSpace *fw;
	datastruct::Vector<CFG *> _cfgs;
public:
	static Identifier<CFGInfo *> ID;
	CFGInfo(WorkSpace *fw, elm::Collection<CFG *>& cfgs);
	virtual ~CFGInfo(void);
	void clear(void);
	BasicBlock *findBB(Inst *inst);
	CFG *findCFG(Inst *inst);
	CFG *findCFG(BasicBlock *bb);
	CFG *findCFG(String label);
	elm::Collection<CFG *>& cfgs(void);
};

} // otawa

#endif	// OTAWA_CFG_CFG_INFO_H
