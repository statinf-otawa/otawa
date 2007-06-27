/*
 *	$Id$
 *	FirstLastBuilder processor implementation.
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
 
#ifndef CACHE_FIRSTLASTBUILDER_H_
#define CACHE_FIRSTLASTBUILDER_H_

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/hard/Cache.h>
#include <otawa/proc/Feature.h>
#include <otawa/cache/LBlock.h>


namespace otawa {
	
using namespace elm;

	
extern Identifier<LBlock**> LAST_LBLOCK;

extern Identifier<bool> LBLOCK_ISFIRST;

class FirstLastBuilder : public otawa::CFGProcessor {

	void processLBlockSet(CFG *, LBlockSet *, const hard::Cache *);
	
	public:
	FirstLastBuilder(void);
	virtual void processCFG(otawa::WorkSpace*, otawa::CFG*);
	
	
};

extern Feature<FirstLastBuilder> ICACHE_FIRSTLAST_FEATURE;

}

#endif /*CACHE_FIRSTLASTBUILDER_H_*/
