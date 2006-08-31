/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	BBTimeSimulator.h -- BBTimeSimulator class interface.
 */
#ifndef IPET_BBTIMESIMULATOR_H
#define IPET_BBTIMESIMULATOR_H

#include "SequenceBasicBlock.h"
#include <otawa/cfg.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa { namespace ipet {

class BBTimeSimulator : public BBProcessor{
	void processSequenceBB(FrameWork *fw, SequenceBasicBlock *bb);
	void processNormalBB(FrameWork *fw, BasicBlock *bb);
public:
	BBTimeSimulator(const PropList& props = PropList::EMPTY);
	void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
};

} }

#endif /*IPET_BBTIMESIMULATOR_H*/
