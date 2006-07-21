/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/ipet/Delta.h -- Delta class interface.
 */
 
#ifndef OTAWA_IPET_DELTA_H
#define OTAWA_IPET_DELTA_H

#include <otawa/ipet/BBSequence.h>
#include <otawa/ipet/TreePath.h>
#include <otawa/cfg.h>
#include <otawa/properties.h>
#include <otawa/proc/CFGProcessor.h>


namespace otawa { namespace ipet {
	
class BBSequence;
class Delta;

class Delta: public CFGProcessor {
	friend class BBSequence;
	int nlevels;
	bool explicitNames;
	FrameWork *framework;

public:
	Delta(const PropList& props = PropList::EMPTY);
	virtual void processCFG(FrameWork* fw, CFG* cfg);

	BBSequence* getBBS(BasicBlock *start);
	BBSequence* getBBS(elm::genstruct::Vector<BasicBlock*> *path);
	
	//static GenericIdentifier<int> ID_Levels;
	//static GenericIdentifier<TreePath<BasicBlock*,BBSequence*>*> ID_Tree;
};


} }

#endif /*OTAWA_IPET_DELTA_H*/
