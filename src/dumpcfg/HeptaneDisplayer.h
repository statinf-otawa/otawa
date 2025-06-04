/*
 *	$Id$
 *	Copyright (c) 2025, StatInf.
 *
 *	src/dumpcfg/HeptaneDisplayer.h -- HeptaneDisplayer class interface.
 */
#ifndef OTAWA_DUMPCFG_HEPTANE_DISPLAYER_H
#define OTAWA_DUMPCFG_HEPTANE_DISPLAYER_H

#include "Displayer.h"

// HeptaneDisplayer class
class HeptaneDisplayer: public Displayer {
public:
	HeptaneDisplayer(void);
protected:
	virtual void processWorkSpace(WorkSpace *ws);
private:
	int offset(CFG *cfg);
	static Identifier<int> OFFSET;
	int cfg_cnt;
};

#endif	// OTAWA_DUMPCFG_HEPTANE_DISPLAYER_H

