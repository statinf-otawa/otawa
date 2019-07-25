/*
 *	EdgeProcessor class interface
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
#ifndef OTAWA_PROC_EDGEPROCESSOR_H_
#define OTAWA_PROC_EDGEPROCESSOR_H_

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/Edge.h>

namespace otawa {

class EdgeProcessor: public CFGProcessor {
public:
	EdgeProcessor(void);
	EdgeProcessor(cstring name, elm::Version version);
	EdgeProcessor(AbstractRegistration& reg);
	EdgeProcessor(cstring name, const Version& version, AbstractRegistration& reg);

protected:
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void processEdge(WorkSpace *ws, CFG *cfg, Edge *edge) = 0;
	virtual void cleanupCFG(WorkSpace *ws, CFG *cfg);
	virtual void cleanupBB(WorkSpace *ws, CFG *cfg, Block *bb);
	virtual void cleanupEdge(WorkSpace *ws, CFG *cfg, Edge *edge);
};

}	// otawa

#endif /* OTAWA_PROC_EDGEPROCESSOR_H_ */
