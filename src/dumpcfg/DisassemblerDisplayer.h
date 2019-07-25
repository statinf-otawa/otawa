/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/DisassemblerDisplayer.h -- DisassemblerDisplayer class interface.
 */
#ifndef OTAWA_DUMPCFG_DISASSEMBLER_DISPLAYER_H
#define OTAWA_DUMPCFG_DISASSEMBLER_DISPLAYER_H

#include "Displayer.h"

// DisassemblerDisplayer class
class DisassemblerDisplayer: public Displayer {
public:
	DisassemblerDisplayer(void);
protected:
	virtual void processWorkSpace(WorkSpace *ws);
};

#endif	// OTAWA_DUMPCFG_DISASSEMBLER_DISPLAYER_H

