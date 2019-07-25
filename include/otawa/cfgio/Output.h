/*
 *	CFGOutput class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#ifndef OTAWA_CFGIO_OUTPUT_H_
#define OTAWA_CFGIO_OUTPUT_H_

#include <elm/avl/Set.h>
#include <elm/xom.h>
#include <otawa/cfg.h>
#include <otawa/cfgio/features.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa { namespace cfgio {

using namespace elm;

class Output: public BBProcessor {
public:
	static p::declare reg;
	Output(void);
protected:
	virtual void configure(const PropList& props);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b);
	string id(CFG *cfg);
	string id(Block *bb);
	void processProps(xom::Element *parent, const PropList& props);
	xom::Element *root, *cfg_node;
	int last_bb;
	avl::Set<const AbstractIdentifier *> ids;
	Path path;
	bool all;
	bool no_insts;
	bool line_info;
};

} }	// otawa::cfgio

#endif /* OTAWA_CFGIO_OUTPUT_H_ */
