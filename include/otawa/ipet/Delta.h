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
	elm::Option<int> levels;
	bool explicitNames;

public:
	int max_length;
	double mean_length;
	Delta(const PropList& props = PropList::EMPTY);
	virtual void configure(const PropList& props);
	virtual void processCFG(FrameWork* fw, CFG* cfg);

	static int delta(BBPath &bbp, FrameWork *fw);

	static GenericIdentifier<int> LEVELS;
	static GenericIdentifier<int> DELTA;
	static GenericIdentifier<int> FLUSH_TIME;
};


} } // otawa::ipet

#endif /*OTAWA_IPET_DELTA_H*/
