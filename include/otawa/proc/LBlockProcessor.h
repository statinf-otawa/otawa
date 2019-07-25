/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	otawa/proc/LBlockProcessor.h -- LBlockProcessor class interface.
 */
#ifndef OTAWA_PROC_LBLOCKPROCESSOR_H
#define OTAWA_PROC_LBLOCKPROCESSOR_H

#include <otawa/proc/Processor.h>
#include <otawa/cache/features.h>

namespace otawa { namespace cache {
	
// Extern class
class LBlock;
class BasicBlock;
	
// Processor class
class LBlockProcessor: public Processor {
public:
	LBlockProcessor(AbstractRegistration &registration = reg);
	static p::declare reg;

protected:
	inline const hard::Cache *cache(void) const { return _cache; }
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void processLBlockSet(WorkSpace *ws, LBlockSet *set);
	virtual void processLBlock(WorkSpace *ws, LBlockSet *set, LBlock *lblock);

private:
	const hard::Cache *_cache;
};

} }	// otawa::cache

#endif // OTAWA_PROC_LBLOCKPROCESSOR_H
