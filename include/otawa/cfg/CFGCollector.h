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

#include <elm/data/Vector.h>
#include <otawa/cfg/features.h>
#include <otawa/cfg/AbstractCFGBuilder.h>
#include <otawa/view/features.h>

namespace otawa {

// CFGCollector Class
class CFGCollector: public AbstractCFGBuilder {
public:
	static p::declare reg;

	CFGCollector(p::declare& r = reg);
	void configure(const PropList& props) override;
	void *interfaceFor(const AbstractFeature& f) override;

protected:
	void setup(WorkSpace *ws) override;
	void cleanup(WorkSpace *ws) override;
	void destroy(WorkSpace *ws) override;

private:
	Vector<string> added_funs;
	Vector<Address> added_cfgs;
	CFGCollection *coll;
};

extern view::View& ASSEMBLY_VIEW;
extern view::PropertyType& REGISTERS_PROPERTY;

} // otawa

#endif // OTAWA_CFG_CFG_COLLECTOR_H

