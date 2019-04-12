/*
 *	DotDisplayer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_OSLICE_DOT_DISPLAYER_H
#define OTAWA_OSLICE_DOT_DISPLAYER_H

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
	void displayName(CFG *g, otawa::Block *v, otawa::Block *u = 0);

	bool display_all, display_assembly, source_info;
	elm::io::Output _output;
	int _skip;
	int _show_slicing;
	WorkSpace* _workspace;
};

}}

#endif	// OTAWA_OSLICE_DOT_DISPLAYER_H

