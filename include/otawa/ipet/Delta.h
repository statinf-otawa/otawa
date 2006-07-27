/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/ipet/Delta.h -- Delta class interface.
 */
 
#ifndef OTAWA_IPET_DELTA_H
#define OTAWA_IPET_DELTA_H
#include <otawa/ipet/BBPath.h>
#include <otawa/proc/CFGProcessor.h>


namespace otawa { namespace ipet {
	
class BBPath;
class Delta;

class Delta: public CFGProcessor {
	int nlevels;
	bool explicitNames;

public:
	Delta(const PropList& props = PropList::EMPTY);
	virtual void configure(const PropList& props);
	virtual void processCFG(FrameWork* fw, CFG* cfg);

	static int delta(BBPath &bbp, FrameWork *fw);

	static GenericIdentifier<int> ID_Levels;
	static GenericIdentifier<int> ID_Delta;
};


} } // otawa::ipet

#endif /*OTAWA_IPET_DELTA_H*/
