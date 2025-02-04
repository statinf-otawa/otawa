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
	static Identifier<bool> VIEW;
	Displayer(cstring name, const Version version);
protected:
	virtual void configure(const PropList& props);
	bool display_assembly;
	bool source_info;
	bool display_all;
	bool perform_view;
	bool display_sem;
	bool display_kind;
	bool display_regs;
	bool display_target;
	bool display_bytes;
	sys::Path out;
};

#endif	// OTAWA_DUMPCFG_DISPLAYER_H

