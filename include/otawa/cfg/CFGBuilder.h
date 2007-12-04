/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cfg/CFGBuilder.h -- CFGBuilder class interface.
 */

#ifndef OTAWA_CFG_CFG_BUILDER_H
#define OTAWA_CFG_CFG_BUILDER_H

#include <elm/genstruct/Vector.h>
#include <otawa/proc/Feature.h>
#include <otawa/proc/Processor.h>
#include <otawa/cfg/CFGInfo.h>

namespace otawa {

// CFGBuilder class
class CFGBuilder: public Processor {
	genstruct::Vector<CFG *> _cfgs;
	bool verbose;
	
	BasicBlock *nextBB(Inst *inst);
	BasicBlock *thisBB(Inst *inst);
	void addSubProgram(Inst *inst);
	void buildCFG(WorkSpace *ws, Segment *seg);
	void addFile(WorkSpace *ws, File *file);
	void buildAll(WorkSpace *fw);
public:
	CFGBuilder(void);
	
	// Processor overload
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void configure(const PropList& props = PropList::EMPTY);
};

// Features
extern Feature<CFGBuilder> CFG_INFO_FEATURE;

} // otawa

#endif // OTAWA_CFG_CFG_BUILDER_H

