/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	CATConstraintsBuilder class interface
 */
#ifndef OTAWA_CACHE_CATCONSTRAINTBUILDER_H
#define OTAWA_CACHE_CATCONSTRAINTBUILDER_H


#include <assert.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// Extern classes
class ContextTree;
class LBlockSet;

namespace ipet {
	
// CATConstraintBuilder class
class CATConstraintBuilder: public Processor {
	bool _explicit;
	void processLBlockSet(FrameWork *fw, LBlockSet *lbset);
	void buildLBLOCKSET(LBlockSet *lcache, ContextTree *root);

public:
	CATConstraintBuilder(void);
		
	// CFGProcessor overload
	virtual void processFrameWork(FrameWork *fw);
	virtual void configure(const PropList& props);
};

// Features
extern Feature<CATConstraintBuilder> ICACHE_SUPPORT_FEATURE; 

} }	// otawa::ipet


#endif //OTAWA_TEST_CATCONSTRAINTBUILDER_H_
