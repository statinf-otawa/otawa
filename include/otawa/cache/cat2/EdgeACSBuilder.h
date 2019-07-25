/*
 *	$Id$
 *	This is the Abstract Cache State builder.
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
#ifndef CACHE_EDGEACSBUILDER_H_
#define CACHE_EDGEACSBUILDER_H_

#include <elm/data/Vector.h>
#include <otawa/prop/Identifier.h>
#include <otawa/proc/Processor.h>
#include <otawa/hard/Cache.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/cat2/PERSProblem.h>

namespace otawa {
	
using namespace elm;

extern Identifier<Vector<MUSTProblem::Domain*> *> CACHE_EDGE_ACS_MUST;
extern Identifier<Vector<PERSProblem::Domain*> *> CACHE_EDGE_ACS_PERS;

extern Identifier<fmlevel_t> FIRSTMISS_LEVEL;
extern Identifier<bool> PSEUDO_UNROLLING;
extern Identifier<Vector<MUSTProblem::Domain*>*> CACHE_ACS_MUST_ENTRY;

class EdgeACSBuilder : public otawa::Processor {

	void processLBlockSet(otawa::WorkSpace*, LBlockSet *, const hard::Cache *);	
	fmlevel_t level;
	
	bool unrolling;
	Vector<MUSTProblem::Domain *> *must_entry;
	
	public:
	EdgeACSBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList &props);
	
	
	
};

extern Feature<EdgeACSBuilder> ICACHE_EDGE_ACS_FEATURE;

}

#endif /*CACHE_EDGEACSBUILDER_H_*/
