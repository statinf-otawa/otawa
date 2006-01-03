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
	
/* Extern class */
class LBlockSet;
class CFG;
class LBlock;

/* CATBuilder class */
class CATBuilder: public CFGProcessor {
	static Identifier ID_In;
	static Identifier ID_Out;	
	bool _explicit;
	void processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset);
	void initialize(const PropList& props);
public:
	static Identifier ID_NonConflict;
	static Identifier ID_Node;
	static Identifier ID_HitVar;
	static Identifier ID_MissVar;
	static Identifier ID_BBVar;

	CATBuilder(const PropList& props = PropList::EMPTY);

	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg );
	virtual void configure(const PropList& props);
};

}	// otawa

#endif // OTAWA_IPET_CATBUILDER_H
