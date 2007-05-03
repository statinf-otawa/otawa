/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CFGNormalizer class interface
 */
#ifndef OTAWA_UTIL_CFG_NORMALIZER_H
#define OTAWA_UTIL_CFG_NORMALIZER_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {
	
// CFGNormalizer class
class CFGNormalizer: public CFGProcessor {
	bool force;
	bool verbose;

protected:
	virtual void processCFG(WorkSpace *fw, CFG *cfg);

public:
	
	// Configuration
	static Identifier<bool> FORCE;
	static Identifier<bool> VERBOSE;

	// Methods
	CFGNormalizer(void);
	virtual void configure(const PropList& props = PropList::EMPTY);	
};

// Feature
extern Feature<CFGNormalizer> NORMALIZED_CFGS_FEATURE;

} // otawa

#endif	// OTAWA_UTIL_CFG_NORMALIZER_H
