/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/dumpcfg/DotDisplayer.h -- DotDisplayer class interface.
 */
#ifndef OTAWA_DUMPCFG_DOT_DISPLAYER_H
#define OTAWA_DUMPCFG_DOT_DISPLAYER_H

#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg.h>
using namespace otawa;

namespace otawa { namespace oslice {


// SimpleDisplayer class
class DotDisplayer {
public:
	DotDisplayer(WorkSpace* ws, String path, int show_slicing);
	void display(const CFGCollection& coll);
private:
	void displayLabel(Block *bb);
	void displayName(CFG *g, otawa::Block *v);

	bool display_all, display_assembly, source_info;
	elm::io::Output _output;
	int _skip;
	int _show_slicing;
	WorkSpace* _workspace;
};

}}

#endif	// OTAWA_DUMPCFG_DOT_DISPLAYER_H

