/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/SimpleDisplayer.h -- SimpleDisplayer class interface.
 */
#ifndef OTAWA_DUMPCFG_SIMPLE_DISPLAYER_H
#define OTAWA_DUMPCFG_SIMPLE_DISPLAYER_H

#include "Displayer.h"

// SimpleDisplayer class
class SimpleDisplayer: public Displayer {
public:
	SimpleDisplayer(void);
protected:
	virtual void processWorkSpace(WorkSpace *ws);
private:
	int offset(CFG *cfg);
	static Identifier<int> OFFSET;
	int cfg_cnt;
};

#endif	// OTAWA_DUMPCFG_SIMPLE_DISPLAYER_H

