/*
 *	PostDominance class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
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
#ifndef OTAWA_CFG_POSTDOMINANCE_H
#define OTAWA_CFG_POSTDOMINANCE_H

#include <otawa/cfg/features.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {

class PostDominance: public CFGProcessor, public PostDomInfo {
public:
	static p::declare reg;
	PostDominance(p::declare& r = reg);

	bool pdom(Block *b1, Block *b2) override;
	void *interfaceFor(const AbstractFeature& feature) override;

protected:
	void processCFG(WorkSpace *fw, CFG *cfg) override;
	void destroyCFG(WorkSpace *ws, CFG *g) override;
};

} // otawa

#endif // OTAWA_CFG_POSTDOMINANCE_H
