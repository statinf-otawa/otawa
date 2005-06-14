/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/util/CFGNormalizer.h -- CFGNormalizer class interface.
 */
#ifndef OTAWA_UTIL_CFG_NORMALIZER_H
#define OTAWA_UTIL_CFG_NORMALIZER_H

#include <otawa/proc/CFGProcessor.h>

namespace otawa {
	
// CFGNormalizer class
class CFGNormalizer: public CFGProcessor {
	bool force;
	bool verbose;
public:
	
	// Identifiers
	static Identifier ID_Force;
	static Identifier ID_Verbose;
	static Identifier ID_Done;
	
	// Constructors
	CFGNormalizer(void);
	
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
	
	// Processor overload
	virtual void configure(PropList& props);	
};

} // otawa

#endif	// OTAWA_UTIL_CFG_NORMALIZER_H
