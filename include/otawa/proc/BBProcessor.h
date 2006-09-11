/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	otawa/proc/BBProcessor.h -- BBProcessor class interface.
 */
#ifndef OTAWA_PROC_BBPROCESSOR_H
#define OTAWA_PROC_BBPROCESSOR_H

#include <otawa/cfg/BasicBlock.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {

class BBProcessor: public CFGProcessor {
protected:
	static const unsigned long IS_RECURSIVE = 0x100;
	void init(const PropList& props);
	virtual void processCFG(FrameWork *fw, CFG *cfg);
	virtual void processBB(FrameWork *fw, CFG *cfd, BasicBlock *bb) = 0;
	
public:
	BBProcessor(const PropList& props = PropList::EMPTY);
	BBProcessor(elm::String name, elm::Version version = elm::Version::ZERO,
		const PropList& props = PropList::EMPTY);
	virtual void configure(const PropList& props);
	inline bool isRecursive(void) const;
};


// Configuration Property
extern GenericIdentifier<bool> RECURSIVE;


// Inlines
inline bool BBProcessor::isRecursive(void) const {
	return flags & IS_RECURSIVE;
}

} // otawa

#endif	// OTAWA_PROC_BBPROCESSOR_H
