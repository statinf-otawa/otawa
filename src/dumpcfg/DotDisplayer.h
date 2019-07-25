/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/DotDisplayer.h -- DotDisplayer class interface.
 */
#ifndef OTAWA_DUMPCFG_DOT_DISPLAYER_H
#define OTAWA_DUMPCFG_DOT_DISPLAYER_H

#include "Displayer.h"

// SimpleDisplayer class
class DotDisplayer: public Displayer {
public:
	DotDisplayer(void);
protected:
	virtual void processWorkSpace(WorkSpace *ws);
private:
	void displayLabel(Block *bb);
	void displayName(CFG *g, otawa::Block *v);
};

#endif	// OTAWA_DUMPCFG_DOT_DISPLAYER_H

