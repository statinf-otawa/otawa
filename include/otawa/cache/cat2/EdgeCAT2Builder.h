/*
 *	$Id$
 *	exegraph module implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#ifndef CACHE_EDGECAT2BUILDER_H_
#define CACHE_EDGECAT2BUILDER_H_

#include <otawa/prop/Identifier.h>
#include <otawa/hard/Cache.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/cache/cat2/ACSBuilder.h>

namespace otawa {



extern Identifier<Vector<category_t> *> CATEGORY_EDGE;
extern Identifier<Vector<BasicBlock*> *> CATEGORY_EDGE_HEADER;

class EdgeCAT2Builder: public CFGProcessor{
	void processLBlockSet(otawa::CFG*, LBlockSet *, const hard::Cache *);
	fmlevel_t firstmiss_level;
	public:
	EdgeCAT2Builder(void);
	virtual void processCFG(WorkSpace*, otawa::CFG*);
	virtual void setup(WorkSpace*);
	virtual void configure(const PropList &props);	
};

extern Feature<EdgeCAT2Builder> ICACHE_EDGE_CATEGORY2_FEATURE;


}


#endif /*CACHE_CAT2BUILDER_H_*/
