/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	otawa/proc/CFGProcessor.h -- CFGProcessor class interface.
 */
#ifndef OTAWA_PROC_CFGPROCESSOR_H
#define OTAWA_PROC_CFGPROCESSOR_H

#include <otawa/proc/Processor.h>
#include <elm/genstruct/SLList.h>

namespace otawa {
	
// Extern class
class CFG;
class BasicBlock;
	
// Processor class
class CFGProcessor: public Processor {
	elm::CString name;
	CFG *last;
	void init(const PropList& props);
protected:
	virtual void processFrameWork(FrameWork *fw);
	virtual void processCFG(FrameWork *fw, CFG *cfg) = 0;
public:
	CFGProcessor(const PropList& props = PropList::EMPTY);
	CFGProcessor(elm::String name, elm::Version version,
		const PropList& props = PropList::EMPTY);
	virtual void configure(const PropList& props);	
};

// Configuration Properties
extern GenericIdentifier<CFG *> ENTRY_CFG;
extern GenericIdentifier<bool> RECURSIVE;

// Statistics Properties
extern GenericIdentifier<int> PROCESSED_CFG;

} // otawa

#endif // OTAWA_PROC_CFGPROCESSOR_H
