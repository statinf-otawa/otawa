/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/ipet/BBTimeSimulator.h -- BBTimeSimulator class interface.
 */
#ifndef OTAWA_IPET_BBTIMESIMULATOR_H
#define OTAWA_IPET_BBTIMESIMULATOR_H

#include <otawa/cfg.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa { namespace ipet {

// BBTimeSimulator class
class BBTimeSimulator : public BBProcessor {
public:
	BBTimeSimulator(void);
	void processBB(WorkSpace *fw, CFG *cfg, BasicBlock *bb);
};

} }

#endif /* OTAWA_IPET_BBTIMESIMULATOR_H */
