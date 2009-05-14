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
#include <elm/genstruct/FragTable.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/util/MemBlockMap.h>

namespace otawa {

using namespace elm;

// Classes
class BasicBlock;
class CFG;
class CodeItem;
class WorkSpace;
class Inst;

// CFGInfo class
class CFGInfo: public elm::Lock {
public:
	static Identifier<CFGInfo *>& ID;

	// Constructors
	CFGInfo(WorkSpace *fw);
	virtual ~CFGInfo(void);

	// Accessors
	//BasicBlock *findBB(Inst *inst);
	CFG *findCFG(Inst *inst);
	CFG *findCFG(const BasicBlock *bb);
	CFG *findCFG(String label);

	// Modifiers
	void add(CFG *cfg);
	void add(BasicBlock *bb);
	void clear(void);

	// Iter class
	class Iter: public genstruct::FragTable<CFG *>::Iterator {
	public:
		inline Iter(CFGInfo *info)
			: genstruct::FragTable<CFG *>::Iterator(info->_cfgs) { }
		inline Iter(const Iter& iter)
			: genstruct::FragTable<CFG *>::Iterator(iter) { }
	};

private:
	WorkSpace *fw;
	genstruct::FragTable<CFG *> _cfgs;
	genstruct::FragTable<BasicBlock *> bbs;
	MemBlockMap<BasicBlock> map;
};

} // otawa

#endif	// OTAWA_CFG_CFG_INFO_H
