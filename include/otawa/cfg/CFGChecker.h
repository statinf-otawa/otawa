/*
 *	$Id$
 *	CFGChecker processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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

#ifndef OTAWA_CFG_CFGCHECKER_H_
#define OTAWA_CFG_CFGCHECKER_H_

#include <otawa/cfg/features.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa {

// CFGChecker class
class CFGChecker: public  BBProcessor {
public:
	static Identifier<bool> NO_EXCEPTION;

	CFGChecker(void);
	static p::declare reg;

	virtual void configure(const PropList& props);
	virtual void setup(WorkSpace *ws);
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *bb);
	virtual void cleanup(WorkSpace *ws);

private:
	bool failed, no_exn;
};

}	// otawa

#endif /* OTAWA_CFG_CFGCHECKER_H_ */
