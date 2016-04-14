/*
 *	$Id$
 *	CFGCollector processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006-08, IRIT UPS.
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
#ifndef OTAWA_CFG_CFG_COLLECTOR_H
#define OTAWA_CFG_CFG_COLLECTOR_H

#include <elm/genstruct/Vector.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/AbstractCFGBuilder.h>

namespace otawa {

// CFGCollector Class
class CFGCollector: public AbstractCFGBuilder {
public:
	static p::declare reg;

	CFGCollector(p::declare& r = reg);
	virtual void configure(const PropList& props);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);

private:
	genstruct::Vector<string> added_funs;
	genstruct::Vector<Address> added_cfgs;
};

} // otawa

#endif // OTAWA_CFG_CFG_COLLECTOR_H
