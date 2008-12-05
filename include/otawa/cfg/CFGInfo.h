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
#include <elm/genstruct/Vector.h>
#include <elm/inhstruct/DLList.h>
#include <otawa/cfg/BasicBlock.h>

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
	WorkSpace *fw;
	genstruct::Vector<CFG *> _cfgs;
public:
	static Identifier<CFGInfo *>& ID;
	
	// Constructors
	CFGInfo(WorkSpace *fw);
	virtual ~CFGInfo(void);
	
	// Accessors
	BasicBlock *findBB(Inst *inst);
	CFG *findCFG(Inst *inst);
	CFG *findCFG(BasicBlock *bb);
	CFG *findCFG(String label);
	
	// Modifiers
	void add(CFG *cfg);
	void clear(void);
	
	// Iter class
	class Iter: public genstruct::Vector<CFG *>::Iterator {
	public:
		inline Iter(CFGInfo *info)
			: genstruct::Vector<CFG *>::Iterator(info->_cfgs) { }
		inline Iter(const Iter& iter)
			: genstruct::Vector<CFG *>::Iterator(iter) { }
	};
};

} // otawa

#endif	// OTAWA_CFG_CFG_INFO_H
