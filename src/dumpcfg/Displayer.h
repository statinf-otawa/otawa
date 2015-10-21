/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/Displayer.h -- Displayer class interface.
 */
#ifndef OTAWA_DUMPCFG_DISPLAYER_H
#define OTAWA_DUMPCFG_DISPLAYER_H

#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg.h>

using namespace otawa;

// Displayer class
class Displayer: public Processor {
	friend class DumpCFG;
public:
	static Identifier<bool> DISASSEMBLE;
	static Identifier<bool> SOURCE;
	static Identifier<bool> ALL;
	Displayer(cstring name, const Version version);
protected:
	virtual void configure(const PropList& props);
	bool display_assembly;
	bool source_info;
	bool display_all;
};

#endif	// OTAWA_DUMPCFG_DISPLAYER_H

