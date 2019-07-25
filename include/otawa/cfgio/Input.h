/*
 *	CFGInput class interface
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
#ifndef OTAWA_CFGIO_INPUT_H_
#define OTAWA_CFGIO_INPUT_H_

#include <elm/avl/Set.h>
#include <elm/xom.h>
#include <otawa/cfg.h>
#include <otawa/cfgio/features.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa { namespace cfgio {

using namespace elm;

class Input: public Processor {
	typedef avl::Map<xom::String, Block *> bb_map_t;
	typedef avl::Map<xom::String, CFGMaker *> cfg_map_t;
	typedef Vector<bb_map_t*> bb_map_table_t;
	typedef Vector<Pair<xom::String, xom::String> > edge_list_t;
	typedef Vector<edge_list_t*> edge_list_table_t;

public:
	// inner class
	class CafeBabeInst: public otawa::Inst {
	public:
		virtual kind_t kind(void) { return 0; }
		virtual Address address(void) const { return Address(0xCAFEBABE); }
		virtual t::uint32 size(void) const { return 4; }
	} *cafebabeInst;

	Input(p::declare& r = reg);
	static p::declare reg;
protected:
	virtual void configure(const PropList& props);
	virtual void setup(WorkSpace *ws);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);

private:
	void reset(void);
	void clear(void);
	void raiseError(xom::Element *elt, const string& msg);
	void raiseError(const string& msg);

	Path path;
	CFGCollection *coll;
	cfg_map_t cfg_map;
	bb_map_table_t bb_map_table;
	edge_list_table_t edge_list_table;
	Vector<CFGMaker*> cfgMakers;
};

} }	// otawa::cfgio

#endif /* OTAWA_CFGIO_INPUT_H_ */
