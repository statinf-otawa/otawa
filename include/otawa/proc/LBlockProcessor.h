/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	otawa/proc/LBlockProcessor.h -- LBlockProcessor class interface.
 */
#ifndef OTAWA_PROC_LBLOCKPROCESSOR_H
#define OTAWA_PROC_LBLOCKPROCESSOR_H

#include <otawa/proc/Processor.h>
#include <elm/genstruct/SLList.h>

namespace otawa {
	
// Extern class
class LBlock;
class BasicBlock;
	
// Processor class
class LBlockProcessor: public Processor {
	elm::CString name;
	void init(const PropList& props);
protected:
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void processLBlock(WorkSpace *fw, LBlock *cfg) = 0;
public:
	LBlockProcessor(void);
	LBlockProcessor(elm::String name, elm::Version version);
	virtual void configure(const PropList& props);	
};

} // otawa

#endif // OTAWA_PROC_LBLOCKPROCESSOR_H
