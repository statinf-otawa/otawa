/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cfg/CFGBuilder.h -- CFGBuilder class interface.
 */

#ifndef OTAWA_CFG_CFG_BUILDER_H
#define OTAWA_CFG_CFG_BUILDER_H

#include <otawa/proc/Processor.h>
#include <otawa/cfg/CFGInfo.h>

namespace otawa {

// CFGBuilder class
class CFGBuilder: public Processor {
	static Identifier ID_Entry;
	datastruct::Vector<CFG *> _cfgs;
	
	BasicBlock *nextBB(Inst *inst);
	BasicBlock *thisBB(Inst *inst);
	void addSubProgram(Inst *inst);
	void buildCFG(CodeItem *code);
	void addFile(File *file);
	void buildAll(FrameWork *fw);
public:
	CFGBuilder(const PropList& props = PropList::EMPTY);
	
	// Processor overload
	virtual void processFrameWork(FrameWork *fw);
};

} // otawa

#endif // OTAWA_CFG_CFG_BUILDER_H

