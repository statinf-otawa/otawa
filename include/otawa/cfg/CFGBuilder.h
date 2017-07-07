/*
 *	$Id$
 *	CFGBuilder processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-09, IRIT UPS.
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

#ifndef OTAWA_CFG_CFG_BUILDER_H
#define OTAWA_CFG_CFG_BUILDER_H

#include <otawa/cfg/AbstractCFGBuilder.h>
#include <otawa/cfg/CFGInfo.h>

namespace otawa {

// CFGBuilder class
class CFGBuilder: public AbstractCFGBuilder {
public:
	static p::declare reg;
	CFGBuilder(p::declare& r = reg);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
};

} // otawa

#endif // OTAWA_CFG_CFG_BUILDER_H

