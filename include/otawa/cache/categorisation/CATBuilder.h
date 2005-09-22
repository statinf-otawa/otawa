/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 * 
 * otawa/prog/CATBuilder.h -- CATBuilder class interface.
 */
#ifndef OTAWA_IPET_CACHE_CATBUILDER_H
#define OTAWA_IPET_CACHE_CATBUILDER_H

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>

namespace otawa {
class LBlockSet;
class CFG;
class LBlock;
class CATBuilder: public CFGProcessor {
	static Identifier ID_In;
	static Identifier ID_Out;	
	void processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset);
public:

	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg );
};

}	// otawa

#endif // OTAWA_IPET_CATBUILDER_H
