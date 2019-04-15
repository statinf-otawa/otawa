/*
 *	$Id$
 *	ContextualProcessor class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_PROC_CONTEXTUALPROCESSOR_H_
#define OTAWA_PROC_CONTEXTUALPROCESSOR_H_

#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/ContextualProperty.h>

namespace otawa {

class Edge;
class Point;

// ContextualProcessor class
class ContextualProcessor: public CFGProcessor {
public:
	static p::declare reg;
	ContextualProcessor(p::declare &_reg = reg);

protected:
	void processCFG(WorkSpace *fw, CFG *cfg) override;
	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *bb, const ContextualPath& path) = 0;
};

} // otawa

#endif /* OTAWA_PROC_CONTEXTUALPROCESSOR_H_ */

