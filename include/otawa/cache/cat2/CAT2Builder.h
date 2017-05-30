/*
 *	CAT2Builder processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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

#ifndef OTAWA_CACHE_CAT2BUILDER_H_
#define OTAWA_CACHE_CAT2BUILDER_H_

#include <otawa/prop/Identifier.h>
#include <otawa/hard/Cache.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/proc/CFGProcessor.h>
//#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/cache/cat2/ACSBuilder.h>
#include <otawa/cache/features.h>

namespace otawa {

// CAT2Builder class
class CAT2Builder: public CFGProcessor {
public:
	CAT2Builder(AbstractRegistration& registration = reg);
	static p::declare reg;

	virtual void processCFG(WorkSpace*, otawa::CFG*);
	virtual void setup(WorkSpace*);
	virtual void configure(const PropList &props);	

private:
	void processLBlockSet(otawa::CFG*, LBlockSet *, const hard::Cache *);
	fmlevel_t firstmiss_level;
	cache::CategoryStats *cstats;
	int ah_cnt, am_cnt, pers_cnt, nc_cnt, total_cnt;
};

}  // otawa


#endif /* OTAWA_CACHE_CAT2BUILDER_H_*/
